#pragma once

#include "../engine/AclTypes.hpp"
#include <QAbstractTableModel>
#include <vector>

namespace AclAuditor::Model {

class AceTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit AceTableModel(QObject* parent = nullptr);
    ~AceTableModel() override = default;

    void setAces(const std::vector<Engine::AceEntry>& aces);
    void clear();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    const Engine::AceEntry* getAceAt(int row) const;

    void refreshTranslations() {
        emit layoutChanged();
        emit headerDataChanged(Qt::Horizontal, 0, 5);
    }

private:
    std::vector<Engine::AceEntry> m_aces;
};

} // namespace AclAuditor::Model
