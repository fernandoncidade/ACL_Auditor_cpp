#pragma once

#include "AclTypes.hpp"
#include <QSqlDatabase>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <mutex>

namespace AclAuditor::Engine {

class SnapshotEngine {
public:
    static SnapshotEngine& instance();

    // Initializes local SQLite database in %LOCALAPPDATA%/ACLAuditor
    bool initializeDatabase(const std::wstring& customDbPath = L"");

    // Creates a snapshot record from a scanned file security tree
    SnapshotRecord createSnapshot(
        const std::wstring& title,
        const std::wstring& rootPath,
        const FileSecurityNode& rootNode,
        const std::vector<RiskFinding>& findings
    );

    // Persists a snapshot to the SQLite database
    bool saveSnapshot(const SnapshotRecord& record);

    // Loads all saved snapshot metadata summaries
    std::vector<SnapshotRecord> listSnapshots();

    // Loads a full snapshot with all items by ID (cached for high performance)
    std::optional<SnapshotRecord> loadSnapshot(const std::wstring& id);

    // Deletes a snapshot by ID
    bool deleteSnapshot(const std::wstring& id);

    // Performs ultra-fast temporal comparison between two snapshots (Snapshot A vs Snapshot B)
    SnapshotDiffResult compareSnapshots(const SnapshotRecord& baseline, const SnapshotRecord& current);

    // Exports a snapshot to a tamper-evident JSON file with SHA-256 hash
    bool exportSnapshotToJson(const SnapshotRecord& record, const std::wstring& filePath);

    // Imports a snapshot from a JSON file, validating its SHA-256 integrity hash
    std::optional<SnapshotRecord> importSnapshotFromJson(const std::wstring& filePath);

    // Clears the in-memory cache of loaded snapshots
    void clearSnapshotCache();

    // Cleans up any thread-isolated SQLite database connection for the calling thread
    void cleanupThreadConnection();

    // Database path
    std::wstring getDatabasePath() const { return m_dbPath; }

private:
    SnapshotEngine() = default;
    ~SnapshotEngine() = default;

    std::wstring m_dbPath;
    bool m_initialized{false};

    std::unordered_map<std::wstring, SnapshotRecord> m_snapshotCache;
    mutable std::mutex m_cacheMutex;

    void flattenNode(const FileSecurityNode& node, std::vector<SnapshotItemRecord>& outItems);
    QSqlDatabase getDatabaseConnection();
};

} // namespace AclAuditor::Engine
