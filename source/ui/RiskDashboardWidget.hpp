#pragma once

#include "../engine/AclTypes.hpp"
#include "../model/RiskTableModel.hpp"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QTextBrowser>
#include <QGroupBox>

namespace AclAuditor::UI {

class MultiColumnSortFilterProxyModel;

class RiskDashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit RiskDashboardWidget(QWidget* parent = nullptr);
    ~RiskDashboardWidget() override = default;

    void setFindings(const std::vector<Engine::RiskFinding>& findings);
    void clear();
    void retranslateUi();

public slots:
    void applyTheme(bool isDark);

signals:
    void openRemediationPlanRequested(const std::vector<Engine::RiskFinding>& findings);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onSelectionChanged();
    void onFilterTextChanged(const QString& text);
    void onCopyIcaclsClicked();
    void onCopyPowerShellClicked();
    void onGeneratePlanClicked();

private:
    void setupUi();
    void updateMetricCards();

    std::vector<Engine::RiskFinding> m_allFindings;
    std::vector<Engine::RiskFinding> m_filteredFindings;

    // Metrics cards widgets
    class QFrame* m_cardCritical{nullptr};
    QLabel* m_lblTitleCritical{nullptr};
    QLabel* m_lblCardCritical{nullptr};

    class QFrame* m_cardHigh{nullptr};
    QLabel* m_lblTitleHigh{nullptr};
    QLabel* m_lblCardHigh{nullptr};

    class QFrame* m_cardMedium{nullptr};
    QLabel* m_lblTitleMedium{nullptr};
    QLabel* m_lblCardMedium{nullptr};

    class QFrame* m_cardWritable{nullptr};
    QLabel* m_lblTitleWritable{nullptr};
    QLabel* m_lblCardWritable{nullptr};

    class QFrame* m_cardOrphaned{nullptr};
    QLabel* m_lblTitleOrphaned{nullptr};
    QLabel* m_lblCardOrphaned{nullptr};

    QLineEdit* m_searchEdit{nullptr};
    QTableView* m_tableView{nullptr};
    Model::RiskTableModel* m_model{nullptr};
    MultiColumnSortFilterProxyModel* m_proxyModel{nullptr};

    // Details panel
    QGroupBox* m_detailGroup{nullptr};
    QLabel* m_lblDetailTitle{nullptr};
    QTextBrowser* m_detailText{nullptr};
    QLineEdit* m_icaclsEdit{nullptr};
    QLineEdit* m_powershellEdit{nullptr};
    QPushButton* m_btnCopyIcacls{nullptr};
    QPushButton* m_btnCopyPs{nullptr};
    QPushButton* m_btnPlan{nullptr};
};

} // namespace AclAuditor::UI
