#pragma once

#include "../engine/AclTypes.hpp"
#include <QAbstractTableModel>
#include <vector>

namespace AclAuditor::Model {

class DiffTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit DiffTableModel(QObject* parent = nullptr);
    ~DiffTableModel() override = default;

    void setDiff(const Engine::SnapshotDiffResult& diff);
    void setDiff(Engine::SnapshotDiffResult&& diff);
    void clear();
    void refreshTranslations();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    std::vector<Engine::DiffEntry> m_entries;
};

} // namespace AclAuditor::Model
