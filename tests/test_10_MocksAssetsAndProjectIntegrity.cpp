#include "CompileTranslationsTerminal.hpp"
#include "find_VersionEditor.hpp"
#include "utils/ApplicationPathUtils.hpp"

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QtTest>

class TestMocksAssetsAndProjectIntegrity : public QObject {
    Q_OBJECT

private slots:
    void testVersionEditorSpecsAndCheckExpectedLinesOnRepo() {
        const QString sourceDir = QDir::cleanPath(QStringLiteral(ACL_AUDITOR_SOURCE_DIR));
        const QString detected = mocks::version_editor::normalize_dir(mocks::version_editor::detect_project_root());
        QVERIFY(QDir(sourceDir).exists());
        QVERIFY(!detected.isEmpty());

        const auto specs = mocks::version_editor::file_specs(sourceDir);
        QCOMPARE(specs.size(), 33);

        const auto statuses = mocks::version_editor::check_expected_lines(sourceDir);
        QCOMPARE(statuses.size(), 33);

        const QStringList clcRelativePaths = {
            QStringLiteral("source/assets/CLC/CLC_pt_BR.txt"),
            QStringLiteral("source/assets/CLC/CLC_en_US.txt")
        };

        bool sawAppxManifest = false;
        bool sawLibraryTxt = false;

        for (const auto& st : statuses) {
            QVERIFY2(st.exists, qPrintable(QStringLiteral("Missing tracked file: ") + st.relative_path));
            if (!clcRelativePaths.contains(st.relative_path)) {
                QVERIFY2(!st.found.isEmpty(),
                         qPrintable(st.relative_path + QStringLiteral(" did not match any expected markers.")));
                QVERIFY2(st.missing.isEmpty(),
                         qPrintable(st.relative_path + QStringLiteral(" missing expected markers: ") + st.missing.join(QStringLiteral(", "))));
            }
            if (st.relative_path == QStringLiteral("library/AppxManifest.xml")) {
                sawAppxManifest = true;
                QCOMPARE(st.found.size(), 1);
                QVERIFY(st.missing.isEmpty());
            } else if (st.relative_path == QStringLiteral("library/ACL_Auditor_cpp.txt")) {
                sawLibraryTxt = true;
                QCOMPARE(st.found.size(), 5);
                QVERIFY(st.missing.isEmpty());
            }
        }
        QVERIFY(sawAppxManifest);
        QVERIFY(sawLibraryTxt);
    }

    void testVersionEditorApplyUpdatesInIsolatedSandbox() {
        const QString sourceDir = QDir::cleanPath(QStringLiteral(ACL_AUDITOR_SOURCE_DIR));
        QTemporaryDir sandbox;
        QVERIFY(sandbox.isValid());
        const QString sandboxRoot = sandbox.path();

        // Copy all 33 monitored files into the temporary sandbox
        const auto specs = mocks::version_editor::file_specs(sourceDir);
        for (const auto& spec : specs) {
            const QString srcFile = QDir(sourceDir).filePath(spec.relative_path);
            const QString dstFile = QDir(sandboxRoot).filePath(spec.relative_path);
            QVERIFY(QDir().mkpath(QFileInfo(dstFile).absolutePath()));
            QVERIFY2(QFile::copy(srcFile, dstFile), qPrintable(spec.relative_path));
        }

        const QString syntheticVersion = QStringLiteral("2099.12.25.0");
        mocks::version_editor::UpdateContext ctx;
        ctx.pt_version = syntheticVersion;
        ctx.pt_date = QStringLiteral("25 de dezembro de 2099");
        ctx.en_version = syntheticVersion;
        ctx.en_date = QStringLiteral("December 25, 2099");
        ctx.pt_readme_version = QStringLiteral("v2099.12.25.0");
        ctx.en_readme_version = QStringLiteral("v2099.12.25.0");

        const auto results = mocks::version_editor::apply_updates(ctx, sandboxRoot);
        QCOMPARE(results.size(), 33);
        for (const auto& res : results) {
            QVERIFY2(res.ok, qPrintable(res.relative_path + QStringLiteral(": ") + res.message));
        }

        auto readNormalizedContentAndLines = [](const QString& filePath, QString* outContent, QStringList* outLines) -> bool {
            QFile f(filePath);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return false;
            }
            const QString content = QString::fromUtf8(f.readAll());
            if (outContent != nullptr) {
                *outContent = content;
            }
            if (outLines != nullptr) {
                *outLines = content.split(QLatin1Char('\n'));
            }
            return true;
        };

        // Verify updated tokens in representative sandboxed targets
        const QStringList checkRelPaths = {
            QStringLiteral("source/main.cpp"),
            QStringLiteral("library/ACL_Auditor.iss"),
            QStringLiteral("README.md"),
            QStringLiteral("docs/architecture.md"),
            QStringLiteral("SECURITY.md")
        };
        for (const QString& rel : checkRelPaths) {
            QString content;
            QVERIFY(readNormalizedContentAndLines(QDir(sandboxRoot).filePath(rel), &content, nullptr));
            QVERIFY2(content.contains(syntheticVersion), qPrintable(rel));
        }

        // Verify library/AppxManifest.xml Identity Version update and that no other lines changed
        {
            QStringList origLines;
            QStringList updLines;
            QString updContent;
            QVERIFY(readNormalizedContentAndLines(
                QDir(sourceDir).filePath(QStringLiteral("library/AppxManifest.xml")), nullptr, &origLines));
            QVERIFY(readNormalizedContentAndLines(
                QDir(sandboxRoot).filePath(QStringLiteral("library/AppxManifest.xml")), &updContent, &updLines));
            QCOMPARE(updLines.size(), origLines.size());
            QVERIFY(updContent.contains(QStringLiteral(
                "  <Identity Name=\"fernandoncidade.ACLAuditor\" Publisher=\"CN=F1FFF36E-09E6-42F5-BAE5-3124C3054234\" Version=\"2099.12.25.0\" ProcessorArchitecture=\"x64\" />")));

            int changedCount = 0;
            for (qsizetype i = 0; i < origLines.size(); ++i) {
                if (updLines[i] != origLines[i]) {
                    QVERIFY2(updLines[i].contains(QStringLiteral("<Identity "))
                                 && updLines[i].contains(QStringLiteral("Version=\"2099.12.25.0\"")),
                             qPrintable(QStringLiteral("Unexpected line change in AppxManifest.xml: ") + updLines[i]));
                    ++changedCount;
                }
            }
            QCOMPARE(changedCount, 1);
        }

        // Verify library/ACL_Auditor_cpp.txt updates only the 5 target lines and preserves all others
        {
            QStringList origLines;
            QStringList updLines;
            QString updContent;
            QVERIFY(readNormalizedContentAndLines(
                QDir(sourceDir).filePath(QStringLiteral("library/ACL_Auditor_cpp.txt")), nullptr, &origLines));
            QVERIFY(readNormalizedContentAndLines(
                QDir(sandboxRoot).filePath(QStringLiteral("library/ACL_Auditor_cpp.txt")), &updContent, &updLines));
            QCOMPARE(updLines.size(), origLines.size());

            QVERIFY(updContent.contains(QStringLiteral("docs\\release-v2099.12.25.0.md\";")));
            QVERIFY(updContent.contains(QStringLiteral("NOTAS DE LANÇAMENTO: Versão 2099.12.25.0 — ACL AUDITOR")));
            QVERIFY(updContent.contains(QStringLiteral("Data de Lançamento: 25 de dezembro de 2099")));
            QVERIFY(updContent.contains(QStringLiteral("RELEASE NOTES: Version 2099.12.25.0 — ACL AUDITOR")));
            QVERIFY(updContent.contains(QStringLiteral("Release Date: December 25, 2099")));

            int changedCount = 0;
            for (qsizetype i = 0; i < origLines.size(); ++i) {
                if (updLines[i] != origLines[i]) {
                    const bool isAllowedTarget =
                        updLines[i].contains(QStringLiteral("docs\\release-v"))
                        || updLines[i].startsWith(QStringLiteral("NOTAS DE LANÇAMENTO:"))
                        || updLines[i].startsWith(QStringLiteral("Data de Lançamento:"))
                        || updLines[i].startsWith(QStringLiteral("RELEASE NOTES:"))
                        || updLines[i].startsWith(QStringLiteral("Release Date:"));
                    QVERIFY2(isAllowedTarget,
                             qPrintable(QStringLiteral("Unexpected line change in ACL_Auditor_cpp.txt: ") + updLines[i]));
                    ++changedCount;
                }
            }
            QCOMPARE(changedCount, 5);
        }

        // Post-update check_expected_lines on sandbox
        const auto sandboxStatuses = mocks::version_editor::check_expected_lines(sandboxRoot);
        bool postAppxOk = false;
        bool postTxtOk = false;
        for (const auto& st : sandboxStatuses) {
            if (st.relative_path == QStringLiteral("library/AppxManifest.xml")) {
                QCOMPARE(st.found.size(), 1);
                QVERIFY(st.missing.isEmpty());
                postAppxOk = true;
            } else if (st.relative_path == QStringLiteral("library/ACL_Auditor_cpp.txt")) {
                QCOMPARE(st.found.size(), 5);
                QVERIFY(st.missing.isEmpty());
                postTxtOk = true;
            }
        }
        QVERIFY(postAppxOk);
        QVERIFY(postTxtOk);
    }

    void testTranslationsCompilerTerminalAndLreleaseSandbox() {
        // Negative validation via library call (does not touch real repo .qm files)
        mocks::translations::TranslationCompilerTerminal terminal;
        const int invalidRc = terminal.run({
            QCoreApplication::applicationFilePath(),
            QStringLiteral("--module=nonexistent_module_xyz")
        });
        QVERIFY(invalidRc != 0);

        // Compile clean copies of .ts files in temp dir using lrelease.exe
        QTemporaryDir sandbox;
        QVERIFY(sandbox.isValid());

        const QString srcTransDir = QStringLiteral(ACL_AUDITOR_SOURCE_DIR) + QStringLiteral("/source/language/translations");
        const QStringList tsNames = {
            QStringLiteral("aclauditor_pt_BR.ts"),
            QStringLiteral("aclauditor_en_US.ts")
        };

        for (const QString& tsName : tsNames) {
            const QString copiedTs = sandbox.filePath(tsName);
            QVERIFY(QFile::copy(QDir(srcTransDir).filePath(tsName), copiedTs));
            const QString qmOut = sandbox.filePath(QFileInfo(tsName).completeBaseName() + QStringLiteral(".qm"));

            QProcess lrel;
            lrel.start(QStringLiteral(ACL_AUDITOR_LRELEASE_EXECUTABLE),
                       {copiedTs, QStringLiteral("-qm"), qmOut});
            QVERIFY(lrel.waitForStarted(5000));
            QVERIFY(lrel.waitForFinished(15000));
            QCOMPARE(lrel.exitCode(), 0);
            QVERIFY(QFileInfo::exists(qmOut));
            QVERIFY(QFileInfo(qmOut).size() > 0);
        }
    }

    void testProjectStructureAndRefactoredCmakeScriptsIntegrity() {
        const QDir root(QStringLiteral(ACL_AUDITOR_SOURCE_DIR));
        QVERIFY(root.exists());

        // Refactored / moved files must exist in new locations
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("source/windows_app_icon.rc.in"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("cmake/launch_detached_gui.cmake"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("cmake/deploy_optional_qt_binary.cmake"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("cmake/gerar_distribuicoes_limpas.cmake"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("cmake/limpar_artefatos_regeneraveis_qt.cmake"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("cmake/run_mingw.cmake"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("cmake/run_msvc.cmake"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("tests/CMakeLists.txt"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("tests/TestSupport.hpp"))));
        QVERIFY(QFileInfo::exists(root.filePath(QStringLiteral("tests/test_11_EngineEndToEndRegression.cpp"))));

        // Legacy .ps1 / .bat / moved files must be gone
        QVERIFY(!QFileInfo::exists(root.filePath(QStringLiteral("cmake/windows_app_icon.rc.in"))));
        QVERIFY(!QFileInfo::exists(root.filePath(QStringLiteral("cmake/gerar_distribuicoes_limpas.ps1"))));
        QVERIFY(!QFileInfo::exists(root.filePath(QStringLiteral("cmake/limpar_artefatos_regeneraveis_qt.ps1"))));
        QVERIFY(!QFileInfo::exists(root.filePath(QStringLiteral("cmake/run_mingw.bat"))));
        QVERIFY(!QFileInfo::exists(root.filePath(QStringLiteral("cmake/run_msvc.bat"))));
        QVERIFY(!QFileInfo::exists(root.filePath(QStringLiteral("mocks/teste.cpp"))));
    }
};

QTEST_MAIN(TestMocksAssetsAndProjectIntegrity)
#if !defined(__INTELLISENSE__)
#include "test_10_MocksAssetsAndProjectIntegrity.moc"
#endif
