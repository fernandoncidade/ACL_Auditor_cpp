#pragma once

#include "../engine/AclTypes.hpp"
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>

namespace AclAuditor::UI {

enum class ReportFormat {
    Html,
    Pdf,
    Csv,
    Json
};

class ReportExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReportExportDialog(
        const Engine::FileSecurityNode& scannedRoot,
        const std::vector<Engine::RiskFinding>& findings,
        QWidget* parent = nullptr
    );
    ~ReportExportDialog() override = default;

public slots:
    void retranslateUi();

private slots:
    void onSelectLogoClicked();
    void onExportClicked();

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();

    QString generateHtmlReport() const;
    QString generateCsvReport() const;
    QString generateJsonReport() const;
    bool exportToPdf(const QString& targetPdfPath, const QString& htmlContent) const;

    Engine::FileSecurityNode m_rootNode;
    std::vector<Engine::RiskFinding> m_findings;

    QGroupBox* m_metaGroup{nullptr};
    QGroupBox* m_optGroup{nullptr};
    QLabel* m_lblFormat{nullptr};
    QLabel* m_lblCompany{nullptr};
    QLabel* m_lblClient{nullptr};
    QLabel* m_lblAuditor{nullptr};
    QLabel* m_lblLogo{nullptr};

    QComboBox* m_formatCombo{nullptr};
    QLineEdit* m_companyEdit{nullptr};
    QLineEdit* m_clientEdit{nullptr};
    QLineEdit* m_auditorEdit{nullptr};
    QLineEdit* m_logoPathEdit{nullptr};
    QPushButton* m_btnBrowseLogo{nullptr};

    QCheckBox* m_chkExecutiveSummary{nullptr};
    QCheckBox* m_chkTechnicalDetails{nullptr};
    QCheckBox* m_chkRemediationScripts{nullptr};

    QPushButton* m_btnExport{nullptr};
    QPushButton* m_btnCancel{nullptr};
};

} // namespace AclAuditor::UI
