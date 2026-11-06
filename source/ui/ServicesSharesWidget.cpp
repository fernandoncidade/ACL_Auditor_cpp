#include "ServicesSharesWidget.hpp"
#include "ThemeManager.hpp"
#include "../engine/ServiceAuditor.hpp"
#include "../engine/SmbShareAuditor.hpp"
#include "../language/GerenciadorTraducao.hpp"

#include "TableEnhancer.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QEvent>

namespace AclAuditor::UI {

namespace {

QString translateServiceDisplayName(const std::wstring& serviceName, const std::wstring& displayName) {
    QString originalName = QString::fromStdWString(displayName).trimmed();
    if (originalName.isEmpty()) {
        originalName = QString::fromStdWString(serviceName).trimmed();
    }

    QString translated = Language::GerenciadorTraducao::trText(originalName);
    if (translated != originalName) {
        return translated;
    }

    const bool isEn = (Language::GerenciadorTraducao::instance().idiomaAtual() == Language::Idioma::InglesEstadosUnidos);
    const QString sName = QString::fromStdWString(serviceName).trimmed();

    struct ServiceNameMap {
        const char* serviceName;
        const char* ptName;
        const char* enName;
    };
    static const ServiceNameMap s_knownServices[] = {
        {"Spooler", "Spooler de Impressão", "Print Spooler"},
        {"LanmanServer", "Servidor", "Server"},
        {"LanmanWorkstation", "Estação de trabalho", "Workstation"},
        {"mpssvc", "Firewall do Windows Defender", "Windows Defender Firewall"},
        {"Schedule", "Agendador de Tarefas", "Task Scheduler"},
        {"wuauserv", "Windows Update", "Windows Update"},
        {"BITS", "Serviço de Transferência Inteligente em Segundo Plano", "Background Intelligent Transfer Service"},
        {"EventLog", "Log de Eventos do Windows", "Windows Event Log"},
        {"Dhcp", "Cliente DHCP", "DHCP Client"},
        {"Dnscache", "Cliente DNS", "DNS Client"},
        {"RpcSs", "Chamada de Procedimento Remoto (RPC)", "Remote Procedure Call (RPC)"},
        {"CryptSvc", "Serviços de Criptografia", "Cryptographic Services"},
        {"Winmgmt", "Instrumentação de Gerenciamento do Windows", "Windows Management Instrumentation"},
        {"Themes", "Temas", "Themes"},
        {"Audiosrv", "Áudio do Windows", "Windows Audio"},
        {"AudioEndpointBuilder", "Construtor de Pontos de Extremidade de Áudio do Windows", "Windows Audio Endpoint Builder"},
        {"Appinfo", "Informações sobre Aplicativos", "Application Information"},
        {"AppIDSvc", "Identidade do Aplicativo", "Application Identity"},
        {"AppReadiness", "Preparação de Aplicativos", "App Readiness"},
        {"AppMgmt", "Gerenciamento de Aplicativo", "Application Management"},
        {"AppXSvc", "Serviço de Implantação AppX (AppXSVC)", "AppX Deployment Service (AppXSVC)"},
        {"ALG", "Serviço Gateway de Camada de Aplicativo", "Application Layer Gateway Service"},
        {"W32Time", "Horário do Windows", "Windows Time"},
        {"SSDPSRV", "Descoberta SSDP", "SSDP Discovery"},
        {"SamSs", "Gerenciador de Contas de Segurança", "Security Accounts Manager"},
        {"PlugPlay", "Plug and Play", "Plug and Play"},
        {"PolicyAgent", "Agente de Política IPsec", "IPsec Policy Agent"},
        {"Netman", "Conexões de Rede", "Network Connections"},
        {"NlaSvc", "Reconhecimento de Locais de Rede", "Network Location Awareness"},
        {"netprofm", "Serviço de Lista de Redes", "Network List Service"},
        {"ProfSvc", "Serviço de Perfil de Usuário", "User Profile Service"},
        {"UserManager", "Gerenciador de Usuários", "User Manager"},
        {"SstpSvc", "Serviço SSTP", "Secure Socket Tunneling Protocol Service"},
        {"TermService", "Serviços de Área de Trabalho Remota", "Remote Desktop Services"},
        {"TrkWks", "Cliente de Acompanhamento de Link Distribuído", "Distributed Link Tracking Client"},
        {"WsmSvc", "Gerenciamento Remoto do Windows (WS-Management)", "Windows Remote Management (WS-Management)"},
        {"WinDefend", "Serviço Antivírus do Microsoft Defender", "Microsoft Defender Antivirus Service"},
        {"SecurityHealthService", "Serviço de Segurança do Windows", "Windows Security Service"},
        {"wscsvc", "Central de Segurança", "Security Center"},
        {"DoSvc", "Otimização de Entrega", "Delivery Optimization"},
        {"DiagTrack", "Experiências do Usuário Conectado e Telemetria", "Connected User Experiences and Telemetry"},
        {"DPS", "Serviço de Política de Diagnóstico", "Diagnostic Policy Service"},
        {"lmhosts", "Auxiliar NetBIOS TCP/IP", "TCP/IP NetBIOS Helper"},
        {"nsi", "Serviço de Interface de Repositório de Rede", "Network Store Interface Service"},
        {"WdiServiceHost", "Host de Serviço de Diagnóstico", "Diagnostic Service Host"},
        {"WdiSystemHost", "Host de Sistema de Diagnóstico", "Diagnostic System Host"}
    };

    for (const auto& item : s_knownServices) {
        if (sName.compare(QLatin1String(item.serviceName), Qt::CaseInsensitive) == 0 ||
            originalName.compare(QString::fromUtf8(item.ptName), Qt::CaseInsensitive) == 0 ||
            originalName.compare(QString::fromUtf8(item.enName), Qt::CaseInsensitive) == 0) {
            return isEn ? QString::fromUtf8(item.enName) : QString::fromUtf8(item.ptName);
        }
    }

    return originalName;
}

QString translateShareComment(const std::wstring& comment) {
    QString original = QString::fromStdWString(comment).trimmed();
    if (original.isEmpty()) {
        return QString();
    }

    QString translated = Language::GerenciadorTraducao::trText(original);
    if (translated != original) {
        return translated;
    }

    const bool isEn = (Language::GerenciadorTraducao::instance().idiomaAtual() == Language::Idioma::InglesEstadosUnidos);

    struct CommentMap {
        const char* ptText;
        const char* enText;
    };
    static const CommentMap s_knownComments[] = {
        {"Recurso compartilhado padrão", "Default share"},
        {"Recurso compartilhado padrao", "Default share"},
        {"Compartilhamento padrão", "Default share"},
        {"Compartilhamento padrao", "Default share"},
        {"Administração remota", "Remote Admin"},
        {"Administracao remota", "Remote Admin"},
        {"Compartilhamento de administração remota", "Remote Admin"},
        {"IPC remoto", "Remote IPC"},
        {"Comunicação remota entre processos", "Remote IPC"},
        {"Drivers de impressora", "Printer Drivers"},
        {"Drivers de impressora compartilhados", "Printer Drivers"},
        {"Compartilhamento administrativo", "Administrative share"}
    };

    for (const auto& item : s_knownComments) {
        if (original.compare(QString::fromUtf8(item.ptText), Qt::CaseInsensitive) == 0 ||
            original.compare(QString::fromUtf8(item.enText), Qt::CaseInsensitive) == 0) {
            return isEn ? QString::fromUtf8(item.enText) : QString::fromUtf8(item.ptText);
        }
    }

    return original;
}

} // namespace

ServicesSharesWidget::ServicesSharesWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    retranslateUi();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ServicesSharesWidget::applyTheme);
    applyTheme(ThemeManager::instance().isDark());

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &ServicesSharesWidget::retranslateUi);
}

void ServicesSharesWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void ServicesSharesWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    m_tabWidget = new QTabWidget(this);

    // --- Tab 1: Serviços do Windows ---
    auto* servicesTab = new QWidget(this);
    auto* sLayout = new QVBoxLayout(servicesTab);
    sLayout->setContentsMargins(8, 8, 8, 8);

    auto* sTopLayout = new QHBoxLayout();
    m_btnScanServices = new QPushButton(servicesTab);
    m_btnScanServices->setStyleSheet(QStringLiteral("font-weight: bold; padding: 6px 14px;"));
    connect(m_btnScanServices, &QPushButton::clicked, this, &ServicesSharesWidget::scanServices);
    sTopLayout->addWidget(m_btnScanServices);

    m_lblServicesSummary = new QLabel(servicesTab);
    sTopLayout->addWidget(m_lblServicesSummary);
    sTopLayout->addStretch();

    m_servicesSearchEdit = new QLineEdit(servicesTab);
    m_servicesSearchEdit->setClearButtonEnabled(true);
    connect(m_servicesSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        const QString pattern = text.trimmed();
        const int rowCount = m_servicesTable->rowCount();
        const int colCount = m_servicesTable->columnCount();
        for (int r = 0; r < rowCount; ++r) {
            if (pattern.isEmpty()) {
                m_servicesTable->setRowHidden(r, false);
                continue;
            }
            bool match = false;
            for (int c = 0; c < colCount; ++c) {
                auto* item = m_servicesTable->item(r, c);
                if (item && item->text().contains(pattern, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            m_servicesTable->setRowHidden(r, !match);
        }
    });
    sTopLayout->addWidget(m_servicesSearchEdit);

    sLayout->addLayout(sTopLayout);

    m_servicesTable = new QTableWidget(servicesTab);
    m_servicesTable->setColumnCount(6);
    m_servicesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_servicesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_servicesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_servicesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_servicesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_servicesTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_servicesTable->setAlternatingRowColors(true);
    m_servicesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    TableEnhancer::enhanceTableWidget(
        m_servicesTable,
        QStringLiteral("auditoria_servicos_windows"),
        { 140, 220, 110, 160, 260, 350 }
    );
    sLayout->addWidget(m_servicesTable);

    m_tabWidget->addTab(servicesTab, QString());

    // --- Tab 2: Compartilhamentos SMB ---
    auto* sharesTab = new QWidget(this);
    auto* shLayout = new QVBoxLayout(sharesTab);
    shLayout->setContentsMargins(8, 8, 8, 8);

    auto* shTopLayout = new QHBoxLayout();
    m_btnScanShares = new QPushButton(sharesTab);
    m_btnScanShares->setStyleSheet(QStringLiteral("font-weight: bold; padding: 6px 14px;"));
    connect(m_btnScanShares, &QPushButton::clicked, this, &ServicesSharesWidget::scanShares);
    shTopLayout->addWidget(m_btnScanShares);

    m_lblSharesSummary = new QLabel(sharesTab);
    shTopLayout->addWidget(m_lblSharesSummary);
    shTopLayout->addStretch();

    m_sharesSearchEdit = new QLineEdit(sharesTab);
    m_sharesSearchEdit->setClearButtonEnabled(true);
    connect(m_sharesSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        const QString pattern = text.trimmed();
        const int rowCount = m_sharesTable->rowCount();
        const int colCount = m_sharesTable->columnCount();
        for (int r = 0; r < rowCount; ++r) {
            if (pattern.isEmpty()) {
                m_sharesTable->setRowHidden(r, false);
                continue;
            }
            bool match = false;
            for (int c = 0; c < colCount; ++c) {
                auto* item = m_sharesTable->item(r, c);
                if (item && item->text().contains(pattern, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            m_sharesTable->setRowHidden(r, !match);
        }
    });
    shTopLayout->addWidget(m_sharesSearchEdit);

    shLayout->addLayout(shTopLayout);

    m_sharesTable = new QTableWidget(sharesTab);
    m_sharesTable->setColumnCount(6);
    m_sharesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_sharesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_sharesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_sharesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_sharesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_sharesTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_sharesTable->setAlternatingRowColors(true);
    m_sharesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    TableEnhancer::enhanceTableWidget(
        m_sharesTable,
        QStringLiteral("auditoria_compartilhamentos_smb"),
        { 150, 250, 130, 160, 160, 220 }
    );
    shLayout->addWidget(m_sharesTable);

    m_tabWidget->addTab(sharesTab, QString());

    mainLayout->addWidget(m_tabWidget);
}

void ServicesSharesWidget::retranslateUi() {
    if (m_btnScanServices) {
        m_btnScanServices->setText(Language::GerenciadorTraducao::trText("Auditar Serviços do Windows"));
    }
    if (m_servicesSearchEdit) {
        m_servicesSearchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Buscar nos serviços..."));
    }
    if (m_servicesTable) {
        m_servicesTable->setHorizontalHeaderLabels({
            Language::GerenciadorTraducao::trText("Serviço"),
            Language::GerenciadorTraducao::trText("Nome de Exibição"),
            Language::GerenciadorTraducao::trText("Inicialização"),
            Language::GerenciadorTraducao::trText("Conta"),
            Language::GerenciadorTraducao::trText("Vulnerabilidade Detectada"),
            Language::GerenciadorTraducao::trText("Caminho do Executável")
        });
    }
    if (!m_lastServices.empty()) {
        renderServicesTable();
    } else if (m_lblServicesSummary) {
        m_lblServicesSummary->setText(Language::GerenciadorTraducao::trText("Clique no botão acima para auditar binários e caminhos de serviços do Windows."));
    }

    if (m_btnScanShares) {
        m_btnScanShares->setText(Language::GerenciadorTraducao::trText("Auditar Compartilhamentos de Rede (SMB)"));
    }
    if (m_sharesSearchEdit) {
        m_sharesSearchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Buscar nos compartilhamentos..."));
    }
    if (m_sharesTable) {
        m_sharesTable->setHorizontalHeaderLabels({
            Language::GerenciadorTraducao::trText("Compartilhamento"),
            Language::GerenciadorTraducao::trText("Caminho Local"),
            Language::GerenciadorTraducao::trText("Everyone no Share?"),
            Language::GerenciadorTraducao::trText("Controle Total Everyone?"),
            Language::GerenciadorTraducao::trText("Conflito Share vs NTFS"),
            Language::GerenciadorTraducao::trText("Comentário")
        });
    }
    if (!m_lastShares.empty()) {
        renderSharesTable();
    } else if (m_lblSharesSummary) {
        m_lblSharesSummary->setText(Language::GerenciadorTraducao::trText("Clique no botão acima para listar e correlacionar compartilhamentos e permissões NTFS."));
    }

    if (m_tabWidget) {
        m_tabWidget->setTabText(0, Language::GerenciadorTraducao::trText("Serviços do Windows"));
        m_tabWidget->setTabText(1, Language::GerenciadorTraducao::trText("Compartilhamentos SMB"));
    }
}

void ServicesSharesWidget::renderServicesTable() {
    m_servicesTable->setSortingEnabled(false);
    m_servicesTable->setRowCount(0);
    int vulnerableCount = 0;
    m_servicesTable->setRowCount(static_cast<int>(m_lastServices.size()));

    for (int i = 0; i < static_cast<int>(m_lastServices.size()); ++i) {
        const auto& s = m_lastServices[i];

        m_servicesTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdWString(s.serviceName)));
        m_servicesTable->setItem(i, 1, new QTableWidgetItem(translateServiceDisplayName(s.serviceName, s.displayName)));
        m_servicesTable->setItem(i, 2, new QTableWidgetItem(Language::GerenciadorTraducao::trText(QString::fromStdWString(s.startType))));
        m_servicesTable->setItem(i, 3, new QTableWidgetItem(Language::GerenciadorTraducao::trText(QString::fromStdWString(s.serviceAccount))));

        QString vulnText = Language::GerenciadorTraducao::trText("Conforme");
        QColor vulnColor(40, 160, 60);

        if (s.isBinaryWritableByNonAdmin) {
            vulnText = Language::GerenciadorTraducao::trText("CRÍTICO: Binário Gravável por Usuário Comum");
            vulnColor = QColor(220, 40, 40);
            vulnerableCount++;
        } else if (s.isUnquotedPathWithSpaces) {
            vulnText = Language::GerenciadorTraducao::trText("ALTO: Unquoted Service Path com Espaços");
            vulnColor = QColor(230, 110, 20);
            vulnerableCount++;
        }

        auto* vulnItem = new QTableWidgetItem(vulnText);
        vulnItem->setForeground(vulnColor);
        m_servicesTable->setItem(i, 4, vulnItem);

        m_servicesTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdWString(s.binaryPath)));
    }

    m_servicesTable->setSortingEnabled(true);

    if (m_servicesSearchEdit && !m_servicesSearchEdit->text().trimmed().isEmpty()) {
        const QString pattern = m_servicesSearchEdit->text().trimmed();
        const int rowCount = m_servicesTable->rowCount();
        const int colCount = m_servicesTable->columnCount();
        for (int r = 0; r < rowCount; ++r) {
            bool match = false;
            for (int c = 0; c < colCount; ++c) {
                auto* item = m_servicesTable->item(r, c);
                if (item && item->text().contains(pattern, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            m_servicesTable->setRowHidden(r, !match);
        }
    }

    m_lblServicesSummary->setText(QStringLiteral(
        "%1: %2 %3. %4 %5."
    ).arg(Language::GerenciadorTraducao::trText("Auditoria concluída"))
     .arg(m_lastServices.size())
     .arg(Language::GerenciadorTraducao::trText("serviços inspecionados"))
     .arg(vulnerableCount)
     .arg(Language::GerenciadorTraducao::trText("vulnerabilidade(s) de privilégio encontrada(s)")));
}

void ServicesSharesWidget::renderSharesTable() {
    m_sharesTable->setSortingEnabled(false);
    m_sharesTable->setRowCount(0);
    m_sharesTable->setRowCount(static_cast<int>(m_lastShares.size()));
    int riskyShares = 0;

    for (int i = 0; i < static_cast<int>(m_lastShares.size()); ++i) {
        const auto& sh = m_lastShares[i];

        m_sharesTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdWString(sh.shareName)));
        m_sharesTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdWString(sh.localPath)));

        auto* itemEv = new QTableWidgetItem(sh.isEveryoneAllowed ? Language::GerenciadorTraducao::trText("SIM") : Language::GerenciadorTraducao::trText("Não"));
        if (sh.isEveryoneAllowed) itemEv->setForeground(QColor(200, 140, 20));
        m_sharesTable->setItem(i, 2, itemEv);

        auto* itemFc = new QTableWidgetItem(sh.isEveryoneFullControl ? Language::GerenciadorTraducao::trText("SIM (PERIGOSO)") : Language::GerenciadorTraducao::trText("Não"));
        if (sh.isEveryoneFullControl) {
            itemFc->setForeground(QColor(220, 40, 40));
            riskyShares++;
        }
        m_sharesTable->setItem(i, 3, itemFc);

        auto* itemMis = new QTableWidgetItem(sh.isNtfsMismatch ? Language::GerenciadorTraducao::trText("ALERTA: Gravação Aberta") : Language::GerenciadorTraducao::trText("Alinhado"));
        if (sh.isNtfsMismatch) {
            itemMis->setForeground(QColor(220, 40, 40));
            riskyShares++;
        }
        m_sharesTable->setItem(i, 4, itemMis);

        m_sharesTable->setItem(i, 5, new QTableWidgetItem(translateShareComment(sh.comment)));
    }

    m_sharesTable->setSortingEnabled(true);

    if (m_sharesSearchEdit && !m_sharesSearchEdit->text().trimmed().isEmpty()) {
        const QString pattern = m_sharesSearchEdit->text().trimmed();
        const int rowCount = m_sharesTable->rowCount();
        const int colCount = m_sharesTable->columnCount();
        for (int r = 0; r < rowCount; ++r) {
            bool match = false;
            for (int c = 0; c < colCount; ++c) {
                auto* item = m_sharesTable->item(r, c);
                if (item && item->text().contains(pattern, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            m_sharesTable->setRowHidden(r, !match);
        }
    }

    m_lblSharesSummary->setText(QStringLiteral(
        "%1: %2 %3. %4 %5."
    ).arg(Language::GerenciadorTraducao::trText("Auditoria concluída"))
     .arg(m_lastShares.size())
     .arg(Language::GerenciadorTraducao::trText("compartilhamento(s) inspecionado(s)"))
     .arg(riskyShares)
     .arg(Language::GerenciadorTraducao::trText("ponto(s) de atenção encontrado(s)")));
}

void ServicesSharesWidget::scanServices() {
    m_lastServices = Engine::ServiceAuditor::auditServices();
    renderServicesTable();
}

void ServicesSharesWidget::scanShares() {
    m_lastShares = Engine::SmbShareAuditor::auditShares();
    renderSharesTable();
}

void ServicesSharesWidget::applyTheme(bool isDark) {
    auto styleBtn = [isDark](QPushButton* btn) {
        if (!btn) return;
        btn->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #0078d7; color: #ffffff; border: 1px solid #005a9e; border-radius: 4px; font-weight: bold; padding: 6px 14px; } QPushButton:hover { background-color: #1a88e1; }") :
            QStringLiteral("QPushButton { background-color: #0066cc; color: #ffffff; border: 1px solid #0052a3; border-radius: 4px; font-weight: bold; padding: 6px 14px; } QPushButton:hover { background-color: #0073e6; }"));
    };

    styleBtn(m_btnScanServices);
    styleBtn(m_btnScanShares);

    const QString searchStyle = isDark ?
        QStringLiteral("QLineEdit { background-color: #252526; color: #cccccc; border: 1px solid #3c3c3c; border-radius: 4px; padding: 4px 8px; min-width: 220px; } QLineEdit:focus { border-color: #007acc; }") :
        QStringLiteral("QLineEdit { background-color: #ffffff; color: #333333; border: 1px solid #cccccc; border-radius: 4px; padding: 4px 8px; min-width: 220px; } QLineEdit:focus { border-color: #0078d7; }");
    if (m_servicesSearchEdit) m_servicesSearchEdit->setStyleSheet(searchStyle);
    if (m_sharesSearchEdit) m_sharesSearchEdit->setStyleSheet(searchStyle);

    const QString summaryColor = isDark ? QStringLiteral("color: #cbd5e1;") : QStringLiteral("color: #334155;");
    if (m_lblServicesSummary) m_lblServicesSummary->setStyleSheet(summaryColor);
    if (m_lblSharesSummary) m_lblSharesSummary->setStyleSheet(summaryColor);
}

} // namespace AclAuditor::UI
