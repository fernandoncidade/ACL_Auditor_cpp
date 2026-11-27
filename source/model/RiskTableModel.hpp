#pragma once

#include "../engine/AclTypes.hpp"
#include <QAbstractTableModel>
#include <vector>

namespace AclAuditor::Model {

class RiskTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit RiskTableModel(QObject* parent = nullptr);
    ~RiskTableModel() override = default;

    void setFindings(const std::vector<Engine::RiskFinding>& findings);
    void clear();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void refreshTranslations();

    const Engine::RiskFinding* getFindingAt(int row) const;
    const std::vector<Engine::RiskFinding>& getAllFindings() const { return m_findings; }

    static QString formatFindingDescriptionLocalized(const Engine::RiskFinding& f);
    static QString formatFindingTechnicalDetailLocalized(const Engine::RiskFinding& f);

private:
    std::vector<Engine::RiskFinding> m_findings;
};

} // namespace AclAuditor::Model
