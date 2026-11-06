#include "VisualExplanationWidget.hpp"
#include "ThemeManager.hpp"
#include "../engine/EffectiveAccessCalculator.hpp"
#include "../language/GerenciadorTraducao.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFont>
#include <QIcon>
#include <QEvent>

namespace AclAuditor::UI {

VisualExplanationWidget::VisualExplanationWidget(QWidget* parent) : QWidget(parent) {
    setupUi();

    // Populate user combo with discovered local accounts
    auto users = Engine::EffectiveAccessCalculator::enumerateSystemUsers();
    for (const auto& u : users) {
        m_userCombo->addItem(QString::fromStdWString(u));
    }

    retranslateUi();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &VisualExplanationWidget::applyTheme);
    applyTheme(ThemeManager::instance().isDark());

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &VisualExplanationWidget::retranslateUi);
}

void VisualExplanationWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void VisualExplanationWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Header / Selection bar
    m_inputGroup = new QGroupBox(this);
    auto* inputLayout = new QGridLayout(m_inputGroup);
    inputLayout->setContentsMargins(10, 10, 10, 10);
    inputLayout->setSpacing(8);

    m_lblPathTitle = new QLabel(this);
    inputLayout->addWidget(m_lblPathTitle, 0, 0);
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setReadOnly(true);
    inputLayout->addWidget(m_pathEdit, 0, 1);

    m_lblUserTitle = new QLabel(this);
    inputLayout->addWidget(m_lblUserTitle, 1, 0);
    m_userCombo = new QComboBox(this);
    m_userCombo->setEditable(true);
    m_userCombo->setInsertPolicy(QComboBox::InsertAtTop);
    inputLayout->addWidget(m_userCombo, 1, 1);

    m_btnCalculate = new QPushButton(this);
    m_btnCalculate->setStyleSheet(QStringLiteral("font-weight: bold; padding: 6px 14px;"));
    inputLayout->addWidget(m_btnCalculate, 1, 2);

    connect(m_btnCalculate, &QPushButton::clicked, this, &VisualExplanationWidget::onCalculateClicked);
    connect(m_userCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VisualExplanationWidget::onCalculateClicked);

    mainLayout->addWidget(m_inputGroup);

    // Verdict Card
    m_verdictCard = new QFrame(this);
    m_verdictCard->setFrameShape(QFrame::StyledPanel);
    m_verdictCard->setStyleSheet(QStringLiteral("QFrame { background-color: palette(alternate-base); border-radius: 6px; padding: 8px; }"));
    auto* cardLayout = new QVBoxLayout(m_verdictCard);
    cardLayout->setContentsMargins(10, 8, 10, 8);
    cardLayout->setSpacing(6);

    m_verdictLabel = new QLabel(this);
    QFont f = m_verdictLabel->font();
    f.setPointSize(12);
    f.setBold(true);
    m_verdictLabel->setFont(f);
    cardLayout->addWidget(m_verdictLabel);

    m_rightsSummaryLabel = new QLabel(m_verdictCard);
    m_rightsSummaryLabel->setWordWrap(true);
    cardLayout->addWidget(m_rightsSummaryLabel);

    // Quick Rights Row
    auto* rightsRowLayout = new QHBoxLayout();
    rightsRowLayout->setSpacing(15);

    m_lblRead = new QLabel(m_verdictCard);
    m_lblWrite = new QLabel(m_verdictCard);
    m_lblExecute = new QLabel(m_verdictCard);
    m_lblDelete = new QLabel(m_verdictCard);
    m_lblChangePerms = new QLabel(m_verdictCard);

    rightsRowLayout->addWidget(m_lblRead);
    rightsRowLayout->addWidget(m_lblWrite);
    rightsRowLayout->addWidget(m_lblExecute);
    rightsRowLayout->addWidget(m_lblDelete);
    rightsRowLayout->addWidget(m_lblChangePerms);
    rightsRowLayout->addStretch();

    cardLayout->addLayout(rightsRowLayout);
    mainLayout->addWidget(m_verdictCard);

    // Visual Tree Section
    m_treeGroup = new QGroupBox(this);
    auto* treeLayout = new QVBoxLayout(m_treeGroup);
    treeLayout->setContentsMargins(8, 8, 8, 8);

    m_chainTree = new QTreeWidget(m_treeGroup);
    m_chainTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_chainTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_chainTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_chainTree->setAlternatingRowColors(true);
    m_chainTree->setAnimated(true);
    treeLayout->addWidget(m_chainTree);

    mainLayout->addWidget(m_treeGroup, 2);

    // Natural Language Explanation Section
    m_textGroup = new QGroupBox(this);
    auto* textLayout = new QVBoxLayout(m_textGroup);
    textLayout->setContentsMargins(8, 8, 8, 8);

    m_explanationText = new QTextBrowser(m_textGroup);
    m_explanationText->setReadOnly(true);
    m_explanationText->setMaximumHeight(140);
    textLayout->addWidget(m_explanationText);

    mainLayout->addWidget(m_textGroup, 1);
}

void VisualExplanationWidget::retranslateUi() {
    if (m_inputGroup) m_inputGroup->setTitle(Language::GerenciadorTraducao::trText("Parâmetros da Consulta de Acesso Efetivo"));
    if (m_lblPathTitle) m_lblPathTitle->setText(Language::GerenciadorTraducao::trText("Objeto em Análise:"));
    if (m_pathEdit && m_currentPath.isEmpty()) {
        m_pathEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Selecione um arquivo ou diretório para auditar o acesso efetivo."));
    }
    if (m_lblUserTitle) m_lblUserTitle->setText(Language::GerenciadorTraducao::trText("Conta / Usuário para Simulação:"));
    if (m_btnCalculate) m_btnCalculate->setText(Language::GerenciadorTraducao::trText("Calcular Acesso Efetivo"));

    if (m_treeGroup) m_treeGroup->setTitle(Language::GerenciadorTraducao::trText("Árvore Causal e Justificativa de Permissões"));
    if (m_chainTree) {
        m_chainTree->setHeaderLabels({
            Language::GerenciadorTraducao::trText("Regra / Entidade"),
            Language::GerenciadorTraducao::trText("Origem da Herança"),
            Language::GerenciadorTraducao::trText("Permissão")
        });
    }

    if (m_textGroup) m_textGroup->setTitle(Language::GerenciadorTraducao::trText("Cadeia Hierárquica de Resolução de Acesso:"));

    if (!m_lastResult.targetPrincipal.sidString.empty()) {
        renderVerdictCards(m_lastResult);
        renderExplanationTree(m_lastResult);
        renderExplanatoryText(m_lastResult);
    } else {
        applyTheme(ThemeManager::instance().isDark());
    }
}

void VisualExplanationWidget::applyTheme(bool isDark) {
    if (m_btnCalculate) {
        m_btnCalculate->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #0078d7; color: #ffffff; border: 1px solid #005a9e; border-radius: 4px; font-weight: bold; padding: 6px 14px; } QPushButton:hover { background-color: #1a88e1; }") :
            QStringLiteral("QPushButton { background-color: #0066cc; color: #ffffff; border: 1px solid #0052a3; border-radius: 4px; font-weight: bold; padding: 6px 14px; } QPushButton:hover { background-color: #0073e6; }"));
    }

    if (m_verdictCard) {
        m_verdictCard->setStyleSheet(isDark ?
            QStringLiteral("QFrame { background-color: #1e293b; border: 1px solid #334155; border-radius: 6px; padding: 8px; }") :
            QStringLiteral("QFrame { background-color: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px; }"));
    }

    if (!m_lastResult.targetPrincipal.sidString.empty()) {
        renderVerdictCards(m_lastResult);
    } else {
        if (m_verdictLabel) {
            m_verdictLabel->setStyleSheet(isDark ?
                QStringLiteral("color: #94a3b8; font-size: 12pt; font-weight: bold;") :
                QStringLiteral("color: #64748b; font-size: 12pt; font-weight: bold;"));
            m_verdictLabel->setText(Language::GerenciadorTraducao::trText("Selecione um arquivo ou diretório para auditar o acesso efetivo."));
        }
        const QString emptyStyle = isDark ? QStringLiteral("color: #94a3b8; font-weight: 500;") : QStringLiteral("color: #64748b; font-weight: 500;");
        if (m_lblRead) { m_lblRead->setStyleSheet(emptyStyle); m_lblRead->setText(QStringLiteral("%1: -").arg(Language::GerenciadorTraducao::trText("LEITURA"))); }
        if (m_lblWrite) { m_lblWrite->setStyleSheet(emptyStyle); m_lblWrite->setText(QStringLiteral("%1: -").arg(Language::GerenciadorTraducao::trText("GRAVAÇÃO"))); }
        if (m_lblExecute) { m_lblExecute->setStyleSheet(emptyStyle); m_lblExecute->setText(QStringLiteral("%1: -").arg(Language::GerenciadorTraducao::trText("EXECUÇÃO"))); }
        if (m_lblDelete) { m_lblDelete->setStyleSheet(emptyStyle); m_lblDelete->setText(QStringLiteral("%1: -").arg(Language::GerenciadorTraducao::trText("EXCLUSÃO"))); }
        if (m_lblChangePerms) { m_lblChangePerms->setStyleSheet(emptyStyle); m_lblChangePerms->setText(QStringLiteral("%1: -").arg(Language::GerenciadorTraducao::trText("ALTERAR ACL"))); }
    }
}

void VisualExplanationWidget::setTargetObject(const QString& objectPath) {
    m_currentPath = objectPath;
    m_pathEdit->setText(objectPath);
    evaluateAccess();
}

void VisualExplanationWidget::evaluateAccess() {
    if (m_currentPath.isEmpty()) return;

    QString username = m_userCombo->currentText().trimmed();
    if (username.isEmpty()) {
        username = QStringLiteral("Everyone");
    }

    m_lastResult = Engine::EffectiveAccessCalculator::calculateEffectiveAccess(m_currentPath.toStdWString(), username.toStdWString());

    renderVerdictCards(m_lastResult);
    renderExplanationTree(m_lastResult);
    renderExplanatoryText(m_lastResult);
}

void VisualExplanationWidget::onCalculateClicked() {
    evaluateAccess();
}

void VisualExplanationWidget::renderVerdictCards(const Engine::EffectiveAccessResult& result) {
    const bool isDark = ThemeManager::instance().isDark();
    if (result.targetPrincipal.sidString.empty()) {
        m_verdictLabel->setStyleSheet(isDark ?
            QStringLiteral("color: #94a3b8; font-size: 12pt; font-weight: bold;") :
            QStringLiteral("color: #64748b; font-size: 12pt; font-weight: bold;"));
        m_verdictLabel->setText(Language::GerenciadorTraducao::trText("Selecione um arquivo ou diretório para auditar o acesso efetivo."));
        return;
    }

    QString verdictText;
    QString colorStyle;

    if (!result.conflictingDenyAces.empty()) {
        verdictText = Language::GerenciadorTraducao::trText("ACESSO NEGADO (REGRA DE NEGAÇÃO / DENY PREVALECE)");
        colorStyle = isDark ? QStringLiteral("color: #f87171; font-size: 13pt; font-weight: bold;")
                            : QStringLiteral("color: #dc2626; font-size: 13pt; font-weight: bold;");
    } else if (result.canFullControl) {
        verdictText = Language::GerenciadorTraducao::trText("CONTROLE TOTAL (ACESSO ADMINISTRATIVO COMPLETO)");
        colorStyle = isDark ? QStringLiteral("color: #4ade80; font-size: 13pt; font-weight: bold;")
                            : QStringLiteral("color: #16a34a; font-size: 13pt; font-weight: bold;");
    } else if (result.canModify) {
        verdictText = Language::GerenciadorTraducao::trText("MODIFICAR (LER, GRAVAR, EXECUTAR E EXCLUIR)");
        colorStyle = isDark ? QStringLiteral("color: #38bdf8; font-size: 13pt; font-weight: bold;")
                            : QStringLiteral("color: #0284c7; font-size: 13pt; font-weight: bold;");
    } else if (result.canReadAndExecute) {
        verdictText = Language::GerenciadorTraducao::trText("LEITURA E EXECUÇÃO (LISTAR, LER E EXECUTAR)");
        colorStyle = isDark ? QStringLiteral("color: #60a5fa; font-size: 13pt; font-weight: bold;")
                            : QStringLiteral("color: #2563eb; font-size: 13pt; font-weight: bold;");
    } else if (result.canWrite) {
        verdictText = Language::GerenciadorTraducao::trText("GRAVAÇÃO / CRIAÇÃO DE ARQUIVOS");
        colorStyle = isDark ? QStringLiteral("color: #fb923c; font-size: 13pt; font-weight: bold;")
                            : QStringLiteral("color: #ea580c; font-size: 13pt; font-weight: bold;");
    } else if (result.canRead) {
        verdictText = Language::GerenciadorTraducao::trText("SOMENTE LEITURA");
        colorStyle = isDark ? QStringLiteral("color: #94a3b8; font-size: 13pt; font-weight: bold;")
                            : QStringLiteral("color: #64748b; font-size: 13pt; font-weight: bold;");
    } else {
        verdictText = Language::GerenciadorTraducao::trText("SEM ACESSO (NENHUMA PERMISSÃO CONCEDIDA)");
        colorStyle = isDark ? QStringLiteral("color: #94a3b8; font-size: 13pt; font-weight: bold;")
                            : QStringLiteral("color: #64748b; font-size: 13pt; font-weight: bold;");
    }

    m_verdictLabel->setStyleSheet(colorStyle);
    m_verdictLabel->setText(verdictText);

    auto makeStatus = [isDark](bool granted, const QString& name) {
        if (granted) {
            const QString green = isDark ? QStringLiteral("#4ade80") : QStringLiteral("#16a34a");
            return QStringLiteral("<span style='color: %1; font-weight: bold;'>[✓] %2: %3</span>")
                .arg(green, name, Language::GerenciadorTraducao::trText("SIM"));
        } else {
            const QString neutral = isDark ? QStringLiteral("#94a3b8") : QStringLiteral("#64748b");
            return QStringLiteral("<span style='color: %1;'>[✗] %2: %3</span>")
                .arg(neutral, name, Language::GerenciadorTraducao::trText("NÃO"));
        }
    };

    m_lblRead->setStyleSheet(QString());
    m_lblWrite->setStyleSheet(QString());
    m_lblExecute->setStyleSheet(QString());
    m_lblDelete->setStyleSheet(QString());
    m_lblChangePerms->setStyleSheet(QString());

    m_lblRead->setText(makeStatus(result.canRead, Language::GerenciadorTraducao::trText("LEITURA")));
    m_lblWrite->setText(makeStatus(result.canWrite, Language::GerenciadorTraducao::trText("GRAVAÇÃO")));
    m_lblExecute->setText(makeStatus(result.canReadAndExecute, Language::GerenciadorTraducao::trText("EXECUÇÃO")));
    m_lblDelete->setText(makeStatus(result.canDelete, Language::GerenciadorTraducao::trText("EXCLUSÃO")));
    m_lblChangePerms->setText(makeStatus(result.canChangePermissions, Language::GerenciadorTraducao::trText("ALTERAR ACL")));
}

static QString translateRightsList(const QString& raw) {
    if (raw.contains(QStringLiteral(", "))) {
        const QStringList parts = raw.split(QStringLiteral(", "));
        QStringList trParts;
        trParts.reserve(parts.size());
        for (const QString& p : parts) {
            trParts.append(Language::GerenciadorTraducao::trText(p.trimmed()));
        }
        return trParts.join(QStringLiteral(", "));
    }
    return Language::GerenciadorTraducao::trText(raw);
}

static QString translateChainText(const std::wstring& ws) {
    if (ws.empty()) return QString();
    QString qs = QString::fromStdWString(ws);

    if (qs.startsWith(QStringLiteral("Identidade Analisada: "))) {
        return Language::GerenciadorTraducao::trText("Identidade Analisada: %1").arg(qs.mid(22));
    }
    if (qs.startsWith(QStringLiteral("Atribuição Direta: "))) {
        return Language::GerenciadorTraducao::trText("Atribuição Direta: %1").arg(qs.mid(19));
    }
    if (qs.startsWith(QStringLiteral("Via Grupo: "))) {
        return Language::GerenciadorTraducao::trText("Via Grupo: %1").arg(qs.mid(11));
    }
    if (qs == QStringLiteral("Origem da Regra: Herdada")) {
        return Language::GerenciadorTraducao::trText("Origem da Regra: Herdada");
    }
    if (qs == QStringLiteral("Origem da Regra: Explícita")) {
        return Language::GerenciadorTraducao::trText("Origem da Regra: Explícita");
    }
    if (qs.startsWith(QStringLiteral("Direito CONCEDIDO: "))) {
        return Language::GerenciadorTraducao::trText("Direito CONCEDIDO: %1").arg(translateRightsList(qs.mid(19).trimmed()));
    }
    if (qs.startsWith(QStringLiteral("Direito NEGADO: "))) {
        return Language::GerenciadorTraducao::trText("Direito NEGADO: %1").arg(translateRightsList(qs.mid(16).trimmed()));
    }
    if (qs == QStringLiteral("Alvo da auditoria de acesso efetivo")) {
        return Language::GerenciadorTraducao::trText("Alvo da auditoria de acesso efetivo");
    }
    if (qs == QStringLiteral("Permissão concedida diretamente ao usuário/objeto")) {
        return Language::GerenciadorTraducao::trText("Permissão concedida diretamente ao usuário/objeto");
    }
    if (qs == QStringLiteral("Grupo universal do Windows NT do qual o usuário é membro implícito")) {
        return Language::GerenciadorTraducao::trText("Grupo universal do Windows NT do qual o usuário é membro implícito");
    }
    if (qs == QStringLiteral("O usuário pertence a este grupo de segurança")) {
        return Language::GerenciadorTraducao::trText("O usuário pertence a este grupo de segurança");
    }
    if (qs == QStringLiteral("Herdada da pasta pai")) {
        return Language::GerenciadorTraducao::trText("Herdada da pasta pai");
    }
    if (qs.startsWith(QStringLiteral("Herdada de: "))) {
        return Language::GerenciadorTraducao::trText("Herdada de: %1").arg(qs.mid(12));
    }
    if (qs == QStringLiteral("Definida explicitamente neste objeto (não herdada)")) {
        return Language::GerenciadorTraducao::trText("Definida explicitamente neste objeto (não herdada)");
    }
    return translateRightsList(qs);
}

static void addChainNodeToTree(QTreeWidgetItem* parentItem, const Engine::EffectiveChainStep& step) {
    auto* item = new QTreeWidgetItem(parentItem);
    item->setText(0, translateChainText(step.title));
    item->setText(1, translateChainText(step.relationship));
    item->setText(2, translateChainText(step.accessGranted));

    if (step.accessType == Engine::AccessType::Deny) {
        item->setForeground(0, QColor(220, 50, 50));
        item->setForeground(2, QColor(220, 50, 50));
    } else if (step.isDecisive) {
        item->setForeground(2, QColor(40, 160, 60));
    }

    for (const auto& child : step.children) {
        addChainNodeToTree(item, child);
    }

    item->setExpanded(true);
}

void VisualExplanationWidget::renderExplanationTree(const Engine::EffectiveAccessResult& result) {
    m_chainTree->clear();

    for (const auto& rootStep : result.explanationTree) {
        auto* item = new QTreeWidgetItem(m_chainTree);
        item->setText(0, translateChainText(rootStep.title));
        item->setText(1, translateChainText(rootStep.relationship));
        item->setText(2, translateChainText(rootStep.accessGranted));

        QFont boldFont = item->font(0);
        boldFont.setBold(true);
        item->setFont(0, boldFont);
        item->setFont(2, boldFont);

        for (const auto& child : rootStep.children) {
            addChainNodeToTree(item, child);
        }

        item->setExpanded(true);
    }
}

void VisualExplanationWidget::renderExplanatoryText(const Engine::EffectiveAccessResult& result) {
    if (result.targetPrincipal.sidString.empty()) {
        m_explanationText->clear();
        return;
    }

    QString html;
    QTextStream ss(&html);

    ss << "<b>" << Language::GerenciadorTraducao::trText("Identidade:") << "</b> "
       << QString::fromStdWString(result.targetPrincipal.getDisplayName()) << "<br>";

    if (!result.conflictingDenyAces.empty()) {
        ss << "<span style='color: #dc2626; font-weight: bold;'>"
           << Language::GerenciadorTraducao::trText("[ATENÇÃO - REGRA DE NEGAÇÃO ATIVA] Foram encontradas %1 regras de 'Acesso Negado' (Deny) aplicáveis que sobrepõem permissões de concessão.").arg(result.conflictingDenyAces.size())
           << "</span><br>";
    }

    QString verdict;
    if (result.canFullControl) {
        verdict = Language::GerenciadorTraducao::trText("Veredito: CONTROLE TOTAL. O usuário possui direitos administrativos totais sobre o objeto.");
    } else if (result.canModify) {
        verdict = Language::GerenciadorTraducao::trText("Veredito: MODIFICAR. O usuário pode ler, gravar, alterar atributos e excluir arquivos neste local.");
    } else if (result.canReadAndExecute) {
        verdict = Language::GerenciadorTraducao::trText("Veredito: LEITURA E EXECUÇÃO. O usuário pode listar pastas, ler arquivos e executar programas.");
    } else if (result.canWrite) {
        verdict = Language::GerenciadorTraducao::trText("Veredito: GRAVAÇÃO. O usuário pode criar novos arquivos ou anexar dados.");
    } else if (result.canRead) {
        verdict = Language::GerenciadorTraducao::trText("Veredito: SOMENTE LEITURA. O usuário pode visualizar o conteúdo, mas não pode alterar ou executar.");
    } else {
        verdict = Language::GerenciadorTraducao::trText("Veredito: SEM ACESSO. Nenhuma permissão de concessão aplicável foi identificada na lista de controle de acesso.");
    }

    ss << "<b>" << verdict << "</b><br><br>";
    ss << "<b>" << Language::GerenciadorTraducao::trText("Cadeia de autorização:") << "</b><br>";

    if (result.matchingAces.empty() && result.conflictingDenyAces.empty()) {
        ss << " • " << Language::GerenciadorTraducao::trText("Nenhum ACE na DACL corresponde a este usuário ou a nenhum dos seus grupos.") << "<br>";
    } else {
        for (const auto& ace : result.matchingAces) {
            ss << " • " << Language::GerenciadorTraducao::trText("Concedido via [%1]: %2 (%3)")
                .arg(QString::fromStdWString(ace.trustee.getDisplayName()))
                .arg(Language::GerenciadorTraducao::trText(QString::fromStdWString(ace.getRightsSummary()).toUtf8().constData()))
                .arg(Language::GerenciadorTraducao::trText(QString::fromStdWString(ace.getScopeDescription()).toUtf8().constData()))
               << "<br>";
        }
        for (const auto& ace : result.conflictingDenyAces) {
            ss << " • " << Language::GerenciadorTraducao::trText("NEGADO via [%1]: %2 (%3)")
                .arg(QString::fromStdWString(ace.trustee.getDisplayName()))
                .arg(Language::GerenciadorTraducao::trText(QString::fromStdWString(ace.getRightsSummary()).toUtf8().constData()))
                .arg(Language::GerenciadorTraducao::trText(QString::fromStdWString(ace.getScopeDescription()).toUtf8().constData()))
               << "<br>";
        }
    }

    m_explanationText->setHtml(html);
}

} // namespace AclAuditor::UI
