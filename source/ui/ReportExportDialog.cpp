#include "ReportExportDialog.hpp"
#include "Win32DwmHelper.hpp"
#include "ThemeManager.hpp"
#include "../engine/RiskScanner.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include "../utils/ApplicationPathUtils.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEvent>

namespace AclAuditor::UI {

ReportExportDialog::ReportExportDialog(
    const Engine::FileSecurityNode& scannedRoot,
    const std::vector<Engine::RiskFinding>& findings,
    QWidget* parent
) : QDialog(parent), m_rootNode(scannedRoot), m_findings(findings) {
    const QIcon appIcon = Utils::ApplicationPathUtils::getAppIcon();
    if (!appIcon.isNull()) {
        setWindowIcon(appIcon);
    }
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    resize(540, 480);
    setMinimumSize(480, 400);

    Win32DwmHelper::applyMicaAndDarkMode(this);

    setupUi();
    retranslateUi();

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &ReportExportDialog::retranslateUi);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](bool isDark) {
        Win32DwmHelper::applyTitleBarTheme(this, isDark);
    });
    Win32DwmHelper::applyTitleBarTheme(this, ThemeManager::instance().isDark());
}

void ReportExportDialog::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void ReportExportDialog::retranslateUi() {
    setWindowTitle(Language::GerenciadorTraducao::trText("Exportador de Relatórios Técnicos e Executivos"));

    if (m_metaGroup) {
        m_metaGroup->setTitle(Language::GerenciadorTraducao::trText("Informações de Consultoria e Auditoria"));
    }
    if (m_lblFormat) {
        m_lblFormat->setText(Language::GerenciadorTraducao::trText("Formato do Relatório:"));
    }
    if (m_formatCombo) {
        int curIdx = m_formatCombo->currentIndex();
        m_formatCombo->blockSignals(true);
        m_formatCombo->setItemText(0, Language::GerenciadorTraducao::trText("HTML Interativo (Completo e Responsivo)"));
        m_formatCombo->setItemText(1, Language::GerenciadorTraducao::trText("PDF Executivo (Formatado para Impressão)"));
        m_formatCombo->setItemText(2, Language::GerenciadorTraducao::trText("CSV Estruturado (Excel / SIEM)"));
        m_formatCombo->setItemText(3, Language::GerenciadorTraducao::trText("JSON Completo (Automação / API)"));
        m_formatCombo->setCurrentIndex(curIdx >= 0 ? curIdx : 0);
        m_formatCombo->blockSignals(false);
    }

    if (m_lblCompany) {
        m_lblCompany->setText(Language::GerenciadorTraducao::trText("Empresa de Consultoria:"));
    }
    if (m_lblClient) {
        m_lblClient->setText(Language::GerenciadorTraducao::trText("Cliente / Organização:"));
    }
    if (m_lblAuditor) {
        m_lblAuditor->setText(Language::GerenciadorTraducao::trText("Auditor Responsável:"));
    }
    if (m_lblLogo) {
        m_lblLogo->setText(Language::GerenciadorTraducao::trText("Logotipo da Consultoria:"));
    }
    if (m_logoPathEdit) {
        m_logoPathEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Opcional: selecione imagem .png ou .jpg"));
    }
    if (m_btnBrowseLogo) {
        m_btnBrowseLogo->setText(Language::GerenciadorTraducao::trText("Procurar..."));
    }

    if (m_optGroup) {
        m_optGroup->setTitle(Language::GerenciadorTraducao::trText("Seções do Relatório"));
    }
    if (m_chkExecutiveSummary) {
        m_chkExecutiveSummary->setText(Language::GerenciadorTraducao::trText("Incluir Resumo Executivo e Matriz de Riscos"));
    }
    if (m_chkTechnicalDetails) {
        m_chkTechnicalDetails->setText(Language::GerenciadorTraducao::trText("Incluir Tabela Detalhada de Vulnerabilidades Identificadas"));
    }
    if (m_chkRemediationScripts) {
        m_chkRemediationScripts->setText(Language::GerenciadorTraducao::trText("Incluir Scripts de Remediação (PowerShell e icacls)"));
    }

    if (m_btnCancel) {
        m_btnCancel->setText(Language::GerenciadorTraducao::trText("Cancelar"));
    }
    if (m_btnExport) {
        m_btnExport->setText(Language::GerenciadorTraducao::trText("Gerar e Salvar Relatório"));
    }
}

void ReportExportDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    m_metaGroup = new QGroupBox(this);
    auto* metaLayout = new QGridLayout(m_metaGroup);

    m_lblFormat = new QLabel(m_metaGroup);
    metaLayout->addWidget(m_lblFormat, 0, 0);

    m_formatCombo = new QComboBox(m_metaGroup);
    m_formatCombo->addItem(QString(), static_cast<int>(ReportFormat::Html));
    m_formatCombo->addItem(QString(), static_cast<int>(ReportFormat::Pdf));
    m_formatCombo->addItem(QString(), static_cast<int>(ReportFormat::Csv));
    m_formatCombo->addItem(QString(), static_cast<int>(ReportFormat::Json));
    metaLayout->addWidget(m_formatCombo, 0, 1, 1, 2);

    m_lblCompany = new QLabel(m_metaGroup);
    metaLayout->addWidget(m_lblCompany, 1, 0);
    m_companyEdit = new QLineEdit(QStringLiteral("Consultoria de Segurança e Infraestrutura"), m_metaGroup);
    metaLayout->addWidget(m_companyEdit, 1, 1, 1, 2);

    m_lblClient = new QLabel(m_metaGroup);
    metaLayout->addWidget(m_lblClient, 2, 0);
    m_clientEdit = new QLineEdit(QStringLiteral("Empresa Auditada"), m_metaGroup);
    metaLayout->addWidget(m_clientEdit, 2, 1, 1, 2);

    m_lblAuditor = new QLabel(m_metaGroup);
    metaLayout->addWidget(m_lblAuditor, 3, 0);
    m_auditorEdit = new QLineEdit(QStringLiteral("Especialista em Segurança Windows"), m_metaGroup);
    metaLayout->addWidget(m_auditorEdit, 3, 1, 1, 2);

    m_lblLogo = new QLabel(m_metaGroup);
    metaLayout->addWidget(m_lblLogo, 4, 0);
    m_logoPathEdit = new QLineEdit(m_metaGroup);
    metaLayout->addWidget(m_logoPathEdit, 4, 1);
    m_btnBrowseLogo = new QPushButton(m_metaGroup);
    connect(m_btnBrowseLogo, &QPushButton::clicked, this, &ReportExportDialog::onSelectLogoClicked);
    metaLayout->addWidget(m_btnBrowseLogo, 4, 2);

    mainLayout->addWidget(m_metaGroup);

    m_optGroup = new QGroupBox(this);
    auto* optLayout = new QVBoxLayout(m_optGroup);

    m_chkExecutiveSummary = new QCheckBox(m_optGroup);
    m_chkExecutiveSummary->setChecked(true);
    optLayout->addWidget(m_chkExecutiveSummary);

    m_chkTechnicalDetails = new QCheckBox(m_optGroup);
    m_chkTechnicalDetails->setChecked(true);
    optLayout->addWidget(m_chkTechnicalDetails);

    m_chkRemediationScripts = new QCheckBox(m_optGroup);
    m_chkRemediationScripts->setChecked(true);
    optLayout->addWidget(m_chkRemediationScripts);

    mainLayout->addWidget(m_optGroup);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_btnCancel = new QPushButton(this);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_btnCancel);

    m_btnExport = new QPushButton(this);
    m_btnExport->setStyleSheet(QStringLiteral("font-weight: bold; padding: 6px 16px;"));
    connect(m_btnExport, &QPushButton::clicked, this, &ReportExportDialog::onExportClicked);
    btnLayout->addWidget(m_btnExport);

    mainLayout->addLayout(btnLayout);
}

void ReportExportDialog::onSelectLogoClicked() {
    QString file = QFileDialog::getOpenFileName(
        this,
        Language::GerenciadorTraducao::trText("Selecionar Logotipo"),
        QString(),
        Language::GerenciadorTraducao::trText("Imagens (*.png *.jpg *.jpeg *.bmp)")
    );
    if (!file.isEmpty()) {
        m_logoPathEdit->setText(file);
    }
}

QString ReportExportDialog::generateHtmlReport() const {
    QString timeNow = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm:ss"));
    int riskScore = Engine::RiskScanner::calculateRiskScore(m_findings);

    int critCount = 0, highCount = 0, medCount = 0, lowCount = 0;
    for (const auto& f : m_findings) {
        if (f.severity == Engine::RiskSeverity::Critical) critCount++;
        else if (f.severity == Engine::RiskSeverity::High) highCount++;
        else if (f.severity == Engine::RiskSeverity::Medium) medCount++;
        else lowCount++;
    }

    QString html;
    QTextStream out(&html);

    out << "<!DOCTYPE html>\n<html lang=\"pt-BR\">\n<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<title>Relatório de Auditoria NTFS - ACL Auditor</title>\n"
        << "<style>\n"
        << "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background: #f8f9fa; color: #212529; margin: 0; padding: 24px; }\n"
        << ".container { max-width: 1000px; margin: 0 auto; background: #fff; padding: 32px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.08); }\n"
        << ".header { border-bottom: 2px solid #0066cc; padding-bottom: 16px; margin-bottom: 24px; display: flex; justify-content: space-between; align-items: center; }\n"
        << ".header h1 { margin: 0; color: #004085; font-size: 24px; }\n"
        << ".meta { font-size: 13px; color: #6c757d; line-height: 1.6; }\n"
        << ".cards { display: grid; grid-template-columns: repeat(4, 1fr); gap: 12px; margin: 20px 0; }\n"
        << ".card { background: #fdfdfd; border: 1px solid #dee2e6; border-radius: 6px; padding: 12px; text-align: center; }\n"
        << ".card .val { font-size: 24px; font-weight: bold; }\n"
        << ".card.crit { border-left: 4px solid #dc3545; color: #dc3545; }\n"
        << ".card.high { border-left: 4px solid #fd7e14; color: #fd7e14; }\n"
        << ".card.med { border-left: 4px solid #ffc107; color: #ffc107; }\n"
        << ".card.low { border-left: 4px solid #28a745; color: #28a745; }\n"
        << "table { width: 100%; border-collapse: collapse; margin-top: 16px; font-size: 13px; }\n"
        << "th, td { border: 1px solid #dee2e6; padding: 10px; text-align: left; }\n"
        << "th { background: #f1f3f5; font-weight: 600; }\n"
        << ".badge { display: inline-block; padding: 3px 8px; border-radius: 4px; font-size: 11px; font-weight: bold; color: #fff; }\n"
        << ".badge-crit { background: #dc3545; }\n"
        << ".badge-high { background: #fd7e14; }\n"
        << ".badge-med { background: #ffc107; color: #212529; }\n"
        << ".badge-low { background: #28a745; }\n"
        << ".code-block { background: #272822; color: #f8f8f2; padding: 10px; border-radius: 4px; font-family: monospace; font-size: 11px; overflow-x: auto; }\n"
        << "</style>\n</head>\n<body>\n"
        << "<div class=\"container\">\n"
        << "<div class=\"header\">\n"
        << "<div>\n"
        << "<h1>Relatório de Conformidade e Auditoria NTFS</h1>\n"
        << "<div class=\"meta\">Gerado por: <b>" << m_companyEdit->text() << "</b> | Auditor: " << m_auditorEdit->text() << "<br>\n"
        << "Cliente: <b>" << m_clientEdit->text() << "</b> | Data: " << timeNow << "</div>\n"
        << "</div>\n"
        << "</div>\n";

    if (m_chkExecutiveSummary->isChecked()) {
        out << "<h2>Resumo Executivo</h2>\n"
            << "<p>Foi realizada a auditoria de permissões, herança e integridade de acesso no escopo: <code>"
            << QString::fromStdWString(m_rootNode.path) << "</code>.</p>\n"
            << "<p><b>Pontuação Geral de Risco Calculada: " << riskScore << " / 100</b></p>\n"
            << "<div class=\"cards\">\n"
            << "<div class=\"card crit\"><div class=\"val\">" << critCount << "</div><div>Críticos</div></div>\n"
            << "<div class=\"card high\"><div class=\"val\">" << highCount << "</div><div>Altos</div></div>\n"
            << "<div class=\"card med\"><div class=\"val\">" << medCount << "</div><div>Médios</div></div>\n"
            << "<div class=\"card low\"><div class=\"val\">" << lowCount << "</div><div>Baixos / Info</div></div>\n"
            << "</div>\n";
    }

    if (m_chkTechnicalDetails->isChecked() && !m_findings.empty()) {
        out << "<h2>Vulnerabilidades e Desvios de Conformidade Detectados</h2>\n"
            << "<table>\n"
            << "<thead><tr><th>Severidade</th><th>Vulnerabilidade</th><th>Caminho Afetado</th><th>Principal</th><th>Ação Recomendada</th></tr></thead>\n"
            << "<tbody>\n";

        for (const auto& f : m_findings) {
            QString badgeClass = (f.severity == Engine::RiskSeverity::Critical) ? QStringLiteral("badge-crit") :
                                 (f.severity == Engine::RiskSeverity::High) ? QStringLiteral("badge-high") :
                                 (f.severity == Engine::RiskSeverity::Medium) ? QStringLiteral("badge-med") : QStringLiteral("badge-low");

            out << "<tr>\n"
                << "<td><span class=\"badge " << badgeClass << "\">" << QString::fromStdWString(Engine::RiskFinding::severityToString(f.severity)) << "</span></td>\n"
                << "<td><b>" << QString::fromStdWString(f.title) << "</b><br><small>" << QString::fromStdWString(f.description) << "</small></td>\n"
                << "<td><code>" << QString::fromStdWString(f.affectedPath) << "</code></td>\n"
                << "<td>" << QString::fromStdWString(f.affectedPrincipal) << "</td>\n"
                << "<td>" << QString::fromStdWString(f.remediationAdvice) << "</td>\n"
                << "</tr>\n";
        }

        out << "</tbody></table>\n";
    }

    if (m_chkRemediationScripts->isChecked() && !m_findings.empty()) {
        out << "<h2>Plano Técnico de Remediação (Scripts Prontos)</h2>\n"
            << "<p>Os comandos abaixo podem ser executados com privilégios de Administrador para mitigar as vulnerabilidades reportadas:</p>\n";

        for (const auto& f : m_findings) {
            if (!f.icaclsCommand.empty()) {
                out << "<p><b>" << QString::fromStdWString(f.title) << "</b></p>\n"
                    << "<div class=\"code-block\">" << QString::fromStdWString(f.icaclsCommand) << "</div>\n";
            }
        }
    }

    out << "</div>\n</body>\n</html>\n";
    return html;
}

QString ReportExportDialog::generateCsvReport() const {
    QString csv = QStringLiteral("\xEF\xBB\xBF"); // UTF-8 BOM
    QTextStream out(&csv);

    out << "Severidade;Titulo;CaminhoAfetado;Principal;Descricao;Recomendacao;ComandoIcacls\n";

    for (const auto& f : m_findings) {
        auto esc = [](const std::wstring& s) -> QString {
            QString str = QString::fromStdWString(s);
            str.replace(QStringLiteral("\""), QStringLiteral("\"\""));
            return QStringLiteral("\"%1\"").arg(str);
        };

        out << esc(Engine::RiskFinding::severityToString(f.severity)) << ";"
            << esc(f.title) << ";"
            << esc(f.affectedPath) << ";"
            << esc(f.affectedPrincipal) << ";"
            << esc(f.description) << ";"
            << esc(f.remediationAdvice) << ";"
            << esc(f.icaclsCommand) << "\n";
    }

    return csv;
}

QString ReportExportDialog::generateJsonReport() const {
    QJsonObject root;
    root[QStringLiteral("company")] = m_companyEdit->text();
    root[QStringLiteral("client")] = m_clientEdit->text();
    root[QStringLiteral("auditor")] = m_auditorEdit->text();
    root[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root[QStringLiteral("scannedPath")] = QString::fromStdWString(m_rootNode.path);
    root[QStringLiteral("totalFindings")] = static_cast<qint64>(m_findings.size());

    QJsonArray arr;
    for (const auto& f : m_findings) {
        QJsonObject obj;
        obj[QStringLiteral("severity")] = QString::fromStdWString(Engine::RiskFinding::severityToString(f.severity));
        obj[QStringLiteral("title")] = QString::fromStdWString(f.title);
        obj[QStringLiteral("path")] = QString::fromStdWString(f.affectedPath);
        obj[QStringLiteral("principal")] = QString::fromStdWString(f.affectedPrincipal);
        obj[QStringLiteral("description")] = QString::fromStdWString(f.description);
        obj[QStringLiteral("technicalDetail")] = QString::fromStdWString(f.technicalDetail);
        obj[QStringLiteral("remediation")] = QString::fromStdWString(f.remediationAdvice);
        obj[QStringLiteral("icaclsCommand")] = QString::fromStdWString(f.icaclsCommand);
        obj[QStringLiteral("powershellCommand")] = QString::fromStdWString(f.powershellCommand);
        arr.append(obj);
    }
    root[QStringLiteral("findings")] = arr;

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

bool ReportExportDialog::exportToPdf(const QString& targetPdfPath, const QString& htmlContent) const {
    QPdfWriter pdfWriter(targetPdfPath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    pdfWriter.setResolution(300);

    QTextDocument doc;
    doc.setHtml(htmlContent);
    doc.print(&pdfWriter);
    return true;
}

void ReportExportDialog::onExportClicked() {
    ReportFormat fmt = static_cast<ReportFormat>(m_formatCombo->currentData().toInt());

    QString filter;
    QString defaultExt;

    switch (fmt) {
        case ReportFormat::Html: filter = Language::GerenciadorTraducao::trText("Arquivos HTML (*.html)"); defaultExt = QStringLiteral(".html"); break;
        case ReportFormat::Pdf:  filter = Language::GerenciadorTraducao::trText("Arquivos PDF (*.pdf)");   defaultExt = QStringLiteral(".pdf"); break;
        case ReportFormat::Csv:  filter = Language::GerenciadorTraducao::trText("Arquivos CSV (*.csv)");   defaultExt = QStringLiteral(".csv"); break;
        case ReportFormat::Json: filter = Language::GerenciadorTraducao::trText("Arquivos JSON (*.json)"); defaultExt = QStringLiteral(".json"); break;
    }

    QString defaultFileName = QStringLiteral("Relatorio_ACL_Auditor_%1%2")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmm")), defaultExt);

    QString savePath = QFileDialog::getSaveFileName(
        this,
        Language::GerenciadorTraducao::trText("Salvar Relatório"),
        defaultFileName,
        filter
    );
    if (savePath.isEmpty()) return;

    bool ok = false;
    if (fmt == ReportFormat::Html) {
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << generateHtmlReport();
            file.close();
            ok = true;
        }
    } else if (fmt == ReportFormat::Pdf) {
        ok = exportToPdf(savePath, generateHtmlReport());
    } else if (fmt == ReportFormat::Csv) {
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << generateCsvReport();
            file.close();
            ok = true;
        }
    } else if (fmt == ReportFormat::Json) {
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << generateJsonReport();
            file.close();
            ok = true;
        }
    }

    if (ok) {
        QMessageBox::information(
            this,
            Language::GerenciadorTraducao::trText("Sucesso"),
            Language::GerenciadorTraducao::trText("Relatório gerado com sucesso em:\n%1").arg(savePath)
        );
        accept();
    } else {
        QMessageBox::warning(
            this,
            Language::GerenciadorTraducao::trText("Erro"),
            Language::GerenciadorTraducao::trText("Falha ao salvar relatório no local selecionado.")
        );
    }
}

} // namespace AclAuditor::UI
