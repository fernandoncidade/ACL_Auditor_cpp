#include "RiskDashboardWidget.hpp"
#include "ThemeManager.hpp"
#include "TableEnhancer.hpp"
#include "../language/GerenciadorTraducao.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QFont>
#include <QEvent>

namespace AclAuditor::UI {

RiskDashboardWidget::RiskDashboardWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    retranslateUi();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &RiskDashboardWidget::applyTheme);
    applyTheme(ThemeManager::instance().isDark());

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &RiskDashboardWidget::retranslateUi);
}

void RiskDashboardWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void RiskDashboardWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Cards layout (Top metrics)
    auto* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(10);

    auto createCard = [this](QFrame*& cardOut, QLabel*& titleOut, QLabel*& valueOut) -> QWidget* {
        auto* card = new QFrame(this);
        card->setFrameShape(QFrame::StyledPanel);
        cardOut = card;
        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(8, 6, 8, 6);
        layout->setSpacing(2);

        titleOut = new QLabel(card);
        valueOut = new QLabel(QStringLiteral("0"), card);
        QFont f = valueOut->font();
        f.setPointSize(16);
        f.setBold(true);
        valueOut->setFont(f);

        layout->addWidget(titleOut);
        layout->addWidget(valueOut);
        return card;
    };

    cardsLayout->addWidget(createCard(m_cardCritical, m_lblTitleCritical, m_lblCardCritical));
    cardsLayout->addWidget(createCard(m_cardHigh, m_lblTitleHigh, m_lblCardHigh));
    cardsLayout->addWidget(createCard(m_cardMedium, m_lblTitleMedium, m_lblCardMedium));
    cardsLayout->addWidget(createCard(m_cardWritable, m_lblTitleWritable, m_lblCardWritable));
    cardsLayout->addWidget(createCard(m_cardOrphaned, m_lblTitleOrphaned, m_lblCardOrphaned));

    mainLayout->addLayout(cardsLayout);

    // Filter and search row
    auto* filterLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &RiskDashboardWidget::onFilterTextChanged);
    filterLayout->addWidget(m_searchEdit);

    m_btnPlan = new QPushButton(this);
    m_btnPlan->setStyleSheet(QStringLiteral("font-weight: bold; padding: 5px 12px;"));
    connect(m_btnPlan, &QPushButton::clicked, this, &RiskDashboardWidget::onGeneratePlanClicked);
    filterLayout->addWidget(m_btnPlan);

    mainLayout->addLayout(filterLayout);

    // Table view
    m_model = new Model::RiskTableModel(this);
    m_proxyModel = new MultiColumnSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    m_tableView = new QTableView(this);
    TableEnhancer::enhanceTableView(
        m_tableView,
        m_proxyModel,
        QStringLiteral("auditoria_riscos_conformidade"),
        { 100, 260, 320, 180, 300 }
    );
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RiskDashboardWidget::onSelectionChanged);

    mainLayout->addWidget(m_tableView, 3);

    // Detail Panel
    m_detailGroup = new QGroupBox(this);
    auto* detailLayout = new QVBoxLayout(m_detailGroup);
    detailLayout->setContentsMargins(10, 8, 10, 8);
    detailLayout->setSpacing(6);

    m_lblDetailTitle = new QLabel(m_detailGroup);
    QFont fTitle = m_lblDetailTitle->font();
    fTitle.setBold(true);
    m_lblDetailTitle->setFont(fTitle);
    detailLayout->addWidget(m_lblDetailTitle);

    m_detailText = new QTextBrowser(m_detailGroup);
    m_detailText->setReadOnly(true);
    m_detailText->setMaximumHeight(90);
    detailLayout->addWidget(m_detailText);

    // Quick Command Generators
    auto* icaclsLayout = new QHBoxLayout();
    icaclsLayout->addWidget(new QLabel(QStringLiteral("icacls:"), m_detailGroup));
    m_icaclsEdit = new QLineEdit(m_detailGroup);
    m_icaclsEdit->setReadOnly(true);
    icaclsLayout->addWidget(m_icaclsEdit, 1);
    m_btnCopyIcacls = new QPushButton(QStringLiteral("Copiar"), m_detailGroup);
    connect(m_btnCopyIcacls, &QPushButton::clicked, this, &RiskDashboardWidget::onCopyIcaclsClicked);
    icaclsLayout->addWidget(m_btnCopyIcacls);
    detailLayout->addLayout(icaclsLayout);

    auto* psLayout = new QHBoxLayout();
    psLayout->addWidget(new QLabel(QStringLiteral("PowerShell:"), m_detailGroup));
    m_powershellEdit = new QLineEdit(m_detailGroup);
    m_powershellEdit->setReadOnly(true);
    psLayout->addWidget(m_powershellEdit, 1);
    m_btnCopyPs = new QPushButton(QStringLiteral("Copiar"), m_detailGroup);
    connect(m_btnCopyPs, &QPushButton::clicked, this, &RiskDashboardWidget::onCopyPowerShellClicked);
    psLayout->addWidget(m_btnCopyPs);
    detailLayout->addLayout(psLayout);

    mainLayout->addWidget(m_detailGroup, 2);
}

void RiskDashboardWidget::retranslateUi() {
    if (m_lblTitleCritical) m_lblTitleCritical->setText(Language::GerenciadorTraducao::trText("Crítico"));
    if (m_lblTitleHigh) m_lblTitleHigh->setText(Language::GerenciadorTraducao::trText("Alto"));
    if (m_lblTitleMedium) m_lblTitleMedium->setText(Language::GerenciadorTraducao::trText("Médio"));
    if (m_lblTitleWritable) m_lblTitleWritable->setText(Language::GerenciadorTraducao::trText("Pastas Graváveis (World-Writable)"));
    if (m_lblTitleOrphaned) m_lblTitleOrphaned->setText(Language::GerenciadorTraducao::trText("SIDs Órfãos / Desconhecidos"));

    if (m_searchEdit) m_searchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Filtrar por nome de arquivo/pasta..."));
    if (m_btnPlan) m_btnPlan->setText(Language::GerenciadorTraducao::trText("Gerar Plano de Remediação"));

    if (m_detailGroup) m_detailGroup->setTitle(Language::GerenciadorTraducao::trText("Descrição do Risco"));
    if (m_btnCopyIcacls) m_btnCopyIcacls->setText(Language::GerenciadorTraducao::trText("Copiar"));
    if (m_btnCopyPs) m_btnCopyPs->setText(Language::GerenciadorTraducao::trText("Copiar"));

    if (m_model) m_model->refreshTranslations();

    if (m_filteredFindings.empty()) {
        if (m_lblDetailTitle) {
            m_lblDetailTitle->setText(Language::GerenciadorTraducao::trText("Nenhum risco ou vulnerabilidade detectado no escopo auditado."));
        }
    } else {
        onSelectionChanged();
    }
}

void RiskDashboardWidget::setFindings(const std::vector<Engine::RiskFinding>& findings) {
    m_allFindings = findings;
    m_filteredFindings = findings;
    m_model->setFindings(m_filteredFindings);
    updateMetricCards();
}

void RiskDashboardWidget::clear() {
    m_allFindings.clear();
    m_filteredFindings.clear();
    m_model->clear();
    updateMetricCards();
}

void RiskDashboardWidget::updateMetricCards() {
    size_t crit = 0, high = 0, med = 0, writable = 0, orphaned = 0;

    for (const auto& f : m_allFindings) {
        if (f.severity == Engine::RiskSeverity::Critical) crit++;
        else if (f.severity == Engine::RiskSeverity::High) high++;
        else if (f.severity == Engine::RiskSeverity::Medium) med++;

        if (f.category == Engine::RiskCategory::WorldWritable) {
            writable++;
        }
        if (f.category == Engine::RiskCategory::OrphanedPrincipal) {
            orphaned++;
        }
    }

    m_lblCardCritical->setText(QString::number(crit));
    m_lblCardHigh->setText(QString::number(high));
    m_lblCardMedium->setText(QString::number(med));
    m_lblCardWritable->setText(QString::number(writable));
    m_lblCardOrphaned->setText(QString::number(orphaned));
}

void RiskDashboardWidget::onFilterTextChanged(const QString& text) {
    if (text.trimmed().isEmpty()) {
        m_filteredFindings = m_allFindings;
    } else {
        m_filteredFindings.clear();
        QString lower = text.toLower();
        for (const auto& f : m_allFindings) {
            QString path = QString::fromStdWString(f.affectedPath).toLower();
            QString title = QString::fromStdWString(f.title).toLower();
            QString desc = QString::fromStdWString(f.description).toLower();
            QString princ = QString::fromStdWString(f.affectedPrincipal).toLower();

            if (path.contains(lower) || title.contains(lower) || desc.contains(lower) || princ.contains(lower)) {
                m_filteredFindings.push_back(f);
            }
        }
    }
    m_model->setFindings(m_filteredFindings);
}

void RiskDashboardWidget::onSelectionChanged() {
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        m_lblDetailTitle->setText(Language::GerenciadorTraducao::trText("Selecione um arquivo ou diretório primeiro."));
        m_detailText->clear();
        m_icaclsEdit->clear();
        m_powershellEdit->clear();
        return;
    }

    QModelIndex srcIndex = m_proxyModel ? m_proxyModel->mapToSource(selected.first()) : selected.first();
    int row = srcIndex.row();
    const auto* finding = m_model->getFindingAt(row);
    if (!finding) return;

    m_lblDetailTitle->setText(Language::GerenciadorTraducao::trText(QString::fromStdWString(finding->title)));
    m_detailText->setHtml(QStringLiteral(
        "<b>%1:</b> %2<br>"
        "<b>%3:</b> %4<br>"
        "<b>%5:</b> %6<br>"
        "<b>%7:</b> %8<br>"
        "<b>%9:</b> %10"
    ).arg(
        Language::GerenciadorTraducao::trText("Caminho Afetado"),
        QString::fromStdWString(finding->affectedPath),
        Language::GerenciadorTraducao::trText("Identidade Afetada"),
        Language::GerenciadorTraducao::trText(QString::fromStdWString(finding->affectedPrincipal)),
        Language::GerenciadorTraducao::trText("Vulnerabilidade Identificada"),
        Model::RiskTableModel::formatFindingDescriptionLocalized(*finding),
        Language::GerenciadorTraducao::trText("Detalhe Técnico"),
        Model::RiskTableModel::formatFindingTechnicalDetailLocalized(*finding),
        Language::GerenciadorTraducao::trText("Ação Recomendada"),
        Language::GerenciadorTraducao::trText(QString::fromStdWString(finding->remediationAdvice))
    ));

    m_icaclsEdit->setText(QString::fromStdWString(finding->icaclsCommand));
    m_powershellEdit->setText(QString::fromStdWString(finding->powershellCommand));
}

void RiskDashboardWidget::onCopyIcaclsClicked() {
    QApplication::clipboard()->setText(m_icaclsEdit->text());
}

void RiskDashboardWidget::onCopyPowerShellClicked() {
    QApplication::clipboard()->setText(m_powershellEdit->text());
}

void RiskDashboardWidget::onGeneratePlanClicked() {
    emit openRemediationPlanRequested(m_filteredFindings);
}

void RiskDashboardWidget::applyTheme(bool isDark) {
    auto styleCard = [isDark](QFrame* card, QLabel* title, QLabel* value, const QString& darkColor, const QString& lightColor) {
        if (!card || !title || !value) return;
        const QString activeColor = isDark ? darkColor : lightColor;
        card->setStyleSheet(isDark ?
            QStringLiteral("QFrame { background-color: #1e293b; border: 1px solid #334155; border-left: 5px solid %1; border-radius: 4px; padding: 6px; }").arg(activeColor) :
            QStringLiteral("QFrame { background-color: #f1f5f9; border: 1px solid #cbd5e1; border-left: 5px solid %1; border-radius: 4px; padding: 6px; }").arg(activeColor));
        title->setStyleSheet(isDark ?
            QStringLiteral("font-size: 8pt; color: #94a3b8; text-transform: uppercase; font-weight: bold;") :
            QStringLiteral("font-size: 8pt; color: #64748b; text-transform: uppercase; font-weight: bold;"));
        value->setStyleSheet(QStringLiteral("color: %1; font-size: 16pt; font-weight: bold;").arg(activeColor));
    };

    styleCard(m_cardCritical, m_lblTitleCritical, m_lblCardCritical, QStringLiteral("#f87171"), QStringLiteral("#dc2626"));
    styleCard(m_cardHigh, m_lblTitleHigh, m_lblCardHigh, QStringLiteral("#fb923c"), QStringLiteral("#ea580c"));
    styleCard(m_cardMedium, m_lblTitleMedium, m_lblCardMedium, QStringLiteral("#facc15"), QStringLiteral("#d97706"));
    styleCard(m_cardWritable, m_lblTitleWritable, m_lblCardWritable, QStringLiteral("#f472b6"), QStringLiteral("#db2777"));
    styleCard(m_cardOrphaned, m_lblTitleOrphaned, m_lblCardOrphaned, QStringLiteral("#c084fc"), QStringLiteral("#7c3aed"));

    if (m_btnPlan) {
        m_btnPlan->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #0078d7; color: #ffffff; border: 1px solid #005a9e; border-radius: 4px; font-weight: bold; padding: 5px 14px; } QPushButton:hover { background-color: #1a88e1; }") :
            QStringLiteral("QPushButton { background-color: #0066cc; color: #ffffff; border: 1px solid #0052a3; border-radius: 4px; font-weight: bold; padding: 5px 14px; } QPushButton:hover { background-color: #0073e6; }"));
    }

    if (m_btnCopyIcacls) {
        m_btnCopyIcacls->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #2d3034; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px 10px; font-weight: bold; } QPushButton:hover { background-color: #383c42; }") :
            QStringLiteral("QPushButton { background-color: #e9ecef; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px 10px; font-weight: bold; } QPushButton:hover { background-color: #dee2e6; }"));
    }
    if (m_btnCopyPs) {
        m_btnCopyPs->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #2d3034; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px 10px; font-weight: bold; } QPushButton:hover { background-color: #383c42; }") :
            QStringLiteral("QPushButton { background-color: #e9ecef; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px 10px; font-weight: bold; } QPushButton:hover { background-color: #dee2e6; }"));
    }

    if (m_lblDetailTitle) {
        m_lblDetailTitle->setStyleSheet(isDark ? QStringLiteral("color: #f0f2f5; font-weight: bold;")
                                              : QStringLiteral("color: #0f172a; font-weight: bold;"));
    }
}

} // namespace AclAuditor::UI
