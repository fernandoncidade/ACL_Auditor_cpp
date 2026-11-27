#include "TestSupport.hpp"
#include "engine/AclTypes.hpp"
#include "engine/RiskScanner.hpp"
#include "engine/SnapshotEngine.hpp"

#include <QFile>
#include <QtTest>

using namespace AclAuditor::Engine;

class TestSnapshotEngineAndDiff : public QObject {
    Q_OBJECT

private slots:
    void testDatabaseCrudAndCachingLifecycle() {
        AclTestSupport::IsolatedTestAppDataGuard appDataGuard;
        QVERIFY(appDataGuard.isValid());

        SnapshotEngine& engine = SnapshotEngine::instance();
        QVERIFY(engine.initializeDatabase(appDataGuard.dbFilePath(QStringLiteral("snap_crud.db")).toStdWString()));
        QVERIFY(!engine.getDatabasePath().empty());

        const FileSecurityNode baselineTree = AclTestSupport::makeSampleTree(L"C:\\SnapRoot");
        const auto findings = RiskScanner::evaluateTreeRisks(baselineTree);

        const SnapshotRecord created = engine.createSnapshot(
            L"Baseline Audit", baselineTree.path, baselineTree, findings);
        QVERIFY(!created.id.empty());
        QCOMPARE(created.sha256Hash.size(), size_t{64});
        QVERIFY(created.totalDirectories >= 1);
        QVERIFY(created.totalFiles >= 2);
        QVERIFY(!created.items.empty());

        QVERIFY(engine.saveSnapshot(created));

        const auto listed = engine.listSnapshots();
        bool foundInList = false;
        for (const auto& s : listed) {
            if (s.id == created.id) {
                foundInList = true;
                QCOMPARE(QString::fromStdWString(s.title), QStringLiteral("Baseline Audit"));
            }
        }
        QVERIFY(foundInList);

        // Cached load + cold DB load after clearing cache
        QVERIFY(engine.loadSnapshot(created.id).has_value());
        engine.clearSnapshotCache();
        const auto loadedFromDb = engine.loadSnapshot(created.id);
        QVERIFY(loadedFromDb.has_value());
        QCOMPARE(loadedFromDb->items.size(), created.items.size());
        QCOMPARE(QString::fromStdString(loadedFromDb->sha256Hash),
                 QString::fromStdString(created.sha256Hash));

        QVERIFY(engine.deleteSnapshot(created.id));
        engine.clearSnapshotCache();
        QVERIFY(!engine.loadSnapshot(created.id).has_value());
        engine.cleanupThreadConnection();
    }

    void testCompareSnapshotsDetectsAllChangeClasses() {
        SnapshotEngine& engine = SnapshotEngine::instance();

        FileSecurityNode treeA = AclTestSupport::makeSampleTree(L"C:\\CompareRoot");
        SnapshotRecord snapA = engine.createSnapshot(
            L"Snap A", treeA.path, treeA, RiskScanner::evaluateTreeRisks(treeA));

        FileSecurityNode treeB = treeA;
        // 1. Modify owner & inheritance on root
        treeB.security.owner = AclTestSupport::makeSid(
            L"S-1-5-18", L"SYSTEM", L"NT AUTHORITY", PrincipalCategory::ServiceAccount);
        treeB.security.isInheritanceBlocked = false;
        // 2. Add a new ACE and remove an old ACE
        treeB.security.dacl.pop_back();
        treeB.security.dacl.push_back(AclTestSupport::makeAce(
            L"S-1-5-11", L"Authenticated Users", L"NT AUTHORITY",
            PrincipalCategory::WellKnown, AccessType::Allow, FILE_ALL_ACCESS, false));
        // 3. Remove one child object (secrets.env) and add a brand new child
        treeB.children.pop_back();
        FileSecurityNode addedChild;
        addedChild.path = L"C:\\CompareRoot\\new_config.json";
        addedChild.fileName = L"new_config.json";
        addedChild.isDirectory = false;
        addedChild.security.hasDacl = true;
        addedChild.security.owner = treeB.security.owner;
        treeB.children.push_back(addedChild);

        SnapshotRecord snapB = engine.createSnapshot(
            L"Snap B", treeB.path, treeB, RiskScanner::evaluateTreeRisks(treeB));

        const SnapshotDiffResult diff = engine.compareSnapshots(snapA, snapB);
        QVERIFY(diff.totalChanges > 0);
        QCOMPARE(diff.changes.size(), diff.totalChanges);

        bool sawOwnerChanged = false;
        bool sawInheritanceChanged = false;
        bool sawAceAdded = false;
        bool sawAceRemoved = false;
        bool sawObjectAdded = false;
        bool sawObjectRemoved = false;

        for (const auto& entry : diff.changes) {
            switch (entry.changeType) {
                case DiffChangeType::OwnerChanged:       sawOwnerChanged = true; break;
                case DiffChangeType::InheritanceChanged: sawInheritanceChanged = true; break;
                case DiffChangeType::AceAdded:           sawAceAdded = true; break;
                case DiffChangeType::AceRemoved:         sawAceRemoved = true; break;
                case DiffChangeType::ObjectAdded:        sawObjectAdded = true; break;
                case DiffChangeType::ObjectRemoved:      sawObjectRemoved = true; break;
                default: break;
            }
        }

        QVERIFY(sawOwnerChanged);
        QVERIFY(sawInheritanceChanged);
        QVERIFY(sawAceAdded);
        QVERIFY(sawAceRemoved);
        QVERIFY(sawObjectAdded);
        QVERIFY(sawObjectRemoved);
    }

    void testJsonExportAndImportRoundTrip() {
        AclTestSupport::NtfsTestSandbox sandbox;
        QVERIFY(sandbox.isValid());

        SnapshotEngine& engine = SnapshotEngine::instance();
        const FileSecurityNode tree = AclTestSupport::makeSampleTree(L"C:\\ExportRoot");
        const SnapshotRecord original = engine.createSnapshot(
            L"Export Snapshot", tree.path, tree, RiskScanner::evaluateTreeRisks(tree));

        const std::wstring jsonPath = sandbox.filePathW(QStringLiteral("snapshot_export.json"));
        QVERIFY(engine.exportSnapshotToJson(original, jsonPath));
        QVERIFY(QFile::exists(QString::fromStdWString(jsonPath)));

        const auto imported = engine.importSnapshotFromJson(jsonPath);
        QVERIFY(imported.has_value());
        QCOMPARE(QString::fromStdWString(imported->id), QString::fromStdWString(original.id));
        QCOMPARE(QString::fromStdWString(imported->title), QString::fromStdWString(original.title));
        QCOMPARE(QString::fromStdString(imported->sha256Hash), QString::fromStdString(original.sha256Hash));
        QCOMPARE(imported->items.size(), original.items.size());

        // Malformed JSON and missing files must return nullopt cleanly
        QVERIFY(sandbox.createFile(QStringLiteral("bad.json"), QByteArray("{ not valid json")));
        QVERIFY(!engine.importSnapshotFromJson(sandbox.filePathW(QStringLiteral("bad.json"))).has_value());
        QVERIFY(!engine.importSnapshotFromJson(sandbox.filePathW(QStringLiteral("does_not_exist.json"))).has_value());
    }
};

QTEST_GUILESS_MAIN(TestSnapshotEngineAndDiff)
#if !defined(__INTELLISENSE__)
#include "test_06_SnapshotEngineAndDiff.moc"
#endif
