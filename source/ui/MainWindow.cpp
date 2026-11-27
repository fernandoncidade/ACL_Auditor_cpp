#include "MainWindow.hpp"
#include "ThemeManager.hpp"
#include "VisualExplanationWidget.hpp"
#include "RiskDashboardWidget.hpp"
#include "ServicesSharesWidget.hpp"
#include "SnapshotDiffWidget.hpp"
#include "ReportExportDialog.hpp"
#include "RemediationDialog.hpp"
#include "AboutDialog.hpp"
#include "ManualDialog.hpp"
#include "TableEnhancer.hpp"
#include "Win32DwmHelper.hpp"
#include "../engine/NtfsSecurityUtils.hpp"
#include "../engine/RiskScanner.hpp"
#include "../engine/SnapshotEngine.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include "../utils/ExplorerIntegration.hpp"
#include "../utils/ApplicationPathUtils.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QTimer>
#include <QInputDialog>
#include <QEvent>
#include <QCloseEvent>

namespace AclAuditor::UI {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1240, 820);

    QIcon appIcon = Utils::ApplicationPathUtils::getAppIcon();
    if (!appIcon.isNull()) {
        setWindowIcon(appIcon);
    }

    Win32DwmHelper::applyMicaAndDarkMode(this);

    // Initialize database
    Engine::SnapshotEngine::instance().initializeDatabase();

    // Enable security privileges
    Engine::NtfsSecurityUtils::enableSecurityPrivileges();

    setupUi();
    setupMenus();
    setupStatusBar();

    retranslateUi();

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, [this](const QString&) {
        retranslateUi();
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MainWindow::applyTheme);

    applyTheme(ThemeManager::instance().isDark());

    // Start memory usage timer
    auto* memTimer = new QTimer(this);
    connect(memTimer, &QTimer::timeout, this, &MainWindow::updateStatusBarInfo);
    memTimer->start(3000);
    updateStatusBarInfo();
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // 1. Fechar explicitamente todas as janelas secundárias registradas (modeless / top-level)
    for (auto& winPtr : m_secondaryWindows) {
        if (winPtr) {
            winPtr->close();
        }
    }
    m_secondaryWindows.clear();

    // 2. Garantir o encerramento de quaisquer outras janelas de nível superior da aplicação
    const auto topLevels = QApplication::topLevelWidgets();
    for (QWidget* w : topLevels) {
        if (w && w != this && w->isVisible()) {
            w->close();
        }
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::registerSecondaryWindow(QWidget* window) {
    if (!window) return;
    if (!m_secondaryWindows.contains(window)) {
        m_secondaryWindows.append(window);
    }
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(6);

    m_mainTabWidget = new QTabWidget(central);

    // ========================================================================
    // TAB 1: Navegador de Permissões NTFS & Inspetor de ACEs
    // ========================================================================
    auto* navTab = new QWidget(m_mainTabWidget);
    auto* navLayout = new QVBoxLayout(navTab);
    navLayout->setContentsMargins(6, 6, 6, 6);
    navLayout->setSpacing(8);

    // Path selection row
    auto* pathRow = new QHBoxLayout();
    pathRow->setSpacing(6);

    m_lblPathPrefix = new QLabel(navTab);
    pathRow->addWidget(m_lblPathPrefix);

    m_pathEdit = new QLineEdit(QStringLiteral("C:\\"), navTab);
    pathRow->addWidget(m_pathEdit, 1);

    m_btnBrowse = new QPushButton(navTab);
    connect(m_btnBrowse, &QPushButton::clicked, this, &MainWindow::onBrowseFolderClicked);
    pathRow->addWidget(m_btnBrowse);

    m_btnScan = new QPushButton(navTab);
    m_btnScan->setStyleSheet(QStringLiteral("font-weight: bold; padding: 5px 14px;"));
    connect(m_btnScan, &QPushButton::clicked, this, &MainWindow::onScanClicked);
    pathRow->addWidget(m_btnScan);

    navLayout->addLayout(pathRow);

    // Quick Action Bar
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(8);

    m_btnExplain = new QPushButton(navTab);
    m_btnExplain->setStyleSheet(QStringLiteral("font-weight: bold; color: #0066CC;"));
    connect(m_btnExplain, &QPushButton::clicked, this, &MainWindow::onExplainCurrentObject);
    actionRow->addWidget(m_btnExplain);

    m_btnSnapshot = new QPushButton(navTab);
    connect(m_btnSnapshot, &QPushButton::clicked, this, &MainWindow::onTakeSnapshot);
    actionRow->addWidget(m_btnSnapshot);

    m_btnReport = new QPushButton(navTab);
    connect(m_btnReport, &QPushButton::clicked, this, &MainWindow::onOpenReportDialog);
    actionRow->addWidget(m_btnReport);

    actionRow->addStretch();
    navLayout->addLayout(actionRow);

    // Splitter: Left Tree, Right ACE Table
    auto* splitter = new QSplitter(Qt::Horizontal, navTab);

    // Left container
    auto* leftContainer = new QWidget(splitter);
    auto* leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    m_treeModel = new Model::AclTreeModel(this);
    m_treeView = new QTreeView(leftContainer);
    m_treeView->setModel(m_treeModel);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onTreeSelectionChanged);
    leftLayout->addWidget(m_treeView);
    splitter->addWidget(leftContainer);

    // Right container
    auto* rightContainer = new QWidget(splitter);
    auto* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);

    m_lblCurrentObjectInfo = new QLabel(rightContainer);
    m_lblCurrentObjectInfo->setWordWrap(true);
    m_lblCurrentObjectInfo->setStyleSheet(QStringLiteral("font-weight: bold; padding: 4px; background: palette(alternate-base); border-radius: 4px;"));
    rightLayout->addWidget(m_lblCurrentObjectInfo);

    m_aceSearchEdit = new QLineEdit(rightContainer);
    m_aceSearchEdit->setClearButtonEnabled(true);
    m_aceSearchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Buscar nas permissões (ACEs)..."));
    connect(m_aceSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (m_aceProxyModel) {
            m_aceProxyModel->setFilterPattern(text);
        }
    });
    rightLayout->addWidget(m_aceSearchEdit);

    m_aceModel = new Model::AceTableModel(this);
    m_aceProxyModel = new MultiColumnSortFilterProxyModel(this);
    m_aceProxyModel->setSourceModel(m_aceModel);

    m_aceTableView = new QTableView(rightContainer);
    TableEnhancer::enhanceTableView(
        m_aceTableView,
        m_aceProxyModel,
        QStringLiteral("auditoria_permissoes_aces"),
        { 130, 220, 180, 200, 220, 110 }
    );
    rightLayout->addWidget(m_aceTableView, 1);
    splitter->addWidget(rightContainer);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 4);
    navLayout->addWidget(splitter, 1);

    m_mainTabWidget->addTab(navTab, QStringLiteral("Explorador de Permissões NTFS"));

    // ========================================================================
    // TAB 2: Acesso Efetivo & Explicação Visual
    // ========================================================================
    m_visualExplanationWidget = new VisualExplanationWidget(m_mainTabWidget);
    m_mainTabWidget->addTab(m_visualExplanationWidget, QStringLiteral("Árvore Explicativa de Acesso Efetivo"));

    // ========================================================================
    // TAB 3: Scanner de Riscos & Conformidade
    // ========================================================================
    m_riskDashboardWidget = new RiskDashboardWidget(m_mainTabWidget);
    connect(m_riskDashboardWidget, &RiskDashboardWidget::openRemediationPlanRequested, this, &MainWindow::onOpenRemediationDialog);
    m_mainTabWidget->addTab(m_riskDashboardWidget, QStringLiteral("Dashboard de Riscos & Conformidade"));

    // ========================================================================
    // TAB 4: Serviços & Compartilhamentos SMB
    // ========================================================================
    m_servicesSharesWidget = new ServicesSharesWidget(m_mainTabWidget);
    m_mainTabWidget->addTab(m_servicesSharesWidget, QStringLiteral("Serviços & Compartilhamentos SMB"));

    // ========================================================================
    // TAB 5: Comparação Temporal de Snapshots
    // ========================================================================
    m_snapshotDiffWidget = new SnapshotDiffWidget(m_mainTabWidget);
    m_mainTabWidget->addTab(m_snapshotDiffWidget, QStringLiteral("Comparação Temporal (Snapshots)"));

    rootLayout->addWidget(m_mainTabWidget, 1);
    setCentralWidget(central);
}

void MainWindow::setupMenus() {
    auto* bar = menuBar();

    // Menu Arquivo
    m_menuArquivo = bar->addMenu(QStringLiteral("&Arquivo"));
    m_actionBrowse = m_menuArquivo->addAction(QStringLiteral("Selecionar Pasta para Auditoria..."), QKeySequence::Open, this, &MainWindow::onBrowseFolderClicked);
    m_actionScan = m_menuArquivo->addAction(QStringLiteral("Escanear Diretório"), QKeySequence(Qt::Key_F5), this, &MainWindow::onScanClicked);
    m_menuArquivo->addSeparator();
    m_actionSnapshot = m_menuArquivo->addAction(QStringLiteral("Salvar Snapshot Atual..."), QKeySequence::Save, this, &MainWindow::onTakeSnapshot);
    m_actionReport = m_menuArquivo->addAction(QStringLiteral("Exportar Relatório Executivo (PDF, HTML, CSV, JSON)..."), this, &MainWindow::onOpenReportDialog);
    m_menuArquivo->addSeparator();
    m_actionExit = m_menuArquivo->addAction(QStringLiteral("Sair"), QKeySequence::Quit, this, &QWidget::close);

    // Menu Auditoria
    m_menuAuditoria = bar->addMenu(QStringLiteral("&Auditoria"));
    m_actionExplain = m_menuAuditoria->addAction(QStringLiteral("Explicar Cadeia de Acesso Efetivo"), this, &MainWindow::onExplainCurrentObject);
    m_actionRemediation = m_menuAuditoria->addAction(QStringLiteral("Plano de Remediação (icacls / PowerShell)..."), this, &MainWindow::onOpenRemediationDialog);
    m_menuAuditoria->addSeparator();
    m_actionAuditServices = m_menuAuditoria->addAction(QStringLiteral("Auditar Serviços do Windows"), m_servicesSharesWidget, &ServicesSharesWidget::scanServices);
    m_actionAuditShares = m_menuAuditoria->addAction(QStringLiteral("Auditar Compartilhamentos SMB"), m_servicesSharesWidget, &ServicesSharesWidget::scanShares);

    // Menu Configurações & Idiomas
    m_menuConfig = bar->addMenu(QStringLiteral("&Configurações"));
    m_menuIdiomas = m_menuConfig->addMenu(QStringLiteral("&Idiomas"));

    m_langActionGroup = new QActionGroup(this);
    m_langActionGroup->setExclusive(true);

    m_actionPtBr = m_menuIdiomas->addAction(QStringLiteral("Português (Brasil)"));
    m_actionPtBr->setCheckable(true);
    m_langActionGroup->addAction(m_actionPtBr);
    connect(m_actionPtBr, &QAction::triggered, this, []() {
        Language::GerenciadorTraducao::instance().definir_idioma(QStringLiteral("pt_BR"));
    });

    m_actionEnUs = m_menuIdiomas->addAction(QStringLiteral("English (United States)"));
    m_actionEnUs->setCheckable(true);
    m_langActionGroup->addAction(m_actionEnUs);
    connect(m_actionEnUs, &QAction::triggered, this, []() {
        Language::GerenciadorTraducao::instance().definir_idioma(QStringLiteral("en_US"));
    });

    // Menu Ferramentas
    m_menuFerramentas = bar->addMenu(QStringLiteral("&Ferramentas"));
    m_actionExplorerIntegration = m_menuFerramentas->addAction(QStringLiteral("Alternar Integração com Menu de Contexto do Windows Explorer"), this, &MainWindow::onToggleExplorerIntegration);
    m_menuFerramentas->addSeparator();
    m_actionTheme = m_menuFerramentas->addAction(QStringLiteral("Atualizar Tema do Sistema Operacional"), this, []() {
        ThemeManager::instance().refreshTheme();
    });

    // Menu Ajuda
    m_menuAjuda = bar->addMenu(QStringLiteral("A&juda"));
    m_actionManual = m_menuAjuda->addAction(QStringLiteral("Manual de Instruções"), QKeySequence::HelpContents, this, &MainWindow::onOpenManual);
    m_actionAbout = m_menuAjuda->addAction(QStringLiteral("Sobre o ACL Auditor"), this, &MainWindow::onOpenAbout);
}

void MainWindow::setupStatusBar() {
    auto* bar = statusBar();

    m_statusElevated = new QLabel(this);
    bool isElevated = Engine::NtfsSecurityUtils::isProcessElevated();
    m_statusElevated->setText(isElevated ? QStringLiteral("[Administrador: SIM]") : QStringLiteral("[Usuário Padrão]"));
    m_statusElevated->setStyleSheet(isElevated ? QStringLiteral("color: #28A745; font-weight: bold;") : QStringLiteral("color: #FD7E14;"));
    bar->addWidget(m_statusElevated);

    m_statusObjectsCount = new QLabel(this);
    bar->addWidget(m_statusObjectsCount, 1);
    m_statusState = StatusState::Ready;
    updateStatusMessage();

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(180);
    m_progressBar->setVisible(false);
    bar->addPermanentWidget(m_progressBar);

    m_statusMemory = new QLabel(this);
    bar->addPermanentWidget(m_statusMemory);
    updateStatusBarInfo();
}

void MainWindow::retranslateUi() {
    setWindowTitle(Language::GerenciadorTraducao::trText("ACL Auditor — Analisador Visual de Permissões e Conformidade NTFS"));

    // Menus
    if (m_menuArquivo) m_menuArquivo->setTitle(Language::GerenciadorTraducao::trText("&Arquivo"));
    if (m_actionBrowse) m_actionBrowse->setText(Language::GerenciadorTraducao::trText("Selecionar Pasta para Auditoria..."));
    if (m_actionScan) m_actionScan->setText(Language::GerenciadorTraducao::trText("Escanear Diretório"));
    if (m_actionSnapshot) m_actionSnapshot->setText(Language::GerenciadorTraducao::trText("Salvar Snapshot Atual..."));
    if (m_actionReport) m_actionReport->setText(Language::GerenciadorTraducao::trText("Exportar Relatório Executivo (PDF, HTML, CSV, JSON)..."));
    if (m_actionExit) m_actionExit->setText(Language::GerenciadorTraducao::trText("Sair"));

    if (m_menuAuditoria) m_menuAuditoria->setTitle(Language::GerenciadorTraducao::trText("&Auditoria"));
    if (m_actionExplain) m_actionExplain->setText(Language::GerenciadorTraducao::trText("Explicar Cadeia de Acesso Efetivo"));
    if (m_actionRemediation) m_actionRemediation->setText(Language::GerenciadorTraducao::trText("Plano de Remediação (icacls / PowerShell)..."));
    if (m_actionAuditServices) m_actionAuditServices->setText(Language::GerenciadorTraducao::trText("Auditar Serviços do Windows"));
    if (m_actionAuditShares) m_actionAuditShares->setText(Language::GerenciadorTraducao::trText("Auditar Compartilhamentos SMB"));

    if (m_menuConfig) m_menuConfig->setTitle(Language::GerenciadorTraducao::trText("&Configurações"));
    if (m_menuIdiomas) m_menuIdiomas->setTitle(Language::GerenciadorTraducao::trText("&Idiomas"));
    if (m_actionPtBr) m_actionPtBr->setText(Language::GerenciadorTraducao::trText("Português (Brasil)"));
    if (m_actionEnUs) m_actionEnUs->setText(Language::GerenciadorTraducao::trText("English (United States)"));

    const QString curLang = Language::GerenciadorTraducao::instance().obter_idioma_atual();
    if (m_actionPtBr) m_actionPtBr->setChecked(curLang == QStringLiteral("pt_BR"));
    if (m_actionEnUs) m_actionEnUs->setChecked(curLang == QStringLiteral("en_US"));

    if (m_menuFerramentas) m_menuFerramentas->setTitle(Language::GerenciadorTraducao::trText("&Ferramentas"));
    if (m_actionExplorerIntegration) m_actionExplorerIntegration->setText(Language::GerenciadorTraducao::trText("Alternar Integração com Menu de Contexto do Windows Explorer"));
    if (m_actionTheme) m_actionTheme->setText(Language::GerenciadorTraducao::trText("Atualizar Tema do Sistema Operacional"));

    if (m_menuAjuda) m_menuAjuda->setTitle(Language::GerenciadorTraducao::trText("A&juda"));
    if (m_actionManual) m_actionManual->setText(Language::GerenciadorTraducao::trText("Manual de Instruções"));
    if (m_actionAbout) m_actionAbout->setText(Language::GerenciadorTraducao::trText("Sobre o ACL Auditor"));

    // Tabs
    if (m_mainTabWidget) {
        m_mainTabWidget->setTabText(0, Language::GerenciadorTraducao::trText("Explorador de Permissões NTFS"));
        m_mainTabWidget->setTabText(1, Language::GerenciadorTraducao::trText("Árvore Explicativa de Acesso Efetivo"));
        m_mainTabWidget->setTabText(2, Language::GerenciadorTraducao::trText("Dashboard de Riscos & Conformidade"));
        m_mainTabWidget->setTabText(3, Language::GerenciadorTraducao::trText("Serviços & Compartilhamentos SMB"));
        m_mainTabWidget->setTabText(4, Language::GerenciadorTraducao::trText("Comparação Temporal (Snapshots)"));
    }

    // Buttons and labels
    if (m_lblPathPrefix) m_lblPathPrefix->setText(Language::GerenciadorTraducao::trText("Diretório para Auditoria:"));
    if (m_btnBrowse) m_btnBrowse->setText(Language::GerenciadorTraducao::trText("Procurar..."));
    if (m_btnScan) m_btnScan->setText(Language::GerenciadorTraducao::trText("Escanear"));
    if (m_btnExplain) m_btnExplain->setText(Language::GerenciadorTraducao::trText("Explicar Cadeia de Acesso Efetivo"));
    if (m_btnSnapshot) m_btnSnapshot->setText(Language::GerenciadorTraducao::trText("Salvar Snapshot Atual..."));
    if (m_btnReport) m_btnReport->setText(Language::GerenciadorTraducao::trText("Exportar Relatório"));

    if (m_aceSearchEdit) {
        m_aceSearchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Buscar nas permissões (ACEs)..."));
    }

    if (m_lblCurrentObjectInfo) {
        if (m_selectedObjectPath.isEmpty()) {
            m_lblCurrentObjectInfo->setText(Language::GerenciadorTraducao::trText("Selecione um arquivo ou diretório primeiro."));
        } else if (m_treeModel && m_treeView) {
            const QModelIndex curIdx = m_treeView->currentIndex();
            const Engine::FileSecurityNode* node = m_treeModel->getNodeAt(curIdx);
            if (node) {
                const QString inheritanceStatus = node->security.isInheritanceBlocked ?
                    Language::GerenciadorTraducao::trText("Herança de Permissões: BLOQUEADA (SE_DACL_PROTECTED)") :
                    Language::GerenciadorTraducao::trText("Herança de Permissões: ATIVA (Herdando da pasta pai)");

                m_lblCurrentObjectInfo->setText(QStringLiteral(
                    "%1: %2 | %3: %4 | %5 | Total: %6 ACEs"
                ).arg(
                    Language::GerenciadorTraducao::trText("Caminho do Objeto"),
                    QString::fromStdWString(node->path),
                    Language::GerenciadorTraducao::trText("Proprietário"),
                    QString::fromStdWString(node->security.owner.getDisplayName()),
                    inheritanceStatus,
                    QString::number(node->security.dacl.size())
                ));
            }
        }
    }

    if (m_treeModel) m_treeModel->refreshTranslations();
    if (m_aceModel) m_aceModel->refreshTranslations();
    if (m_visualExplanationWidget) m_visualExplanationWidget->retranslateUi();
    if (m_riskDashboardWidget) m_riskDashboardWidget->retranslateUi();
    if (m_servicesSharesWidget) m_servicesSharesWidget->retranslateUi();
    if (m_snapshotDiffWidget) m_snapshotDiffWidget->retranslateUi();

    // Status bar
    bool isElevated = Engine::NtfsSecurityUtils::isProcessElevated();
    if (m_statusElevated) {
        m_statusElevated->setText(isElevated ? Language::GerenciadorTraducao::trText("[Administrador: SIM]") : Language::GerenciadorTraducao::trText("[Usuário Padrão]"));
    }
    updateStatusBarInfo();
    updateStatusMessage();
    applyTheme(ThemeManager::instance().isDark());
}

void MainWindow::updateStatusBarInfo() {
    if (m_statusMemory) {
        m_statusMemory->setText(QStringLiteral("%1: %2")
            .arg(Language::GerenciadorTraducao::trText("Memória"))
            .arg(Win32DwmHelper::getProcessMemoryUsageString()));
    }
}

void MainWindow::updateStatusMessage() {
    if (!m_statusObjectsCount) return;

    switch (m_statusState) {
        case StatusState::Ready:
            m_statusObjectsCount->setText(Language::GerenciadorTraducao::trText("Pronto"));
            break;
        case StatusState::DirectorySelected:
            m_statusObjectsCount->setText(
                Language::GerenciadorTraducao::trText("Diretório selecionado: %1 (clique em Escanear)").arg(m_lastStatusPath)
            );
            break;
        case StatusState::Scanning:
            m_statusObjectsCount->setText(
                Language::GerenciadorTraducao::trText("Escaneando diretório: %1...").arg(m_lastStatusPath)
            );
            break;
        case StatusState::ScanCompleted:
            m_statusObjectsCount->setText(
                Language::GerenciadorTraducao::trText("Varredura concluída. %1 itens auditados.").arg(m_lastAuditedCount)
            );
            break;
    }
}

void MainWindow::onBrowseFolderClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        Language::GerenciadorTraducao::trText("Selecionar Pasta para Auditoria..."),
        m_pathEdit->text()
    );
    if (!dir.isEmpty()) {
        const QString nativeDir = QDir::toNativeSeparators(dir);
        m_pathEdit->setText(nativeDir);
        m_statusState = StatusState::DirectorySelected;
        m_lastStatusPath = nativeDir;
        updateStatusMessage();
    }
}

void MainWindow::openPathForAudit(const QString& path) {
    if (!path.isEmpty()) {
        m_pathEdit->setText(QDir::toNativeSeparators(path));
        onScanClicked();
    }
}

void MainWindow::onScanClicked() {
    QString path = m_pathEdit->text().trimmed();
    if (path.isEmpty()) return;

    if (m_reportDialog) {
        m_reportDialog->close();
    }
    if (m_remediationDialog) {
        m_remediationDialog->close();
    }

    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // Busy animation
    m_statusState = StatusState::Scanning;
    m_lastStatusPath = path;
    updateStatusMessage();
    qApp->processEvents();

    std::wstring rootW = path.toStdWString();
    size_t totalScanned = 0;
    m_currentRootNode = Engine::NtfsSecurityUtils::scanDirectoryTree(
        rootW,
        0,
        true,
        [this, &totalScanned](const std::wstring& currentPath, size_t count) {
            totalScanned = count;
            if (count % 15 == 0) {
                m_statusObjectsCount->setText(QStringLiteral("%1 (%2)").arg(count).arg(QString::fromStdWString(currentPath).right(35)));
                qApp->processEvents();
            }
        }
    );

    // Evaluate risks
    m_currentFindings = Engine::RiskScanner::evaluateTreeRisks(m_currentRootNode);

    // Populate models
    m_treeModel->setRootNode(m_currentRootNode);
    m_riskDashboardWidget->setFindings(m_currentFindings);

    m_progressBar->setVisible(false);

    static auto countTotalNodes = [](auto& self, const Engine::FileSecurityNode& node) -> size_t {
        size_t c = 1;
        for (const auto& ch : node.children) {
            c += self(self, ch);
        }
        return c;
    };
    size_t auditedCount = countTotalNodes(countTotalNodes, m_currentRootNode);
    if (totalScanned > auditedCount) {
        auditedCount = totalScanned;
    }
    if (auditedCount == 0 && !m_currentFindings.empty()) {
        auditedCount = m_currentFindings.size();
    }

    m_statusState = StatusState::ScanCompleted;
    m_lastAuditedCount = auditedCount;
    updateStatusMessage();

    // Select root node in tree view
    QModelIndex firstIdx = m_treeModel->index(0, 0);
    if (firstIdx.isValid()) {
        m_treeView->setCurrentIndex(firstIdx);
    }
}

void MainWindow::onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
    const Engine::FileSecurityNode* node = m_treeModel->getNodeAt(current);
    if (!node) return;

    m_selectedObjectPath = QString::fromStdWString(node->path);
    m_aceModel->setAces(node->security.dacl);

    const QString inheritanceStatus = node->security.isInheritanceBlocked ?
        Language::GerenciadorTraducao::trText("Herança de Permissões: BLOQUEADA (SE_DACL_PROTECTED)") :
        Language::GerenciadorTraducao::trText("Herança de Permissões: ATIVA (Herdando da pasta pai)");

    m_lblCurrentObjectInfo->setText(QStringLiteral(
        "%1: %2 | %3: %4 | %5 | Total: %6 ACEs"
    ).arg(
        Language::GerenciadorTraducao::trText("Caminho do Objeto"),
        QString::fromStdWString(node->path),
        Language::GerenciadorTraducao::trText("Proprietário"),
        QString::fromStdWString(node->security.owner.getDisplayName()),
        inheritanceStatus,
        QString::number(node->security.dacl.size())
    ));

    // Also update visual explanation target
    m_visualExplanationWidget->setTargetObject(m_selectedObjectPath);
    applyTheme(ThemeManager::instance().isDark());
}

void MainWindow::onExplainCurrentObject() {
    if (m_selectedObjectPath.isEmpty()) {
        m_selectedObjectPath = m_pathEdit->text();
    }
    m_visualExplanationWidget->setTargetObject(m_selectedObjectPath);
    m_mainTabWidget->setCurrentWidget(m_visualExplanationWidget);
}

void MainWindow::onTakeSnapshot() {
    if (m_currentRootNode.path.empty()) {
        QMessageBox::information(this,
            Language::GerenciadorTraducao::trText("Aviso"),
            Language::GerenciadorTraducao::trText("Selecione um arquivo ou diretório primeiro."));
        return;
    }

    bool ok = false;
    QString title = QInputDialog::getText(
        this,
        Language::GerenciadorTraducao::trText("Salvar Snapshot Atual..."),
        QStringLiteral("Título do Snapshot:"),
        QLineEdit::Normal,
        QStringLiteral("Snapshot - %1").arg(QString::fromStdWString(m_currentRootNode.fileName)),
        &ok
    );

    if (ok && !title.isEmpty()) {
        auto rec = Engine::SnapshotEngine::instance().createSnapshot(
            title.toStdWString(),
            m_currentRootNode.path,
            m_currentRootNode,
            m_currentFindings
        );

        if (Engine::SnapshotEngine::instance().saveSnapshot(rec)) {
            m_snapshotDiffWidget->refreshSnapshotList();
            QMessageBox::information(this,
                Language::GerenciadorTraducao::trText("Sucesso"),
                Language::GerenciadorTraducao::trText("Snapshot capturado com sucesso!\nID: %1\nItens: %2\nHash SHA-256: %3")
                    .arg(QString::fromStdWString(rec.id))
                    .arg(rec.totalFiles + rec.totalDirectories)
                    .arg(QString::fromStdString(rec.sha256Hash)));
        } else {
            QMessageBox::warning(this,
                Language::GerenciadorTraducao::trText("Erro"),
                Language::GerenciadorTraducao::trText("Falha ao salvar snapshot no banco SQLite."));
        }
    }
}

void MainWindow::onOpenReportDialog() {
    if (m_currentRootNode.path.empty()) {
        QMessageBox::information(this,
            Language::GerenciadorTraducao::trText("Aviso"),
            Language::GerenciadorTraducao::trText("Selecione um arquivo ou diretório primeiro."));
        return;
    }

    if (m_reportDialog) {
        m_reportDialog->show();
        m_reportDialog->raise();
        m_reportDialog->activateWindow();
        return;
    }

    m_reportDialog = new ReportExportDialog(m_currentRootNode, m_currentFindings, nullptr);
    registerSecondaryWindow(m_reportDialog);
    m_reportDialog->show();
    m_reportDialog->raise();
    m_reportDialog->activateWindow();
}

void MainWindow::onOpenRemediationDialog() {
    if (m_currentFindings.empty()) {
        QMessageBox::information(this,
            Language::GerenciadorTraducao::trText("Aviso"),
            Language::GerenciadorTraducao::trText("Nenhum risco ou vulnerabilidade detectado no escopo auditado."));
        return;
    }

    if (m_remediationDialog) {
        m_remediationDialog->show();
        m_remediationDialog->raise();
        m_remediationDialog->activateWindow();
        return;
    }

    m_remediationDialog = new RemediationDialog(m_currentRootNode.path, m_currentFindings, nullptr);
    registerSecondaryWindow(m_remediationDialog);
    m_remediationDialog->show();
    m_remediationDialog->raise();
    m_remediationDialog->activateWindow();
}

void MainWindow::onToggleExplorerIntegration() {
    bool registered = Utils::ExplorerIntegration::isContextMenuRegistered();
    if (registered) {
        if (QMessageBox::question(this,
            Language::GerenciadorTraducao::trText("Integração com o Explorer Desativada"),
            Language::GerenciadorTraducao::trText("A opção foi removida do menu de contexto de pastas.")) == QMessageBox::Yes) {
            Utils::ExplorerIntegration::unregisterContextMenu();
            QMessageBox::information(this,
                Language::GerenciadorTraducao::trText("Sucesso"),
                Language::GerenciadorTraducao::trText("Integração com o Explorer Desativada"));
        }
    } else {
        if (QMessageBox::question(this,
            Language::GerenciadorTraducao::trText("Integração com o Explorer Ativada"),
            Language::GerenciadorTraducao::trText("A opção 'Auditar Permissões NTFS' foi adicionada ao menu de contexto de pastas.")) == QMessageBox::Yes) {
            Utils::ExplorerIntegration::registerContextMenu();
            QMessageBox::information(this,
                Language::GerenciadorTraducao::trText("Sucesso"),
                Language::GerenciadorTraducao::trText("Integração com o Explorer Ativada"));
        }
    }
}

void MainWindow::onOpenAbout() {
    if (m_aboutDialog) {
        m_aboutDialog->show();
        m_aboutDialog->raise();
        m_aboutDialog->activateWindow();
        return;
    }

    m_aboutDialog = new AboutDialog(nullptr);
    registerSecondaryWindow(m_aboutDialog);
    m_aboutDialog->show();
    m_aboutDialog->raise();
    m_aboutDialog->activateWindow();
}

void MainWindow::onOpenManual() {
    if (m_manualDialog) {
        m_manualDialog->show();
        m_manualDialog->raise();
        m_manualDialog->activateWindow();
        return;
    }

    m_manualDialog = new ManualDialog(nullptr);
    registerSecondaryWindow(m_manualDialog);
    m_manualDialog->show();
    m_manualDialog->raise();
    m_manualDialog->activateWindow();
}

void MainWindow::applyTheme(bool isDark) {
    Win32DwmHelper::applyTitleBarTheme(this, isDark);
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();
    setPalette(pal);

    if (m_btnScan) {
        m_btnScan->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #0078d7; color: #ffffff; border: 1px solid #005a9e; border-radius: 4px; font-weight: bold; padding: 5px 14px; } QPushButton:hover { background-color: #1a88e1; }") :
            QStringLiteral("QPushButton { background-color: #0066cc; color: #ffffff; border: 1px solid #0052a3; border-radius: 4px; font-weight: bold; padding: 5px 14px; } QPushButton:hover { background-color: #0073e6; }"));
    }

    if (m_btnExplain) {
        m_btnExplain->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #1e293b; color: #38bdf8; border: 1px solid #334155; border-radius: 4px; font-weight: bold; padding: 5px 12px; } QPushButton:hover { background-color: #273549; }") :
            QStringLiteral("QPushButton { background-color: #f0f9ff; color: #0284c7; border: 1px solid #bae6fd; border-radius: 4px; font-weight: bold; padding: 5px 12px; } QPushButton:hover { background-color: #e0f2fe; }"));
    }

    auto styleSecBtn = [isDark](QPushButton* btn) {
        if (!btn) return;
        btn->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #2d3034; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 5px 12px; } QPushButton:hover { background-color: #383c42; }") :
            QStringLiteral("QPushButton { background-color: #f1f5f9; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 4px; padding: 5px 12px; } QPushButton:hover { background-color: #e2e8f0; }"));
    };
    styleSecBtn(m_btnBrowse);
    styleSecBtn(m_btnSnapshot);
    styleSecBtn(m_btnReport);

    if (m_lblCurrentObjectInfo) {
        if (m_selectedObjectPath.isEmpty()) {
            m_lblCurrentObjectInfo->setStyleSheet(isDark ?
                QStringLiteral("QLabel { color: #94a3b8; font-weight: bold; padding: 6px; background-color: #1e293b; border: 1px solid #334155; border-radius: 4px; }") :
                QStringLiteral("QLabel { color: #64748b; font-weight: bold; padding: 6px; background-color: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 4px; }"));
        } else {
            m_lblCurrentObjectInfo->setStyleSheet(isDark ?
                QStringLiteral("QLabel { color: #f0f2f5; font-weight: bold; padding: 6px; background-color: #1e293b; border: 1px solid #334155; border-radius: 4px; }") :
                QStringLiteral("QLabel { color: #0f172a; font-weight: bold; padding: 6px; background-color: #f8fafc; border: 1px solid #cbd5e1; border-radius: 4px; }"));
        }
    }

    if (m_lblPathPrefix) {
        m_lblPathPrefix->setStyleSheet(isDark ? QStringLiteral("color: #f0f2f5; font-weight: 500;")
                                              : QStringLiteral("color: #0f172a; font-weight: 500;"));
    }
    if (m_pathEdit) {
        m_pathEdit->setStyleSheet(isDark ?
            QStringLiteral("QLineEdit { background-color: #18191b; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px 8px; }") :
            QStringLiteral("QLineEdit { background-color: #ffffff; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px 8px; }"));
    }
    if (m_aceSearchEdit) {
        m_aceSearchEdit->setStyleSheet(isDark ?
            QStringLiteral("QLineEdit { background-color: #18191b; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px 8px; }") :
            QStringLiteral("QLineEdit { background-color: #ffffff; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px 8px; }"));
    }

    if (m_statusObjectsCount) {
        m_statusObjectsCount->setStyleSheet(isDark ? QStringLiteral("color: #cbd5e1;") : QStringLiteral("color: #334155;"));
    }

    if (m_visualExplanationWidget) m_visualExplanationWidget->applyTheme(isDark);
    if (m_riskDashboardWidget) m_riskDashboardWidget->applyTheme(isDark);
    if (m_servicesSharesWidget) m_servicesSharesWidget->applyTheme(isDark);
    if (m_snapshotDiffWidget) m_snapshotDiffWidget->applyTheme(isDark);
}

} // namespace AclAuditor::UI
