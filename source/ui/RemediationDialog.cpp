#include "RemediationDialog.hpp"
#include "Win32DwmHelper.hpp"
#include "ThemeManager.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include "../utils/ApplicationPathUtils.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QEvent>

namespace AclAuditor::UI {

RemediationDialog::RemediationDialog(
    const std::wstring& rootPath,
    const std::vector<Engine::RiskFinding>& findings,
    QWidget* parent
) : QDialog(parent) {
    const QIcon appIcon = Utils::ApplicationPathUtils::getAppIcon();
    if (!appIcon.isNull()) {
        setWindowIcon(appIcon);
    }
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    resize(780, 560);
    setMinimumSize(600, 420);

    Win32DwmHelper::applyMicaAndDarkMode(this);

    m_plan = Engine::RemediationPlanner::buildPlan(rootPath, findings, true);
    setupUi();
    retranslateUi();

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &RemediationDialog::retranslateUi);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](bool isDark) {
        Win32DwmHelper::applyTitleBarTheme(this, isDark);
    });
    Win32DwmHelper::applyTitleBarTheme(this, ThemeManager::instance().isDark());
}

void RemediationDialog::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void RemediationDialog::retranslateUi() {
    setWindowTitle(Language::GerenciadorTraducao::trText("Plano de Remediação e Endurecimento (Modo Somente Leitura)"));

    if (m_lblBannerTitle) {
        m_lblBannerTitle->setText(Language::GerenciadorTraducao::trText("MODO SOMENTE LEITURA - GOVERNANÇA E SEGURANÇA"));
    }
    if (m_lblBannerDesc) {
        m_lblBannerDesc->setText(Language::GerenciadorTraducao::trText(
            "O ACL Auditor não realiza alterações automáticas no sistema de arquivos. "
            "Abaixo estão os scripts técnicos gerados para que a equipe de infraestrutura e segurança "
            "possa revisar, auditar e aplicar as correções com backup e plano de reversão garantidos."
        ));
    }

    if (m_tabWidget) {
        m_tabWidget->setTabText(0, Language::GerenciadorTraducao::trText("Script PowerShell (.ps1)"));
        m_tabWidget->setTabText(1, Language::GerenciadorTraducao::trText("Script Batch / icacls (.bat)"));
        m_tabWidget->setTabText(2, Language::GerenciadorTraducao::trText("Relatório Técnico (.md)"));
        m_tabWidget->setTabText(3, Language::GerenciadorTraducao::trText("Script de Rollback (.bat)"));
    }

    if (m_btnCopy) {
        m_btnCopy->setText(Language::GerenciadorTraducao::trText("Copiar Código da Aba Atual"));
    }
    if (m_btnExport) {
        m_btnExport->setText(Language::GerenciadorTraducao::trText("Exportar Script Selecionado..."));
    }
    if (m_btnClose) {
        m_btnClose->setText(Language::GerenciadorTraducao::trText("Fechar"));
    }
}

void RemediationDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(10);

    // Warning / Safe Mode Banner
    auto* banner = new QFrame(this);
    banner->setFrameShape(QFrame::StyledPanel);
    banner->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #2D3748; border-left: 4px solid #3182CE; border-radius: 4px; padding: 10px; }"
    ));
    auto* bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(4, 4, 4, 4);

    m_lblBannerTitle = new QLabel(banner);
    m_lblBannerTitle->setStyleSheet(QStringLiteral("font-weight: bold; color: #63B3ED; font-size: 10pt;"));
    m_lblBannerDesc = new QLabel(banner);
    m_lblBannerDesc->setStyleSheet(QStringLiteral("color: #E2E8F0; font-size: 9pt;"));
    m_lblBannerDesc->setWordWrap(true);

    bannerLayout->addWidget(m_lblBannerTitle);
    bannerLayout->addWidget(m_lblBannerDesc);
    mainLayout->addWidget(banner);

    // Tab widget for scripts
    m_tabWidget = new QTabWidget(this);

    auto createEditor = [this](const std::wstring& content) -> QTextEdit* {
        auto* editor = new QTextEdit(this);
        editor->setReadOnly(true);
        editor->setFontFamily(QStringLiteral("Consolas, Courier New, monospace"));
        editor->setPlainText(QString::fromStdWString(content));
        return editor;
    };

    m_psEditor = createEditor(m_plan.powershellScript);
    m_icaclsEditor = createEditor(m_plan.icaclsScript);
    m_mdEditor = createEditor(m_plan.markdownReport);
    m_rollbackEditor = createEditor(m_plan.rollbackScript);

    m_tabWidget->addTab(m_psEditor, QString());
    m_tabWidget->addTab(m_icaclsEditor, QString());
    m_tabWidget->addTab(m_mdEditor, QString());
    m_tabWidget->addTab(m_rollbackEditor, QString());

    mainLayout->addWidget(m_tabWidget, 1);

    // Buttons
    auto* btnLayout = new QHBoxLayout();

    m_btnCopy = new QPushButton(this);
    connect(m_btnCopy, &QPushButton::clicked, this, &RemediationDialog::onCopyCurrentTab);
    btnLayout->addWidget(m_btnCopy);

    m_btnExport = new QPushButton(this);
    m_btnExport->setStyleSheet(QStringLiteral("font-weight: bold;"));
    connect(m_btnExport, &QPushButton::clicked, this, &RemediationDialog::onExportScript);
    btnLayout->addWidget(m_btnExport);

    btnLayout->addStretch();

    m_btnClose = new QPushButton(this);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_btnClose);

    mainLayout->addLayout(btnLayout);
}

void RemediationDialog::onCopyCurrentTab() {
    int idx = m_tabWidget->currentIndex();
    QString text;
    if (idx == 0) text = m_psEditor->toPlainText();
    else if (idx == 1) text = m_icaclsEditor->toPlainText();
    else if (idx == 2) text = m_mdEditor->toPlainText();
    else if (idx == 3) text = m_rollbackEditor->toPlainText();

    if (!text.isEmpty()) {
        QApplication::clipboard()->setText(text);
        QMessageBox::information(
            this,
            Language::GerenciadorTraducao::trText("Copiado"),
            Language::GerenciadorTraducao::trText("Script copiado para a área de transferência.")
        );
    }
}

void RemediationDialog::onExportScript() {
    int idx = m_tabWidget->currentIndex();
    QString filter;
    QString defaultName;
    std::wstring content;

    if (idx == 0) {
        filter = Language::GerenciadorTraducao::trText("Script PowerShell (*.ps1)");
        defaultName = QStringLiteral("remediar_permissoes_acl.ps1");
        content = m_plan.powershellScript;
    } else if (idx == 1) {
        filter = Language::GerenciadorTraducao::trText("Script Batch (*.bat)");
        defaultName = QStringLiteral("remediar_permissoes_icacls.bat");
        content = m_plan.icaclsScript;
    } else if (idx == 2) {
        filter = Language::GerenciadorTraducao::trText("Documento Markdown (*.md)");
        defaultName = QStringLiteral("plano_remediacao.md");
        content = m_plan.markdownReport;
    } else {
        filter = Language::GerenciadorTraducao::trText("Script Batch (*.bat)");
        defaultName = QStringLiteral("reverter_permissoes_rollback.bat");
        content = m_plan.rollbackScript;
    }

    QString savePath = QFileDialog::getSaveFileName(
        this,
        Language::GerenciadorTraducao::trText("Salvar Script"),
        defaultName,
        filter
    );
    if (!savePath.isEmpty()) {
        if (Engine::RemediationPlanner::exportScriptToFile(content, savePath.toStdWString())) {
            QMessageBox::information(
                this,
                Language::GerenciadorTraducao::trText("Sucesso"),
                Language::GerenciadorTraducao::trText("Script salvo com sucesso em:\n%1").arg(savePath)
            );
        } else {
            QMessageBox::warning(
                this,
                Language::GerenciadorTraducao::trText("Erro"),
                Language::GerenciadorTraducao::trText("Falha ao salvar o arquivo.")
            );
        }
    }
}

} // namespace AclAuditor::UI
