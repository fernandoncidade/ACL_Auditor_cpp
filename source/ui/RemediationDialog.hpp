#pragma once

#include "../engine/RemediationPlanner.hpp"
#include <QDialog>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

namespace AclAuditor::UI {

class RemediationDialog : public QDialog {
    Q_OBJECT

public:
    explicit RemediationDialog(
        const std::wstring& rootPath,
        const std::vector<Engine::RiskFinding>& findings,
        QWidget* parent = nullptr
    );
    ~RemediationDialog() override = default;

public slots:
    void retranslateUi();

private slots:
    void onCopyCurrentTab();
    void onExportScript();

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();

    Engine::RemediationPlan m_plan;

    QLabel* m_lblBannerTitle{nullptr};
    QLabel* m_lblBannerDesc{nullptr};

    QTabWidget* m_tabWidget{nullptr};
    QTextEdit* m_psEditor{nullptr};
    QTextEdit* m_icaclsEditor{nullptr};
    QTextEdit* m_mdEditor{nullptr};
    QTextEdit* m_rollbackEditor{nullptr};

    QPushButton* m_btnCopy{nullptr};
    QPushButton* m_btnExport{nullptr};
    QPushButton* m_btnClose{nullptr};
};

} // namespace AclAuditor::UI
