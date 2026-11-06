#include "SnapshotEngine.hpp"
#include "NtfsSecurityUtils.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QUuid>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QThread>
#include <map>
#include <set>
#include <string_view>
#include <unordered_map>
#include <mutex>

namespace AclAuditor::Engine {

static AceEntry parseAceFromJson(const QJsonObject& aObj) {
    AceEntry ace;
    const QString sidStr = aObj[QStringLiteral("sid")].toString();
    const QString nameStr = aObj[QStringLiteral("name")].toString();
    std::wstring sidW = sidStr.toStdWString();
    std::wstring nameW = nameStr.toStdWString();

    ace.trustee.sidString = sidW;
    if (!nameW.empty()) {
        auto slashPos = nameW.find(L'\\');
        if (slashPos != std::wstring::npos) {
            ace.trustee.domainName = nameW.substr(0, slashPos);
            ace.trustee.accountName = nameW.substr(slashPos + 1);
        } else {
            ace.trustee.accountName = nameW;
        }
    } else {
        ace.trustee = NtfsSecurityUtils::resolveAccountOrSidString(sidW);
    }

    ace.type = (aObj[QStringLiteral("type")].toString() == QStringLiteral("Allow") ? AccessType::Allow : AccessType::Deny);
    ace.accessMask = static_cast<DWORD>(aObj[QStringLiteral("mask")].toInteger());
    ace.isInherited = aObj[QStringLiteral("inherited")].toBool();

    DWORD mask = ace.accessMask;
    ace.canReadData = (mask & FILE_READ_DATA) != 0;
    ace.canWriteData = (mask & FILE_WRITE_DATA) != 0;
    ace.canAppendData = (mask & FILE_APPEND_DATA) != 0;
    ace.canExecute = (mask & FILE_EXECUTE) != 0;
    ace.canDeleteChild = (mask & FILE_DELETE_CHILD) != 0;
    ace.canDelete = (mask & DELETE) != 0;
    ace.canReadAttributes = (mask & FILE_READ_ATTRIBUTES) != 0;
    ace.canWriteAttributes = (mask & FILE_WRITE_ATTRIBUTES) != 0;
    ace.canReadEA = (mask & FILE_READ_EA) != 0;
    ace.canWriteEA = (mask & FILE_WRITE_EA) != 0;
    ace.canReadPermissions = (mask & READ_CONTROL) != 0;
    ace.canWriteDac = (mask & WRITE_DAC) != 0;
    ace.canWriteOwner = (mask & WRITE_OWNER) != 0;
    ace.canSynchronize = (mask & SYNCHRONIZE) != 0;

    if ((mask & FILE_ALL_ACCESS) == FILE_ALL_ACCESS || (mask & GENERIC_ALL) == GENERIC_ALL) {
        ace.fullControl = true;
        ace.modify = true;
        ace.readAndExecute = true;
        ace.read = true;
        ace.write = true;
    } else {
        DWORD modifyMask = FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | DELETE;
        if ((mask & modifyMask) == modifyMask) {
            ace.modify = true;
        }
        if ((mask & FILE_GENERIC_READ) == FILE_GENERIC_READ) {
            ace.read = true;
        }
        if ((mask & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE) {
            ace.readAndExecute = true;
        }
        if ((mask & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE ||
            ((mask & FILE_WRITE_DATA) && (mask & FILE_APPEND_DATA))) {
            ace.write = true;
        }
        if (!ace.fullControl && !ace.modify && !ace.readAndExecute && !ace.read && !ace.write) {
            ace.special = true;
        }
    }

    return ace;
}

SnapshotEngine& SnapshotEngine::instance() {
    static SnapshotEngine inst;
    return inst;
}

static QString getThreadConnectionName() {
    auto threadId = reinterpret_cast<quintptr>(QThread::currentThreadId());
    return QStringLiteral("ACLAuditorConn_%1").arg(threadId);
}

QSqlDatabase SnapshotEngine::getDatabaseConnection() {
    const QString connName = getThreadConnectionName();
    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase db = QSqlDatabase::database(connName);
        if (db.isOpen()) {
            return db;
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    db.setDatabaseName(QString::fromStdWString(m_dbPath));
    if (!db.open()) {
        qWarning("SnapshotEngine: Falha ao abrir conexao SQLite %s", qPrintable(connName));
    }
    return db;
}

void SnapshotEngine::cleanupThreadConnection() {
    const QString connName = getThreadConnectionName();
    if (QSqlDatabase::contains(connName)) {
        {
            auto db = QSqlDatabase::database(connName, false);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(connName);
    }
}

bool SnapshotEngine::initializeDatabase(const std::wstring& customDbPath) {
    if (m_initialized) return true;

    QString dbFilePath;
    if (!customDbPath.empty()) {
        dbFilePath = QString::fromStdWString(customDbPath);
    } else {
        QString appData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QDir dir(appData);
        if (!dir.exists()) {
            dir.mkpath(QStringLiteral("."));
        }
        dbFilePath = dir.filePath(QStringLiteral("acl_auditor_snapshots.db"));
    }

    m_dbPath = dbFilePath.toStdWString();

    QSqlDatabase db = getDatabaseConnection();
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery q(db);
    // Table for snapshot metadata
    q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS snapshots ("
        "id TEXT PRIMARY KEY, "
        "title TEXT, "
        "root_path TEXT, "
        "timestamp_iso TEXT, "
        "total_files INTEGER, "
        "total_dirs INTEGER, "
        "total_risks INTEGER, "
        "sha256_hash TEXT, "
        "payload_json TEXT"
        ")"
    ));

    m_initialized = true;
    return true;
}

void SnapshotEngine::flattenNode(const FileSecurityNode& node, std::vector<SnapshotItemRecord>& outItems) {
    SnapshotItemRecord item;
    item.path = node.path;
    item.isDirectory = node.isDirectory;
    item.owner = node.security.owner.getDisplayName();
    item.inheritanceBlocked = node.security.isInheritanceBlocked;
    item.aces = node.security.dacl;
    item.riskScore = node.riskScore;
    outItems.push_back(item);

    for (const auto& child : node.children) {
        flattenNode(child, outItems);
    }
}

SnapshotRecord SnapshotEngine::createSnapshot(
    const std::wstring& title,
    const std::wstring& rootPath,
    const FileSecurityNode& rootNode,
    const std::vector<RiskFinding>& findings
) {
    SnapshotRecord rec;
    rec.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdWString();
    rec.title = title.empty() ? (L"Snapshot - " + rootNode.fileName) : title;
    rec.rootPath = rootPath;
    rec.timestampIso = QDateTime::currentDateTime().toString(Qt::ISODate).toStdWString();
    rec.totalRisks = findings.size();

    flattenNode(rootNode, rec.items);

    rec.totalDirectories = 0;
    rec.totalFiles = 0;
    for (const auto& it : rec.items) {
        if (it.isDirectory) rec.totalDirectories++;
        else rec.totalFiles++;
    }

    // Build JSON representation for hashing and storage
    QJsonObject rootObj;
    rootObj[QStringLiteral("id")] = QString::fromStdWString(rec.id);
    rootObj[QStringLiteral("title")] = QString::fromStdWString(rec.title);
    rootObj[QStringLiteral("rootPath")] = QString::fromStdWString(rec.rootPath);
    rootObj[QStringLiteral("timestamp")] = QString::fromStdWString(rec.timestampIso);
    rootObj[QStringLiteral("totalFiles")] = static_cast<qint64>(rec.totalFiles);
    rootObj[QStringLiteral("totalDirs")] = static_cast<qint64>(rec.totalDirectories);
    rootObj[QStringLiteral("totalRisks")] = static_cast<qint64>(rec.totalRisks);

    QJsonArray itemsArr;
    for (const auto& item : rec.items) {
        QJsonObject iObj;
        iObj[QStringLiteral("path")] = QString::fromStdWString(item.path);
        iObj[QStringLiteral("isDir")] = item.isDirectory;
        iObj[QStringLiteral("owner")] = QString::fromStdWString(item.owner);
        iObj[QStringLiteral("blocked")] = item.inheritanceBlocked;
        iObj[QStringLiteral("risk")] = item.riskScore;

        QJsonArray acesArr;
        for (const auto& ace : item.aces) {
            QJsonObject aObj;
            aObj[QStringLiteral("sid")] = QString::fromStdWString(ace.trustee.sidString);
            aObj[QStringLiteral("name")] = QString::fromStdWString(ace.trustee.getDisplayName());
            aObj[QStringLiteral("type")] = (ace.type == AccessType::Allow ? QStringLiteral("Allow") : QStringLiteral("Deny"));
            aObj[QStringLiteral("mask")] = static_cast<qint64>(ace.accessMask);
            aObj[QStringLiteral("inherited")] = ace.isInherited;
            aObj[QStringLiteral("summary")] = QString::fromStdWString(ace.getRightsSummary());
            acesArr.append(aObj);
        }
        iObj[QStringLiteral("aces")] = acesArr;
        itemsArr.append(iObj);
    }
    rootObj[QStringLiteral("items")] = itemsArr;

    QByteArray docData = QJsonDocument(rootObj).toJson(QJsonDocument::Compact);
    rec.sha256Hash = NtfsSecurityUtils::computeSha256Hex(docData.constData(), docData.size());

    return rec;
}

bool SnapshotEngine::saveSnapshot(const SnapshotRecord& record) {
    if (!m_initialized) {
        if (!initializeDatabase()) return false;
    }

    QSqlDatabase db = getDatabaseConnection();
    if (!db.isOpen()) return false;

    // Serialize items into JSON
    QJsonObject rootObj;
    rootObj[QStringLiteral("id")] = QString::fromStdWString(record.id);
    rootObj[QStringLiteral("title")] = QString::fromStdWString(record.title);
    rootObj[QStringLiteral("rootPath")] = QString::fromStdWString(record.rootPath);
    rootObj[QStringLiteral("timestamp")] = QString::fromStdWString(record.timestampIso);
    rootObj[QStringLiteral("totalFiles")] = static_cast<qint64>(record.totalFiles);
    rootObj[QStringLiteral("totalDirs")] = static_cast<qint64>(record.totalDirectories);
    rootObj[QStringLiteral("totalRisks")] = static_cast<qint64>(record.totalRisks);
    rootObj[QStringLiteral("sha256")] = QString::fromStdString(record.sha256Hash);

    QJsonArray itemsArr;
    for (const auto& item : record.items) {
        QJsonObject iObj;
        iObj[QStringLiteral("path")] = QString::fromStdWString(item.path);
        iObj[QStringLiteral("isDir")] = item.isDirectory;
        iObj[QStringLiteral("owner")] = QString::fromStdWString(item.owner);
        iObj[QStringLiteral("blocked")] = item.inheritanceBlocked;
        iObj[QStringLiteral("risk")] = item.riskScore;

        QJsonArray acesArr;
        for (const auto& ace : item.aces) {
            QJsonObject aObj;
            aObj[QStringLiteral("sid")] = QString::fromStdWString(ace.trustee.sidString);
            aObj[QStringLiteral("name")] = QString::fromStdWString(ace.trustee.getDisplayName());
            aObj[QStringLiteral("type")] = (ace.type == AccessType::Allow ? QStringLiteral("Allow") : QStringLiteral("Deny"));
            aObj[QStringLiteral("mask")] = static_cast<qint64>(ace.accessMask);
            aObj[QStringLiteral("inherited")] = ace.isInherited;
            aObj[QStringLiteral("summary")] = QString::fromStdWString(ace.getRightsSummary());
            acesArr.append(aObj);
        }
        iObj[QStringLiteral("aces")] = acesArr;
        itemsArr.append(iObj);
    }
    rootObj[QStringLiteral("items")] = itemsArr;

    QString jsonStr = QString::fromUtf8(QJsonDocument(rootObj).toJson(QJsonDocument::Compact));

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO snapshots (id, title, root_path, timestamp_iso, total_files, total_dirs, total_risks, sha256_hash, payload_json) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
    ));
    q.addBindValue(QString::fromStdWString(record.id));
    q.addBindValue(QString::fromStdWString(record.title));
    q.addBindValue(QString::fromStdWString(record.rootPath));
    q.addBindValue(QString::fromStdWString(record.timestampIso));
    q.addBindValue(static_cast<qint64>(record.totalFiles));
    q.addBindValue(static_cast<qint64>(record.totalDirectories));
    q.addBindValue(static_cast<qint64>(record.totalRisks));
    q.addBindValue(QString::fromStdString(record.sha256Hash));
    q.addBindValue(jsonStr);

    bool ok = q.exec();
    if (ok) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_snapshotCache[record.id] = record;
    }
    return ok;
}

std::vector<SnapshotRecord> SnapshotEngine::listSnapshots() {
    std::vector<SnapshotRecord> list;
    if (!m_initialized) {
        if (!initializeDatabase()) return list;
    }

    QSqlDatabase db = getDatabaseConnection();
    QSqlQuery q(QStringLiteral("SELECT id, title, root_path, timestamp_iso, total_files, total_dirs, total_risks, sha256_hash FROM snapshots ORDER BY timestamp_iso DESC"), db);

    while (q.next()) {
        SnapshotRecord r;
        r.id = q.value(0).toString().toStdWString();
        r.title = q.value(1).toString().toStdWString();
        r.rootPath = q.value(2).toString().toStdWString();
        r.timestampIso = q.value(3).toString().toStdWString();
        r.totalFiles = q.value(4).toULongLong();
        r.totalDirectories = q.value(5).toULongLong();
        r.totalRisks = q.value(6).toULongLong();
        r.sha256Hash = q.value(7).toString().toStdString();
        list.push_back(r);
    }

    return list;
}

std::optional<SnapshotRecord> SnapshotEngine::loadSnapshot(const std::wstring& id) {
    if (!m_initialized) {
        if (!initializeDatabase()) return std::nullopt;
    }

    // Fast check: in-memory cache for instant zero-latency retrieval
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto it = m_snapshotCache.find(id);
        if (it != m_snapshotCache.end()) {
            return it->second;
        }
    }

    QSqlDatabase db = getDatabaseConnection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT id, title, root_path, timestamp_iso, total_files, total_dirs, total_risks, sha256_hash, payload_json FROM snapshots WHERE id = ?"));
    q.addBindValue(QString::fromStdWString(id));

    if (q.exec() && q.next()) {
        SnapshotRecord r;
        r.id = q.value(0).toString().toStdWString();
        r.title = q.value(1).toString().toStdWString();
        r.rootPath = q.value(2).toString().toStdWString();
        r.timestampIso = q.value(3).toString().toStdWString();
        r.totalFiles = q.value(4).toULongLong();
        r.totalDirectories = q.value(5).toULongLong();
        r.totalRisks = q.value(6).toULongLong();
        r.sha256Hash = q.value(7).toString().toStdString();

        const QByteArray payloadBytes = q.value(8).toByteArray();
        QJsonDocument doc = QJsonDocument::fromJson(payloadBytes.isEmpty() ? q.value(8).toString().toUtf8() : payloadBytes);
        if (doc.isObject()) {
            QJsonArray itemsArr = doc.object()[QStringLiteral("items")].toArray();
            r.items.reserve(itemsArr.size());
            for (const auto& val : itemsArr) {
                QJsonObject itObj = val.toObject();
                SnapshotItemRecord item;
                item.path = itObj[QStringLiteral("path")].toString().toStdWString();
                item.isDirectory = itObj[QStringLiteral("isDir")].toBool();
                item.owner = itObj[QStringLiteral("owner")].toString().toStdWString();
                item.inheritanceBlocked = itObj[QStringLiteral("blocked")].toBool();
                item.riskScore = itObj[QStringLiteral("risk")].toInt();

                QJsonArray acesArr = itObj[QStringLiteral("aces")].toArray();
                item.aces.reserve(acesArr.size());
                for (const auto& aVal : acesArr) {
                    item.aces.push_back(parseAceFromJson(aVal.toObject()));
                }

                r.items.push_back(std::move(item));
            }
        }

        // Cache the parsed snapshot
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            m_snapshotCache[id] = r;
        }

        return r;
    }

    return std::nullopt;
}

bool SnapshotEngine::deleteSnapshot(const std::wstring& id) {
    if (!m_initialized) {
        if (!initializeDatabase()) return false;
    }

    QSqlDatabase db = getDatabaseConnection();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM snapshots WHERE id = ?"));
    q.addBindValue(QString::fromStdWString(id));
    bool ok = q.exec();
    if (ok) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_snapshotCache.erase(id);
    }
    return ok;
}

void SnapshotEngine::clearSnapshotCache() {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    m_snapshotCache.clear();
}

SnapshotDiffResult SnapshotEngine::compareSnapshots(const SnapshotRecord& baseline, const SnapshotRecord& current) {
    SnapshotDiffResult diff;
    diff.baselineId = baseline.id;
    diff.currentId = current.id;
    diff.baselineTimestamp = baseline.timestampIso;
    diff.currentTimestamp = current.timestampIso;

    // Fast O(1) hash maps with std::wstring_view avoiding any string allocations/copies
    std::unordered_map<std::wstring_view, const SnapshotItemRecord*> baseMap;
    baseMap.reserve(baseline.items.size());
    for (const auto& item : baseline.items) {
        baseMap.emplace(std::wstring_view(item.path), &item);
    }

    std::unordered_map<std::wstring_view, const SnapshotItemRecord*> currMap;
    currMap.reserve(current.items.size());
    for (const auto& item : current.items) {
        currMap.emplace(std::wstring_view(item.path), &item);
    }

    // Pre-reserve diff vector to minimize reallocations
    diff.changes.reserve(std::max<size_t>(64, (baseline.items.size() + current.items.size()) / 8));

    // Check for added objects and modifications
    for (const auto& currItem : current.items) {
        std::wstring_view pathView(currItem.path);
        auto itBase = baseMap.find(pathView);
        if (itBase == baseMap.end()) {
            DiffEntry de;
            de.path = currItem.path;
            de.isDirectory = currItem.isDirectory;
            de.changeType = DiffChangeType::ObjectAdded;
            de.summary = L"Novo objeto identificado no snapshot atual";
            de.oldValue = L"(Inexistente)";
            de.newValue = currItem.owner;
            de.severityDelta = RiskSeverity::Info;
            diff.changes.push_back(std::move(de));
        } else {
            const SnapshotItemRecord* baseItem = itBase->second;

            // Check owner change
            if (currItem.owner != baseItem->owner) {
                DiffEntry de;
                de.path = currItem.path;
                de.isDirectory = currItem.isDirectory;
                de.changeType = DiffChangeType::OwnerChanged;
                de.summary = L"Proprietário do objeto foi alterado";
                de.oldValue = baseItem->owner;
                de.newValue = currItem.owner;
                de.severityDelta = RiskSeverity::Medium;
                diff.changes.push_back(std::move(de));
            }

            // Check inheritance block change
            if (currItem.inheritanceBlocked != baseItem->inheritanceBlocked) {
                DiffEntry de;
                de.path = currItem.path;
                de.isDirectory = currItem.isDirectory;
                de.changeType = DiffChangeType::InheritanceChanged;
                de.summary = currItem.inheritanceBlocked ? L"Herança foi BLOQUEADA (DACL protegida)" : L"Herança foi RESTAURADA";
                de.oldValue = baseItem->inheritanceBlocked ? L"Bloqueada" : L"Herdada";
                de.newValue = currItem.inheritanceBlocked ? L"Bloqueada" : L"Herdada";
                de.severityDelta = currItem.inheritanceBlocked ? RiskSeverity::High : RiskSeverity::Low;
                diff.changes.push_back(std::move(de));
            }

            // Fast ACE comparison without heap allocations
            const auto& baseAces = baseItem->aces;
            const auto& currAces = currItem.aces;
            std::vector<bool> baseMatched(baseAces.size(), false);

            for (const auto& currAce : currAces) {
                int matchedIdx = -1;
                for (size_t i = 0; i < baseAces.size(); ++i) {
                    if (!baseMatched[i] &&
                        baseAces[i].trustee.sidString == currAce.trustee.sidString &&
                        baseAces[i].type == currAce.type) {
                        matchedIdx = static_cast<int>(i);
                        break;
                    }
                }

                if (matchedIdx >= 0) {
                    baseMatched[matchedIdx] = true;
                    const auto& baseAce = baseAces[matchedIdx];

                    // Check if rights/permissions changed
                    if (currAce.accessMask != baseAce.accessMask) {
                        bool hasEscalation = (currAce.fullControl && !baseAce.fullControl) ||
                                             (currAce.modify && !baseAce.modify) ||
                                             ((currAce.accessMask & ~baseAce.accessMask) != 0);

                        DiffEntry de;
                        de.path = currItem.path;
                        de.isDirectory = currItem.isDirectory;
                        de.oldValue = baseAce.getRightsSummary();
                        de.newValue = currAce.getRightsSummary();

                        if (hasEscalation) {
                            de.changeType = DiffChangeType::RightsEscalated;
                            de.summary = L"Elevação de permissão para: " + currAce.trustee.getDisplayName();
                            de.severityDelta = (currAce.trustee.sidString == L"S-1-1-0" || currAce.fullControl) ? RiskSeverity::High : RiskSeverity::Medium;
                            if (de.severityDelta == RiskSeverity::High) diff.totalNewRisks++;
                        } else {
                            de.changeType = DiffChangeType::RightsReduced;
                            de.summary = L"Redução de permissão para: " + currAce.trustee.getDisplayName();
                            de.severityDelta = RiskSeverity::Info;
                        }
                        diff.changes.push_back(std::move(de));
                    }
                } else {
                    // New ACE added
                    DiffEntry de;
                    de.path = currItem.path;
                    de.isDirectory = currItem.isDirectory;
                    de.changeType = DiffChangeType::AceAdded;
                    de.summary = L"Nova permissão concedida para: " + currAce.trustee.getDisplayName();
                    de.oldValue = L"(Sem acesso prévio)";
                    de.newValue = currAce.getRightsSummary();
                    de.severityDelta = (currAce.trustee.sidString == L"S-1-1-0" || currAce.fullControl) ? RiskSeverity::High : RiskSeverity::Info;
                    if (de.severityDelta == RiskSeverity::High) diff.totalNewRisks++;
                    diff.changes.push_back(std::move(de));
                }
            }

            // Check for removed ACEs
            for (size_t i = 0; i < baseAces.size(); ++i) {
                if (!baseMatched[i]) {
                    const auto& baseAce = baseAces[i];
                    DiffEntry de;
                    de.path = currItem.path;
                    de.isDirectory = currItem.isDirectory;
                    de.changeType = DiffChangeType::AceRemoved;
                    de.summary = L"Permissão revogada para: " + baseAce.trustee.getDisplayName();
                    de.oldValue = baseAce.getRightsSummary();
                    de.newValue = L"(Acesso removido)";
                    de.severityDelta = RiskSeverity::Info;
                    diff.changes.push_back(std::move(de));
                }
            }
        }
    }

    // Check for removed objects (in baseline but not in current)
    for (const auto& baseItem : baseline.items) {
        std::wstring_view pathView(baseItem.path);
        if (currMap.find(pathView) == currMap.end()) {
            DiffEntry de;
            de.path = baseItem.path;
            de.isDirectory = baseItem.isDirectory;
            de.changeType = DiffChangeType::ObjectRemoved;
            de.summary = L"Objeto foi excluído ou movido fora do escopo";
            de.oldValue = baseItem.owner;
            de.newValue = L"(Excluído)";
            de.severityDelta = RiskSeverity::Info;
            diff.changes.push_back(std::move(de));
        }
    }

    diff.totalChanges = diff.changes.size();
    return diff;
}

bool SnapshotEngine::exportSnapshotToJson(const SnapshotRecord& record, const std::wstring& filePath) {
    QJsonObject rootObj;
    rootObj[QStringLiteral("schema")] = QStringLiteral("ACLAuditor.Snapshot.v1");
    rootObj[QStringLiteral("id")] = QString::fromStdWString(record.id);
    rootObj[QStringLiteral("title")] = QString::fromStdWString(record.title);
    rootObj[QStringLiteral("rootPath")] = QString::fromStdWString(record.rootPath);
    rootObj[QStringLiteral("timestamp")] = QString::fromStdWString(record.timestampIso);
    rootObj[QStringLiteral("totalFiles")] = static_cast<qint64>(record.totalFiles);
    rootObj[QStringLiteral("totalDirs")] = static_cast<qint64>(record.totalDirectories);
    rootObj[QStringLiteral("totalRisks")] = static_cast<qint64>(record.totalRisks);
    rootObj[QStringLiteral("sha256Evidence")] = QString::fromStdString(record.sha256Hash);

    QJsonArray itemsArr;
    for (const auto& item : record.items) {
        QJsonObject iObj;
        iObj[QStringLiteral("path")] = QString::fromStdWString(item.path);
        iObj[QStringLiteral("isDir")] = item.isDirectory;
        iObj[QStringLiteral("owner")] = QString::fromStdWString(item.owner);
        iObj[QStringLiteral("blocked")] = item.inheritanceBlocked;
        iObj[QStringLiteral("risk")] = item.riskScore;

        QJsonArray acesArr;
        for (const auto& ace : item.aces) {
            QJsonObject aObj;
            aObj[QStringLiteral("sid")] = QString::fromStdWString(ace.trustee.sidString);
            aObj[QStringLiteral("name")] = QString::fromStdWString(ace.trustee.getDisplayName());
            aObj[QStringLiteral("type")] = (ace.type == AccessType::Allow ? QStringLiteral("Allow") : QStringLiteral("Deny"));
            aObj[QStringLiteral("mask")] = static_cast<qint64>(ace.accessMask);
            aObj[QStringLiteral("inherited")] = ace.isInherited;
            aObj[QStringLiteral("summary")] = QString::fromStdWString(ace.getRightsSummary());
            acesArr.append(aObj);
        }
        iObj[QStringLiteral("aces")] = acesArr;
        itemsArr.append(iObj);
    }
    rootObj[QStringLiteral("items")] = itemsArr;

    QFile file(QString::fromStdWString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(QJsonDocument(rootObj).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

std::optional<SnapshotRecord> SnapshotEngine::importSnapshotFromJson(const std::wstring& filePath) {
    QFile file(QString::fromStdWString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return std::nullopt;

    QJsonObject obj = doc.object();
    SnapshotRecord rec;
    rec.id = obj[QStringLiteral("id")].toString().toStdWString();
    rec.title = obj[QStringLiteral("title")].toString().toStdWString();
    rec.rootPath = obj[QStringLiteral("rootPath")].toString().toStdWString();
    rec.timestampIso = obj[QStringLiteral("timestamp")].toString().toStdWString();
    rec.totalFiles = static_cast<size_t>(obj[QStringLiteral("totalFiles")].toInteger());
    rec.totalDirectories = static_cast<size_t>(obj[QStringLiteral("totalDirs")].toInteger());
    rec.totalRisks = static_cast<size_t>(obj[QStringLiteral("totalRisks")].toInteger());
    rec.sha256Hash = obj[QStringLiteral("sha256Evidence")].toString().toStdString();

    QJsonArray itemsArr = obj[QStringLiteral("items")].toArray();
    rec.items.reserve(itemsArr.size());
    for (const auto& v : itemsArr) {
        QJsonObject itObj = v.toObject();
        SnapshotItemRecord item;
        item.path = itObj[QStringLiteral("path")].toString().toStdWString();
        item.isDirectory = itObj[QStringLiteral("isDir")].toBool();
        item.owner = itObj[QStringLiteral("owner")].toString().toStdWString();
        item.inheritanceBlocked = itObj[QStringLiteral("blocked")].toBool();
        item.riskScore = itObj[QStringLiteral("risk")].toInt();

        QJsonArray acesArr = itObj[QStringLiteral("aces")].toArray();
        item.aces.reserve(acesArr.size());
        for (const auto& aVal : acesArr) {
            item.aces.push_back(parseAceFromJson(aVal.toObject()));
        }
        rec.items.push_back(std::move(item));
    }

    // Cache imported snapshot for fast subsequent diffs
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_snapshotCache[rec.id] = rec;
    }

    return rec;
}

} // namespace AclAuditor::Engine
