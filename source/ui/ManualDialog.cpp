#include "ManualDialog.hpp"
#include "Win32DwmHelper.hpp"
#include "ThemeManager.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include "../utils/ApplicationPathUtils.hpp"

#include <QAbstractTextDocumentLayout>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextFragment>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace AclAuditor::UI {

ManualDialog::ManualDialog(QWidget* parent) : QDialog(parent) {
    const QIcon appIcon = Utils::ApplicationPathUtils::getAppIcon();
    if (!appIcon.isNull()) {
        setWindowIcon(appIcon);
    }
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMinimumSize(900, 650);
    resize(980, 720);
    Win32DwmHelper::applyMicaAndDarkMode(this);

    m_activeLanguage = Language::GerenciadorTraducao::instance().obter_idioma_atual();
    setupUi();
    retranslateUi();

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &ManualDialog::onIdiomaAlterado);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ManualDialog::applyTheme);

    applyTheme(ThemeManager::instance().isDark());
}

void ManualDialog::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        m_activeLanguage = Language::GerenciadorTraducao::instance().obter_idioma_atual();
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void ManualDialog::onIdiomaAlterado(const QString& idioma) {
    m_activeLanguage = idioma;
    retranslateUi();
}

namespace {

static QString buildManualCss(bool isDark) {
    if (isDark) {
        return QStringLiteral(R"(
<style>
  body { font-family: 'Segoe UI', Tahoma, Helvetica, sans-serif; font-size: 13px; line-height: 1.6; color: #e2e8f0; background-color: #0b1120; margin: 0; padding: 10px; }
  h1 { color: #38bdf8; font-size: 20px; border-bottom: 2px solid #1e293b; padding-bottom: 6px; margin-top: 14px; }
  h2 { color: #60a5fa; font-size: 16px; border-bottom: 1px solid #1e293b; padding-bottom: 4px; margin-top: 22px; }
  h3 { color: #93c5fd; font-size: 14px; margin-top: 14px; }
  p, li { color: #cbd5e1; }
  b, strong { color: #f8fafc; }
  code { font-family: 'Cascadia Code', Consolas, 'Courier New', monospace; background-color: #1e293b; color: #38bdf8; padding: 2px 5px; border-radius: 4px; font-size: 12px; }
  pre { font-family: 'Cascadia Code', Consolas, 'Courier New', monospace; background-color: #020617; color: #e2e8f0; padding: 10px; border-radius: 6px; border: 1px solid #1e293b; overflow-x: auto; font-size: 12px; }
  table { width: 100%; border-collapse: collapse; margin: 12px 0; background-color: #0f172a; border-radius: 6px; overflow: hidden; border: 1px solid #1e293b; }
  th { background-color: #1e293b; color: #f1f5f9; padding: 8px 10px; text-align: left; font-size: 12px; border: 1px solid #334155; }
  td { padding: 7px 10px; border: 1px solid #1e293b; font-size: 12px; color: #cbd5e1; }
  tr:nth-child(even) { background-color: #090d16; }
  hr { border: none; border-top: 1px solid #1e293b; margin: 18px 0; }
  a { color: #38bdf8; text-decoration: none; }
  a:hover { text-decoration: underline; }
  .badge-crit { background-color: #dc2626; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-high { background-color: #ea580c; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-med { background-color: #ca8a04; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-low { background-color: #2563eb; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-ok { background-color: #16a34a; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .card-tip { background-color: #022c22; border-left: 4px solid #10b981; color: #a7f3d0; padding: 10px 14px; margin: 10px 0; border-radius: 0 6px 6px 0; }
  .card-warn { background-color: #451a03; border-left: 4px solid #f97316; color: #fed7aa; padding: 10px 14px; margin: 10px 0; border-radius: 0 6px 6px 0; }
  .card-info { background-color: #082f49; border-left: 4px solid #0284c7; color: #bae6fd; padding: 10px 14px; margin: 10px 0; border-radius: 0 6px 6px 0; }
  .lead-quote { font-size: 14px; color: #38bdf8; font-style: italic; font-weight: bold; margin: 8px 0; }
</style>
)");
    } else {
        return QStringLiteral(R"(
<style>
  body { font-family: 'Segoe UI', Tahoma, Helvetica, sans-serif; font-size: 13px; line-height: 1.6; color: #1e293b; background-color: #ffffff; margin: 0; padding: 10px; }
  h1 { color: #0284c7; font-size: 20px; border-bottom: 2px solid #e2e8f0; padding-bottom: 6px; margin-top: 14px; }
  h2 { color: #1d4ed8; font-size: 16px; border-bottom: 1px solid #e2e8f0; padding-bottom: 4px; margin-top: 22px; }
  h3 { color: #0369a1; font-size: 14px; margin-top: 14px; }
  p, li { color: #334155; }
  b, strong { color: #0f172a; }
  code { font-family: 'Cascadia Code', Consolas, 'Courier New', monospace; background-color: #f1f5f9; color: #0284c7; padding: 2px 5px; border-radius: 4px; font-size: 12px; }
  pre { font-family: 'Cascadia Code', Consolas, 'Courier New', monospace; background-color: #f8fafc; color: #0f172a; padding: 10px; border-radius: 6px; border: 1px solid #e2e8f0; overflow-x: auto; font-size: 12px; }
  table { width: 100%; border-collapse: collapse; margin: 12px 0; background-color: #ffffff; border-radius: 6px; overflow: hidden; border: 1px solid #e2e8f0; }
  th { background-color: #f1f5f9; color: #0f172a; padding: 8px 10px; text-align: left; font-size: 12px; border: 1px solid #cbd5e1; }
  td { padding: 7px 10px; border: 1px solid #e2e8f0; font-size: 12px; color: #334155; }
  tr:nth-child(even) { background-color: #f8fafc; }
  hr { border: none; border-top: 1px solid #e2e8f0; margin: 18px 0; }
  a { color: #0284c7; text-decoration: none; }
  a:hover { text-decoration: underline; }
  .badge-crit { background-color: #dc2626; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-high { background-color: #ea580c; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-med { background-color: #ca8a04; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-low { background-color: #2563eb; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .badge-ok { background-color: #16a34a; color: #ffffff; padding: 2px 6px; border-radius: 3px; font-weight: bold; font-size: 11px; }
  .card-tip { background-color: #f0fdf4; border-left: 4px solid #16a34a; color: #14532d; padding: 10px 14px; margin: 10px 0; border-radius: 0 6px 6px 0; }
  .card-warn { background-color: #fff7ed; border-left: 4px solid #ea580c; color: #7c2d12; padding: 10px 14px; margin: 10px 0; border-radius: 0 6px 6px 0; }
  .card-info { background-color: #f0f9ff; border-left: 4px solid #0284c7; color: #0c4a6e; padding: 10px 14px; margin: 10px 0; border-radius: 0 6px 6px 0; }
  .lead-quote { font-size: 14px; color: #0284c7; font-style: italic; font-weight: bold; margin: 8px 0; }
</style>
)");
    }
}

} // namespace

void ManualDialog::applyTheme(bool isDark) {
    Win32DwmHelper::applyTitleBarTheme(this, isDark);
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();
    setPalette(pal);

    if (m_browser) {
        m_browser->setPalette(pal);
        if (m_browser->viewport()) {
            m_browser->viewport()->setPalette(pal);
        }
        m_browser->setStyleSheet(isDark ?
            QStringLiteral("QTextBrowser { background-color: #0b1120; color: #e2e8f0; border: 1px solid #1e293b; border-radius: 4px; }") :
            QStringLiteral("QTextBrowser { background-color: #ffffff; color: #1e293b; border: 1px solid #cbd5e1; border-radius: 4px; }"));

        int scrollVal = m_browser->verticalScrollBar() ? m_browser->verticalScrollBar()->value() : 0;
        updateManualHtml();
        if (m_browser->verticalScrollBar()) {
            m_browser->verticalScrollBar()->setValue(scrollVal);
        }
    }

    if (m_searchLabel) {
        m_searchLabel->setStyleSheet(isDark ? QStringLiteral("color: #f0f2f5; font-weight: bold;")
                                            : QStringLiteral("color: #212529; font-weight: bold;"));
    }
    if (m_searchEdit) {
        m_searchEdit->setStyleSheet(isDark ?
            QStringLiteral("QLineEdit { background-color: #18191b; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px 8px; }") :
            QStringLiteral("QLineEdit { background-color: #ffffff; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px 8px; }"));
    }

    auto styleToolBtn = [isDark](QToolButton* btn) {
        if (!btn) return;
        btn->setStyleSheet(isDark ?
            QStringLiteral("QToolButton { background-color: #2d3034; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px; } QToolButton:hover { background-color: #383c42; } QToolButton:disabled { background-color: #1e2023; color: #555a64; }") :
            QStringLiteral("QToolButton { background-color: #f1f5f9; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 4px; padding: 4px; } QToolButton:hover { background-color: #e2e8f0; } QToolButton:disabled { background-color: #f8fafc; color: #94a3b8; }"));
    };
    styleToolBtn(m_searchPrevBtn);
    styleToolBtn(m_searchNextBtn);

    if (m_list) {
        m_list->setStyleSheet(isDark ?
            QStringLiteral("QListWidget { background-color: #18191b; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px; } "
                           "QListWidget::item:selected { background-color: #0078d7; color: #ffffff; border-radius: 3px; } "
                           "QListWidget::item:hover:!selected { background-color: #2d3034; }") :
            QStringLiteral("QListWidget { background-color: #ffffff; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px; } "
                           "QListWidget::item:selected { background-color: #0066cc; color: #ffffff; border-radius: 3px; } "
                           "QListWidget::item:hover:!selected { background-color: #e9ecef; }"));
    }
    if (m_btnClose) {
        m_btnClose->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #2d3034; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 6px 18px; font-weight: bold; } QPushButton:hover { background-color: #383c42; }") :
            QStringLiteral("QPushButton { background-color: #e9ecef; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 6px 18px; font-weight: bold; } QPushButton:hover { background-color: #dee2e6; }"));
    }

    updateSearchHighlights();
}

void ManualDialog::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(10);

    // 1. Barra de busca superior (idêntica ao EisenPo)
    auto* search_layout = new QGridLayout();
    m_searchLabel = new QLabel(this);
    m_searchLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setClearButtonEnabled(true);

    m_searchPrevBtn = new QToolButton(this);
    m_searchPrevBtn->setArrowType(Qt::LeftArrow);
    m_searchPrevBtn->setAutoRaise(true);
    m_searchPrevBtn->setEnabled(false);

    m_searchNextBtn = new QToolButton(this);
    m_searchNextBtn->setArrowType(Qt::RightArrow);
    m_searchNextBtn->setAutoRaise(true);
    m_searchNextBtn->setEnabled(false);

    auto* search_controls = new QHBoxLayout();
    search_controls->setContentsMargins(0, 0, 0, 0);
    search_controls->setSpacing(4);
    search_controls->addWidget(m_searchEdit, 1);
    search_controls->addWidget(m_searchPrevBtn);
    search_controls->addWidget(m_searchNextBtn);

    search_layout->setColumnStretch(0, 0);
    search_layout->setColumnStretch(1, 1);
    search_layout->addWidget(m_searchLabel, 0, 0);
    search_layout->addLayout(search_controls, 0, 1);
    root->addLayout(search_layout);

    // 2. Corpo: Índice lateral à esquerda + Visualizador de documentação à direita
    auto* body = new QHBoxLayout();
    body->setSpacing(10);

    m_list = new QListWidget(this);
    m_list->setMinimumWidth(300);

    m_browser = new QTextBrowser(this);
    m_browser->setReadOnly(true);
    m_browser->setOpenLinks(false);
    m_browser->setOpenExternalLinks(false);

    body->addWidget(m_list, 0);
    body->addWidget(m_browser, 1);
    root->addLayout(body, 1);

    // 3. Rodapé com botão Fechar
    auto* footer = new QHBoxLayout();
    footer->addStretch(1);

    m_btnClose = new QPushButton(this);
    m_btnClose->setAutoDefault(false);
    m_btnClose->setDefault(false);
    m_btnClose->setMinimumWidth(100);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);
    footer->addWidget(m_btnClose);

    root->addLayout(footer);

    // Conexões de busca
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ManualDialog::rebuildSearch);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() { navigateSearch(1); });
    connect(m_searchPrevBtn, &QToolButton::clicked, this, [this]() { navigateSearch(-1); });
    connect(m_searchNextBtn, &QToolButton::clicked, this, [this]() { navigateSearch(1); });

    // Conexões de navegação sincronizada
    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) {
        auto* item = m_list->item(row);
        if (item) {
            scrollToAnchor(item->data(Qt::UserRole).toString());
        }
    });

    connect(m_browser, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        const QString anchor = url.fragment().isEmpty() ? url.toString() : url.fragment();
        scrollToAnchor(anchor);
        const int row = findRowForAnchor(anchor);
        if (row >= 0 && row != m_list->currentRow()) {
            const QSignalBlocker blocker(m_list);
            m_list->setCurrentRow(row);
        }
    });
}

void ManualDialog::retranslateUi() {
    const bool isEn = (m_activeLanguage == QStringLiteral("en_US"));

    setWindowTitle(isEn ? QStringLiteral("Operations Manual and NTFS Compliance Guide — ACL Auditor")
                        : QStringLiteral("Manual de Instruções e Guia de Conformidade NTFS — ACL Auditor"));

    m_searchLabel->setText(isEn ? QStringLiteral("Search:") : QStringLiteral("Buscar:"));
    m_searchEdit->setPlaceholderText(isEn ? QStringLiteral("Search text in manual...")
                                          : QStringLiteral("Buscar texto no manual..."));
    m_searchPrevBtn->setToolTip(isEn ? QStringLiteral("Previous result") : QStringLiteral("Resultado anterior"));
    m_searchNextBtn->setToolTip(isEn ? QStringLiteral("Next result") : QStringLiteral("Próximo resultado"));
    m_searchPrevBtn->setAccessibleName(m_searchPrevBtn->toolTip());
    m_searchNextBtn->setAccessibleName(m_searchNextBtn->toolTip());
    m_btnClose->setText(isEn ? QStringLiteral("Close") : QStringLiteral("Fechar"));

    // Popula m_list com o índice de tópicos e âncoras correspondentes
    const int currentRow = m_list->currentRow() >= 0 ? m_list->currentRow() : 0;
    m_list->blockSignals(true);
    m_list->clear();

    const QString manualTitle = isEn ? QStringLiteral("ACL Auditor — User Manual & NTFS Guide")
                                     : QStringLiteral("ACL Auditor — Manual do Usuário e Guia NTFS");
    auto* rootItem = new QListWidgetItem(manualTitle, m_list);
    rootItem->setData(Qt::UserRole, QStringLiteral("root"));

    auto addSection = [this](const QString& title, const QString& anchor) {
        auto* it = new QListWidgetItem(title, m_list);
        it->setData(Qt::UserRole, anchor);
    };

    if (isEn) {
        addSection(QStringLiteral("1. Overview & NTFS Security Architecture"), QStringLiteral("cap1"));
        addSection(QStringLiteral("2. NTFS Permissions Explorer & DACLs"), QStringLiteral("cap2"));
        addSection(QStringLiteral("3. Effective Access & Visual Explainer"), QStringLiteral("cap3"));
        addSection(QStringLiteral("4. Risk & Compliance Scanner"), QStringLiteral("cap4"));
        addSection(QStringLiteral("5. Windows Services & SMB Shares Audit"), QStringLiteral("cap5"));
        addSection(QStringLiteral("6. Historical Snapshots & Temporal Diff"), QStringLiteral("cap6"));
        addSection(QStringLiteral("7. Consulting Mode & Multi-Format Reports"), QStringLiteral("cap7"));
        addSection(QStringLiteral("8. Safe Remediation & Rollback Guarantee"), QStringLiteral("cap8"));
        addSection(QStringLiteral("9. Windows Explorer Shell Integration"), QStringLiteral("cap9"));
        addSection(QStringLiteral("10. CLI Arguments, Shortcuts & FAQ"), QStringLiteral("cap10"));
    } else {
        addSection(QStringLiteral("1. Visão Geral & Arquitetura NTFS"), QStringLiteral("cap1"));
        addSection(QStringLiteral("2. Navegador de Permissões & DACLs"), QStringLiteral("cap2"));
        addSection(QStringLiteral("3. Acesso Efetivo & Explicador Visual"), QStringLiteral("cap3"));
        addSection(QStringLiteral("4. Scanner de Riscos & Conformidade"), QStringLiteral("cap4"));
        addSection(QStringLiteral("5. Auditoria de Serviços & Compartilhamentos SMB"), QStringLiteral("cap5"));
        addSection(QStringLiteral("6. Snapshots Históricos & Comparação Diff"), QStringLiteral("cap6"));
        addSection(QStringLiteral("7. Modo Consultoria & Relatórios Executivos"), QStringLiteral("cap7"));
        addSection(QStringLiteral("8. Remediação Segura & Garantia de Rollback"), QStringLiteral("cap8"));
        addSection(QStringLiteral("9. Integração com Windows Explorer"), QStringLiteral("cap9"));
        addSection(QStringLiteral("10. Linha de Comando, Atalhos & Dúvidas"), QStringLiteral("cap10"));
    }

    if (m_list->count() > 0) {
        m_list->setCurrentRow(qBound(0, currentRow, m_list->count() - 1));
    }
    m_list->blockSignals(false);

    updateManualHtml();
    rebuildSearch(m_searchEdit->text());
}

void ManualDialog::updateManualHtml() {
    if (!m_browser) {
        return;
    }
    const bool isDark = ThemeManager::instance().isDark();
    const bool isEn = (m_activeLanguage == QStringLiteral("en_US"));
    if (m_browser->document()) {
        m_browser->document()->clear();
    }
    m_browser->setHtml(isEn ? buildManualHtmlEn(isDark) : buildManualHtmlPt(isDark));
}

void ManualDialog::scrollToAnchor(const QString& anchorName) {
    if (!m_browser || anchorName.isEmpty()) {
        return;
    }

    if (anchorName == QStringLiteral("root")) {
        if (m_browser->verticalScrollBar()) {
            m_browser->verticalScrollBar()->setValue(0);
        }
        return;
    }

    QTextDocument* doc = m_browser->document();
    if (doc) {
        for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
            for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
                const QTextFragment fragment = it.fragment();
                if (!fragment.isValid()) continue;

                const QTextCharFormat format = fragment.charFormat();
                if (format.isAnchor() && (format.anchorNames().contains(anchorName) || format.anchorHref() == anchorName)) {
                    QTextCursor cursor(doc);
                    cursor.setPosition(fragment.position());
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    m_browser->setTextCursor(cursor);
                    const qreal blockTop = doc->documentLayout()->blockBoundingRect(cursor.block()).top();
                    if (m_browser->verticalScrollBar()) {
                        m_browser->verticalScrollBar()->setValue(blockTop <= 0.0 ? 0 : static_cast<int>(blockTop));
                    }
                    return;
                }
            }
        }
    }

    m_browser->scrollToAnchor(anchorName);
}

int ManualDialog::findRowForAnchor(const QString& anchorName) const {
    if (!m_list || anchorName.isEmpty()) {
        return -1;
    }
    for (int r = 0; r < m_list->count(); ++r) {
        auto* item = m_list->item(r);
        if (item && item->data(Qt::UserRole).toString() == anchorName) {
            return r;
        }
    }
    return -1;
}

void ManualDialog::rebuildSearch(const QString& text) {
    m_searchState.query = text.trimmed();
    m_searchState.matches.clear();
    m_searchState.current_index = -1;

    if (!m_browser || !m_browser->document() || m_searchState.query.isEmpty()) {
        updateSearchHighlights();
        updateSearchButtons();
        return;
    }

    QTextCursor cursor(m_browser->document());
    while (true) {
        cursor = m_browser->document()->find(m_searchState.query, cursor);
        if (cursor.isNull()) {
            break;
        }
        m_searchState.matches.append(cursor);
    }

    if (!m_searchState.matches.isEmpty()) {
        m_searchState.current_index = 0;
        m_browser->setTextCursor(m_searchState.matches.first());
        m_browser->ensureCursorVisible();
    }

    updateSearchHighlights();
    updateSearchButtons();
}

void ManualDialog::navigateSearch(int step) {
    if (!m_browser || m_searchState.matches.isEmpty() || step == 0) {
        return;
    }

    if (m_searchState.current_index < 0) {
        m_searchState.current_index = step < 0 ? m_searchState.matches.size() - 1 : 0;
    } else {
        m_searchState.current_index = (m_searchState.current_index + step + m_searchState.matches.size()) % m_searchState.matches.size();
    }

    const QTextCursor cursor = m_searchState.matches.at(m_searchState.current_index);
    updateSearchHighlights();
    m_browser->setTextCursor(cursor);
    m_browser->ensureCursorVisible();
}

void ManualDialog::updateSearchHighlights() {
    if (!m_browser || !m_browser->document()) {
        return;
    }

    QList<QTextEdit::ExtraSelection> selections;
    if (m_searchState.query.isEmpty()) {
        m_browser->setExtraSelections(selections);
        return;
    }

    QColor hlColor = palette().color(QPalette::Highlight);
    hlColor.setAlpha(95);
    QColor currColor = palette().color(QPalette::Highlight);
    currColor.setAlpha(190);
    const QColor currTextColor = palette().color(QPalette::HighlightedText);

    for (int i = 0; i < m_searchState.matches.size(); ++i) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = m_searchState.matches.at(i);
        sel.format.setBackground(i == m_searchState.current_index ? currColor : hlColor);
        if (i == m_searchState.current_index) {
            sel.format.setForeground(currTextColor);
        }
        selections.append(sel);
    }

    m_browser->setExtraSelections(selections);
}

void ManualDialog::updateSearchButtons() {
    const bool enabled = !m_searchState.matches.isEmpty();
    if (m_searchPrevBtn) {
        m_searchPrevBtn->setEnabled(enabled);
    }
    if (m_searchNextBtn) {
        m_searchNextBtn->setEnabled(enabled);
    }
}

QString ManualDialog::buildManualHtmlPt(bool isDark) const {
    return QStringLiteral(R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
)") + buildManualCss(isDark) + QStringLiteral(R"(
</head>
<body>

<h1>ACL AUDITOR — MANUAL DO USUÁRIO E GUIA DE CONFORMIDADE NTFS</h1>
<p>O <b>ACL Auditor</b> é uma solução corporativa nativa para Windows desenvolvida em <b>C++20</b> e <b>Qt 6.11.1</b>, projetada para consultorias de infraestrutura, equipes de segurança da informação (SecOps) e administradores de sistemas Windows NT/11/Server. A ferramenta opera em modo <b>100% autônomo, local e somente leitura</b>.</p>

<hr/>

<h2><a name="cap1">1. Visão Geral & Arquitetura de Permissões NTFS</a></h2>
<p>No ecossistema Windows NT, cada arquivo, diretório, serviço ou compartilhamento possui um <b>Security Descriptor (SD)</b> gerenciado pelo Security Reference Monitor (SRM) do kernel. O SD é composto por:</p>
<ul>
  <li><b>Owner (Proprietário):</b> SID da conta com privilégio intrínseco de alterar a DACL (<code>WRITE_DAC</code>).</li>
  <li><b>DACL (Discretionary Access Control List):</b> Lista de ACEs que determinam expressamente permissões de acesso (Allow/Deny).</li>
  <li><b>SACL (System Access Control List):</b> Registros de auditoria forense e geração de logs de eventos de segurança.</li>
  <li><b>Flags de Controle:</b> Indicam herança automática (<code>SE_DACL_PRESENT</code>) ou bloqueio/proteção de herança (<code>SE_DACL_PROTECTED</code>).</li>
</ul>

<div class="card-info">
  <b>Precedência Canônica de ACEs do Windows:</b><br/>
  1. Permissões explícitas do tipo <code>DENY</code> têm a prioridade máxima absoluta.<br/>
  2. Permissões explícitas do tipo <code>ALLOW</code>.<br/>
  3. Permissões herdadas do tipo <code>DENY</code> (da pasta pai mais próxima até a raiz).<br/>
  4. Permissões herdadas do tipo <code>ALLOW</code>.<br/>
  5. Se nenhuma regra permitir o direito solicitado, o acesso é sumariamente bloqueado por <i>Negação Implícita</i>.
</div>

<hr/>

<h2><a name="cap2">2. Navegador de Permissões NTFS & Inspeção de DACLs</a></h2>
<p>O Navegador de Permissões permite inspecionar visualmente qualquer unidade de disco local, volume de dados ou pasta de rede montada:</p>
<ol>
  <li><b>Seleção de Destino:</b> Digite o caminho absoluto (ex.: <code>C:\Windows\System32</code> ou <code>D:\Corporativo</code>) ou clique em <b>Procurar...</b>.</li>
  <li><b>Varredura (F5):</b> Clique em <b>Escanear Permissões</b>. A árvore virtual carrega os nós de forma assíncrona, exibindo:
    <ul>
      <li>Ícone de identificação (Pasta ou Arquivo);</li>
      <li>Nome do objeto e proprietário atual (Owner);</li>
      <li>Status de Herança: <span class="badge-ok">Herdado</span> ou <span class="badge-high">Herança Quebrada</span>;</li>
      <li>Pontuação de Risco: <span class="badge-ok">Conforme</span>, <span class="badge-low">Baixo</span>, <span class="badge-med">Médio</span>, <span class="badge-high">Alto</span> ou <span class="badge-crit">Crítico</span>.</li>
    </ul>
  </li>
  <li><b>Tabela Detalhada de ACEs:</b> Ao selecionar qualquer elemento da árvore, o painel direito exibe todas as regras ativas na DACL:
    <ul>
      <li><b>Tipo:</b> <code>ALLOW</code> (Verde) ou <code>DENY</code> (Vermelho);</li>
      <li><b>Identidade / SID:</b> Nome amigável resolvido (ex: <code>BUILTIN\Administradores</code>) e respectivo SID textual;</li>
      <li><b>Direitos:</b> Controle Total, Modificar, Leitura e Execução, Gravação, Permissões Especiais;</li>
      <li><b>Origem da Herança:</b> Caminho exato da pasta pai que originou a regra (via <code>GetInheritanceSourceW</code>);</li>
      <li><b>Máscara Hexadecimal:</b> Código binário nativo da API Win32 (ex.: <code>0x001F01FF</code> para Controle Total).</li>
    </ul>
  </li>
</ol>

<hr/>

<h2><a name="cap3">3. Acesso Efetivo & Explicador Visual ("Por Que Tem Acesso?")</a></h2>
<p>O diferencial mais importante do ACL Auditor é responder matematicamente e visualmente à pergunta fundamental:</p>
<p class="lead-quote">“Por que este usuário consegue ler, alterar ou executar este arquivo?”</p>

<h3>Passo a Passo de Operação:</h3>
<ol>
  <li>Selecione um arquivo ou diretório no navegador e clique em <b>Por Que Tem Acesso?</b>;</li>
  <li>Na aba de Acesso Efetivo, escolha uma conta no seletor integrado ou digite o nome de usuário (ex: <code>fernando</code>, <code>TI-Suporte</code> ou <code>S-1-5-21-...</code>);</li>
  <li>Clique em <b>Explicar Cadeia de Acesso Efetivo</b>;</li>
  <li>O sistema resolve instantaneamente:
    <ul>
      <li>O token de segurança da conta alvo;</li>
      <li>Todos os grupos primários, grupos locais e grupos universais dos quais é membro (<code>Everyone</code>, <code>Authenticated Users</code>, <code>BUILTIN\Users</code>);</li>
      <li>Avaliação das regras explícitas e herdadas aplicando o algoritmo rigoroso do Security Reference Monitor (SRM).</li>
    </ul>
  </li>
  <li><b>Painel de Veredito Rápido:</b> Apresenta badges com os direitos efetivos consolidados:
    <span class="badge-ok">LEITURA</span> <span class="badge-med">GRAVAÇÃO</span> <span class="badge-ok">EXECUÇÃO</span> <span class="badge-crit">EXCLUSÃO</span> <span class="badge-high">ALTERAÇÃO DE ACL</span>
  </li>
  <li><b>Árvore Causal Explicativa:</b> Traça a genealogia da concessão:
<pre>
Identidade Analisada: joao.silva (S-1-5-21-100234-...)
 └── Membro do Grupo: TI-Suporte (S-1-5-21-445892-...)
      └── Grupo Aninhado: Operações-Infra
           └── Origem da Regra: Herdada de D:\Departamentos
                └── Permissão Concedida: MODIFICAR (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
</pre>
  </li>
  <li><b>Parecer Técnico em Linguagem Natural:</b> Uma síntese executiva pronta para ser incluída em laudos periciais e relatórios de auditoria.</li>
</ol>

<hr/>

<h2><a name="cap4">4. Scanner de Riscos & Matriz de Conformidade</a></h2>
<p>O Scanner de Riscos realiza análises heurísticas automáticas baseadas nos padrões <b>CIS Microsoft Windows Benchmarks</b> e <b>NIST SP 800-53</b>:</p>

<table>
  <tr>
    <th>Categoria de Risco</th>
    <th>Severidade</th>
    <th>Descrição Técnica e Impacto</th>
  </tr>
  <tr>
    <td><b>Pastas World-Writable</b></td>
    <td><span class="badge-crit">CRÍTICO</span></td>
    <td>Diretórios do sistema, de aplicativos ou de dados onde <code>Everyone</code>, <code>Users</code> ou <code>Authenticated Users</code> possuem direitos de gravação ou criação de arquivos, permitindo persistência e movimentação lateral.</td>
  </tr>
  <tr>
    <td><b>Binary Planting & DLL Hijacking</b></td>
    <td><span class="badge-crit">CRÍTICO</span></td>
    <td>Executáveis (<code>.exe</code>), bibliotecas dinâmicas (<code>.dll</code>), drivers (<code>.sys</code>) ou scripts (<code>.bat</code>, <code>.ps1</code>) situados em pastas graváveis por usuários comuns, permitindo elevação de privilégios via substituição de binários.</td>
  </tr>
  <tr>
    <td><b>Controle Excessivo de DACL</b></td>
    <td><span class="badge-high">ALTO</span></td>
    <td>Contas não administrativas com direitos <code>WRITE_DAC</code> (alterar permissões) ou <code>WRITE_OWNER</code> (apropriar-se do arquivo), contornando qualquer restrição corporativa.</td>
  </tr>
  <tr>
    <td><b>Contas Órfãs / SIDs Inexistentes</b></td>
    <td><span class="badge-med">MÉDIO</span></td>
    <td>Registros ACE associados a SIDs que não existem mais no SAM local ou Active Directory (ex-funcionários ou grupos deletados). Risco de reaproveitamento de SIDs em florestas legadas.</td>
  </tr>
  <tr>
    <td><b>Herança Quebrada Não Homologada</b></td>
    <td><span class="badge-med">MÉDIO</span></td>
    <td>Diretórios com a flag <code>SE_DACL_PROTECTED</code> que isolam o nó das políticas corporativas da raiz, facilitando bolsões de permissões clandestinas.</td>
  </tr>
  <tr>
    <td><b>Exposição de Dados Confidenciais</b></td>
    <td><span class="badge-high">ALTO</span></td>
    <td>Pastas com termos sensíveis no nome (<code>RH</code>, <code>Financeiro</code>, <code>Folha</code>, <code>Backup</code>, <code>Senhas</code>) com acesso liberado a grupos genéricos.</td>
  </tr>
</table>

<div class="card-tip">
  <b>Ação Rápida de Remediação:</b> Selecione qualquer vulnerabilidade na tabela de riscos para visualizar imediatamente o comando pronto em <code>icacls</code> e <code>PowerShell</code>, ou clique em <b>Gerar Plano de Mitigação</b>.
</div>

<hr/>

<h2><a name="cap5">5. Auditoria de Serviços Windows & Compartilhamentos SMB</a></h2>

<h3>5.1. Serviços do Windows (Service Control Manager)</h3>
<p>Clique em <b>Auditar Serviços do Windows</b> para que a engine enumere todos os serviços instalados via <code>EnumServicesStatusExW</code> e avalie:</p>
<ul>
  <li><b>Unquoted Service Paths (Caminhos sem Aspas com Espaços):</b> Serviços configurados como <code>C:\Program Files\App Dir\service.exe</code> sem aspas. O Windows tenta executar <code>C:\Program.exe</code> e <code>C:\Program Files\App.exe</code> antes do serviço legítimo, permitindo escalada direta para <code>NT AUTHORITY\SYSTEM</code>.</li>
  <li><b>Binários de Serviço Graváveis:</b> Binários executáveis cujas ACLs permitem modificação por contas comuns.</li>
</ul>

<h3>5.2. Compartilhamentos de Rede SMB (Server Message Block)</h3>
<p>Clique em <b>Auditar Compartilhamentos de Rede (SMB)</b> para que a engine execute:</p>
<ul>
  <li>Inspeção das ACLs do compartilhamento via <code>NetShareGetInfo</code> (nível 502);</li>
  <li>Detecção de compartilhamentos administrativos e compartilhamentos com <code>Everyone</code> com Controle Total;</li>
  <li><b>Cruzamento de Matriz SMB x NTFS:</b> Alerta de discrepâncias graves onde o compartilhamento de rede é restrito, mas a pasta local subjacente é World-Writable, ou vice-versa.</li>
</ul>

<hr/>

<h2><a name="cap6">6. Snapshots Históricos & Comparação Temporal (Diff SQLite)</a></h2>
<p>O ACL Auditor inclui um banco de dados local autônomo baseado em <b>SQLite3</b> (localizado em <code>%LOCALAPPDATA%/ACLAuditor/ACLAuditor/acl_auditor_snapshots.db</code>) para registrar o estado de segurança ao longo do tempo:</p>
<ol>
  <li><b>Criar Snapshot:</b> Clique em <b>Salvar Snapshot</b>. A aplicação captura todas as DACLs, proprietários e permissões do diretório atual, gerando um hash de integridade <b>SHA-256</b> como evidência forense.</li>
  <li><b>Comparação Temporal (Diff):</b> Na aba <i>Comparação Temporal</i>, selecione o Snapshot Base (ex.: antes de uma migração ou na última semana) e o Snapshot Atual.</li>
  <li><b>Calcular Diff:</b> O motor analisa as diferenças nó a nó e classifica:
    <ul>
      <li><span class="badge-high">Novo Acesso Concedido:</span> Novas contas adicionadas à DACL;</li>
      <li><span class="badge-low">Acesso Revogado:</span> Contas ou permissões removidas;</li>
      <li><span class="badge-crit">Elevação de Privilégio:</span> Uma conta padrão que obteve controle de escrita ou total;</li>
      <li><span class="badge-med">Alteração de Proprietário:</span> Modificação da conta proprietária do arquivo;</li>
      <li><span class="badge-warn">Alteração de Herança:</span> Bloqueio ou desbloqueio de herança.</li>
    </ul>
  </li>
</ol>

<hr/>

<h2><a name="cap7">7. Modo Consultoria & Geração de Relatórios Executivos</a></h2>
<p>Desenvolvido para consultorias de segurança, prestadores de serviços gerenciados (MSP) e relatórios de auditoria interna:</p>
<ol>
  <li>Acesse o menu <b>Arquivo → Exportar Relatório Executivo...</b> (ou <code>Ctrl+E</code>);</li>
  <li><b>Formatos Disponíveis:</b>
    <ul>
      <li><b>HTML Interativo:</b> Relatório web responsivo com gráficos, tabelas filtráveis e visualização da árvore de riscos, 100% autossuficiente para abertura no Edge, Chrome ou Firefox;</li>
      <li><b>PDF Executivo:</b> Documento diagramado em formato A4, com quebra de páginas, paginação, índice, sumário executivo e espaço para assinatura de conformidade;</li>
      <li><b>CSV Estruturado:</b> Arquivo tabular com cabeçalho completo e codificação UTF-8 BOM, pronto para abertura direta no Microsoft Excel, Power BI ou ingestão em SIEMs;</li>
      <li><b>JSON Estruturado:</b> Formato estruturado para integração em esteiras DevSecOps e pipelines de auditoria contínua.</li>
    </ul>
  </li>
  <li><b>Identidade Visual da Consultoria:</b> Configure o nome da consultoria, nome do cliente avaliado, auditor responsável e carregue o arquivo de logotipo institucional para estampar a capa do laudo.</li>
</ol>

<hr/>

<h2><a name="cap8">8. Remediação Segura, Scripts Automatizados e Rollback</a></h2>
<p>O ACL Auditor adota o princípio de <b>Segurança Não-Destrutiva</b>:</p>
<ul>
  <li>A aplicação <b>nunca altera permissões diretamente sem confirmação e geração de scripts transparentes</b>;</li>
  <li>Ao gerar uma remediação, o sistema produz automaticamente um <b>Script de Correção</b> e um <b>Script de Rollback (Restauração)</b>;</li>
  <li><b>Backup Obrigatório da DACL Prévia:</b> Utiliza o comando nativo <code>icacls /save &lt;arquivo_backup&gt; /t /c</code> antes de executar qualquer alteração;</li>
  <li><b>Script de Rollback Imediato:</b> Executa <code>icacls &lt;diretorio_pai&gt; /restore &lt;arquivo_backup&gt;</code> para reverter o sistema de arquivos ao estado original em caso de imprevistos;</li>
  <li>Scripts são exportados em formato <code>.bat</code> (icacls) e <code>.ps1</code> (PowerShell com tratamento rigoroso de exceções).</li>
</ul>

<hr/>

<h2><a name="cap9">9. Integração com o Windows Explorer (Menu de Contexto)</a></h2>
<p>Para agilizar a rotina do administrador, o ACL Auditor pode ser integrado ao menu de contexto do Windows Explorer:</p>
<ul>
  <li>No menu superior, selecione <b>Ferramentas → Integrar ao Windows Explorer</b>;</li>
  <li>O registro é realizado na chave do usuário atual (<code>HKCU\Software\Classes\Directory\shell\AclAuditor</code>), <b>não requerendo direitos de administrador</b> para instalação;</li>
  <li>Ao clicar com o botão direito sobre qualquer pasta no Windows Explorer, escolha <b>Auditar Permissões com ACL Auditor</b> para abrir a pasta diretamente na aplicação.</li>
</ul>

<hr/>

<h2><a name="cap10">10. Linha de Comando (CLI), Atalhos & Resolução de Dúvidas</a></h2>

<h3>10.1. Linha de Comando e Automação (CLI)</h3>
<p>O ACL Auditor disponibiliza o utilitário standalone <b><code>acl-auditor-cli.exe</code></b> para automação sem interface gráfica (CI/CD, servidores headless, PowerShell e SIEM), além dos argumentos do executável gráfico <b><code>ACLAuditor.exe</code></b>:</p>

<p><b>A. Utilitário Standalone de Console (<code>acl-auditor-cli.exe</code>):</b></p>
<pre>
acl-auditor-cli.exe &lt;comando&gt; [opções]

Comandos:
  scan &lt;caminho&gt;              Audita permissões NTFS, detecta vulnerabilidades e calcula o risco (0-100).
  access &lt;caminho&gt; --user &lt;u&gt; Calcula e explica o acesso efetivo do usuário/grupo sobre o destino.
  services                    Audita serviços do Windows (Unquoted Paths e binários graváveis).
  shares                      Audita compartilhamentos de rede SMB locais.
  --version, -v               Exibe a versão oficial instalada.
  --help, -h                  Exibe o manual de ajuda no terminal.

Opções:
  --format &lt;text|json&gt;        Formato de saída (padrão: text; use json para automação e pipes).
  --depth &lt;n&gt;                 Profundidade de varredura na árvore de diretórios (padrão: 2).
  --risks-only                Exibe exclusivamente vulnerabilidades e comandos de remediação.
  --no-banner                 Suprime o banner inicial em modo texto.
</pre>
<div class="card-tip">
  <b>Atalho Global (Windows Terminal / MSIX):</b> Se instalado via Microsoft Store com <code>AppExecutionAlias</code>, execute diretamente digitando <code>acl-auditor-cli</code> no PowerShell ou CMD.
</div>

<p><b>B. Argumentos do Executável Gráfico (<code>ACLAuditor.exe</code>):</b></p>
<pre>
ACLAuditor.exe [opções] [caminho]

Opções:
  caminho                     Abre a interface gráfica já iniciando a varredura do caminho informado.
  --scan &lt;diretorio&gt;          Inicia a auditoria direta no diretório especificado.
  --snapshot                  Captura e armazena um snapshot do diretório informado no SQLite.
  --report &lt;formato&gt; &lt;saida&gt;  Gera o relatório do diretório nos formatos: html, pdf, csv ou json.
  --help                      Exibe as instruções da linha de comando.
  --version                   Exibe a versão oficial instalada.
</pre>

<h3>10.2. Tabela de Atalhos de Teclado</h3>
<table>
  <tr><th>Atalho</th><th>Ação</th></tr>
  <tr><td><code>F5</code></td><td>Escanear / Atualizar permissões do diretório atual</td></tr>
  <tr><td><code>Ctrl+O</code></td><td>Abrir diálogo de seleção de diretório</td></tr>
  <tr><td><code>Ctrl+E</code></td><td>Exportar Relatório Executivo</td></tr>
  <tr><td><code>Ctrl+S</code></td><td>Salvar Snapshot de Conformidade</td></tr>
  <tr><td><code>Ctrl+F</code></td><td>Focar campo de pesquisa rápida</td></tr>
  <tr><td><code>F1</code></td><td>Abrir este Manual de Instruções</td></tr>
  <tr><td><code>Ctrl+Q</code></td><td>Encerrar a aplicação</td></tr>
</table>

<h3>10.3. Resolução de Dúvidas Frequentes (FAQ)</h3>
<p><b>P: Por que algumas pastas do sistema retornam "Acesso Negado"?</b><br/>
<i>R:</i> Diretórios restritos do Windows (como <code>C:\System Volume Information</code>) exigem elevação administrativa e habilitação do privilégio <code>SeSecurityPrivilege</code>. Execute o ACL Auditor como Administrador para inspeção completa.</p>

<p><b>P: A ferramenta altera arquivos ou instala serviços em segundo plano?</b><br/>
<i>R:</i> Não. O ACL Auditor é 100% autônomo, não cria serviços residentes, não transmite dados para servidores remotos e realiza operações estritamente em modo somente leitura.</p>

<p><b>P: Como auditar um compartilhamento de rede oculto administrativo (ex.: \\Servidor\C$)?</b><br/>
<i>R:</i> Digite o caminho UNC completo na barra de endereços (<code>\\Servidor\C$\Dados</code>) e certifique-se de que sua conta possua credenciais válidas no servidor de destino.</p>

<br/><hr/>
<p style="text-align: center; color: #64748b; font-size: 11px;">
ACL Auditor • Guia de Conformidade e Auditoria NTFS • © 2026 Fernando Nillsson Cidade
</p>
</body>
</html>
)");
}

QString ManualDialog::buildManualHtmlEn(bool isDark) const {
    return QStringLiteral(R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
)") + buildManualCss(isDark) + QStringLiteral(R"(
</head>
<body>

<h1>ACL AUDITOR — USER MANUAL & NTFS COMPLIANCE GUIDE</h1>
<p><b>ACL Auditor</b> is an enterprise-grade native Windows application built in <b>C++20</b> and <b>Qt 6.11.1</b>, designed specifically for IT infrastructure consultancies, SecOps teams, and Windows NT/11/Server system administrators. The application operates in a <b>100% autonomous, local, and read-only mode</b>.</p>

<hr/>

<h2><a name="cap1">1. Overview & NTFS Security Architecture</a></h2>
<p>Within Microsoft Windows NT file systems, every file, directory, registry key, service, and network share is governed by a <b>Security Descriptor (SD)</b> managed by the Windows Security Reference Monitor (SRM). The SD consists of:</p>
<ul>
  <li><b>Owner SID:</b> The security identifier with inherent privilege to modify the DACL (<code>WRITE_DAC</code>).</li>
  <li><b>DACL (Discretionary Access Control List):</b> An ordered list of ACEs that explicitly allow or deny permissions.</li>
  <li><b>SACL (System Access Control List):</b> Audit logging rules that record security access attempts to the Windows Security Event Log.</li>
  <li><b>Control Flags:</b> Indicators of inheritance propagation (<code>SE_DACL_PRESENT</code>) or inheritance blocking (<code>SE_DACL_PROTECTED</code>).</li>
</ul>

<div class="card-info">
  <b>Canonical ACE Ordering Precedence:</b><br/>
  1. Explicit <code>DENY</code> entries have highest precedence.<br/>
  2. Explicit <code>ALLOW</code> entries.<br/>
  3. Inherited <code>DENY</code> entries (evaluated from closest parent up to drive root).<br/>
  4. Inherited <code>ALLOW</code> entries.<br/>
  5. If no ACE grants the requested access rights, access is rejected via <i>Implicit Deny</i>.
</div>

<hr/>

<h2><a name="cap2">2. NTFS Permissions Explorer & DACLs</a></h2>
<p>The Permissions Explorer enables deep inspection of local hard drives, storage volumes, and mounted network paths:</p>
<ol>
  <li><b>Target Selection:</b> Enter any path (e.g., <code>C:\Windows\System32</code> or <code>D:\CorporateData</code>) or click <b>Browse...</b>.</li>
  <li><b>Scan Execution (F5):</b> Click <b>Scan Directory</b>. The virtual tree populates asynchronously, rendering:
    <ul>
      <li>Object Type (Folder or File);</li>
      <li>Object Name and Current Owner;</li>
      <li>Inheritance Status: <span class="badge-ok">Inherited</span> or <span class="badge-high">Inheritance Blocked</span>;</li>
      <li>Calculated Risk Rating: <span class="badge-ok">Compliant</span>, <span class="badge-low">Low</span>, <span class="badge-med">Medium</span>, <span class="badge-high">High</span>, or <span class="badge-crit">Critical</span>.</li>
    </ul>
  </li>
  <li><b>Detailed ACE Inspection Table:</b> Selecting any tree item reveals all active DACL entries:
    <ul>
      <li><b>Type:</b> <code>ALLOW</code> (Green) or <code>DENY</code> (Red);</li>
      <li><b>Identity / SID:</b> Resolved account/group name (e.g., <code>BUILTIN\Administrators</code>) and raw string SID;</li>
      <li><b>Rights:</b> Full Control, Modify, Read & Execute, Write, Special Permissions;</li>
      <li><b>Inheritance Source:</b> Exact ancestor folder path where the rule originated (via <code>GetInheritanceSourceW</code>);</li>
      <li><b>Hex Mask:</b> Low-level Win32 hexadecimal mask (e.g., <code>0x001F01FF</code> for Full Control).</li>
    </ul>
  </li>
</ol>

<hr/>

<h2><a name="cap3">3. Effective Access & Visual Explainer ("Why Does It Have Access?")</a></h2>
<p>The defining capability of ACL Auditor is answering with mathematical precision and visual clarity:</p>
<p class="lead-quote">“Why does this user have permission to read, modify, or execute this file?”</p>

<h3>Step-by-Step Operations:</h3>
<ol>
  <li>Select any file or directory in the Explorer tree and click <b>Why Does It Have Access?</b>;</li>
  <li>In the Effective Access tab, pick a user from the account selector or type any account, group, or SID;</li>
  <li>Click <b>Explain Effective Access Chain</b>;</li>
  <li>The engine resolves:
    <ul>
      <li>The target identity's security token;</li>
      <li>All primary groups, local security groups, and universal SID groups (<code>Everyone</code>, <code>Authenticated Users</code>, <code>BUILTIN\Users</code>);</li>
      <li>Strict SRM token evaluation against all explicit and inherited ACE entries.</li>
    </ul>
  </li>
  <li><b>Instant Verdict Badges:</b>
    <span class="badge-ok">READ</span> <span class="badge-med">WRITE</span> <span class="badge-ok">EXECUTE</span> <span class="badge-crit">DELETE</span> <span class="badge-high">CHANGE PERMISSIONS</span>
  </li>
  <li><b>Hierarchical Causal Tree:</b> Visualizes the full authority chain:
<pre>
Analyzed Identity: john.doe (S-1-5-21-100234-...)
 └── Group Membership: IT-Support (S-1-5-21-445892-...)
      └── Nested Group: Infra-Operations
           └── Rule Inheritance: Inherited from D:\Departments
                └── Granted Permission: MODIFY (FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE)
</pre>
  </li>
  <li><b>Natural Language Technical Report:</b> Executive technical summary ready for compliance documentation.</li>
</ol>

<hr/>

<h2><a name="cap4">4. Risk & Compliance Scanner</a></h2>
<p>The Risk Scanner evaluates files and folders against <b>CIS Microsoft Windows Benchmarks</b> and <b>NIST SP 800-53</b> standards:</p>

<table>
  <tr>
    <th>Risk Category</th>
    <th>Severity</th>
    <th>Technical Description & Exposure</th>
  </tr>
  <tr>
    <td><b>World-Writable Directories</b></td>
    <td><span class="badge-crit">CRITICAL</span></td>
    <td>System, program, or corporate directories where <code>Everyone</code>, <code>Users</code>, or <code>Authenticated Users</code> possess write or file-creation rights, enabling lateral movement and persistence.</td>
  </tr>
  <tr>
    <td><b>Binary Planting & DLL Hijacking</b></td>
    <td><span class="badge-crit">CRITICAL</span></td>
    <td>Executable binaries (<code>.exe</code>), dynamic libraries (<code>.dll</code>), kernel drivers (<code>.sys</code>), or scripts (<code>.bat</code>, <code>.ps1</code>) residing in writable directories, allowing privilege escalation via binary substitution.</td>
  </tr>
  <tr>
    <td><b>Excessive Control Privileges</b></td>
    <td><span class="badge-high">HIGH</span></td>
    <td>Non-administrative accounts granted <code>WRITE_DAC</code> (permission modification) or <code>WRITE_OWNER</code> (take ownership), allowing circumvention of security controls.</td>
  </tr>
  <tr>
    <td><b>Orphaned Accounts / Unresolved SIDs</b></td>
    <td><span class="badge-med">MEDIUM</span></td>
    <td>ACE entries linked to deleted Active Directory or local SAM accounts. Poses security risks of SID reuse or unmanaged legacy access.</td>
  </tr>
  <tr>
    <td><b>Unsanctioned Inheritance Breaks</b></td>
    <td><span class="badge-med">MEDIUM</span></td>
    <td>Directories configured with <code>SE_DACL_PROTECTED</code> that block corporate governance policies from parent paths.</td>
  </tr>
  <tr>
    <td><b>Sensitive Asset Exposure</b></td>
    <td><span class="badge-high">HIGH</span></td>
    <td>Folders containing confidential naming patterns (<code>HR</code>, <code>Finance</code>, <code>Payroll</code>, <code>Backup</code>, <code>Passwords</code>) accessible by broad user groups.</td>
  </tr>
</table>

<div class="card-tip">
  <b>Instant Remediation:</b> Click any finding in the risk table to view ready-to-run <code>icacls</code> and <code>PowerShell</code> commands, or click <b>Generate Mitigation Plan</b>.
</div>

<hr/>

<h2><a name="cap5">5. Windows Services & SMB Shares Audit</a></h2>

<h3>5.1. Windows Services (Service Control Manager)</h3>
<p>Click <b>Audit Windows Services</b> to inspect all services registered in the Service Control Manager via <code>EnumServicesStatusExW</code>:</p>
<ul>
  <li><b>Unquoted Service Paths:</b> Binaries configured with spaces and lacking quotes (e.g., <code>C:\Program Files\Vendor App\service.exe</code>). The Windows Service Manager attempts executing <code>C:\Program.exe</code> first, yielding immediate privilege escalation to <code>NT AUTHORITY\SYSTEM</code>.</li>
  <li><b>Writable Service Binaries:</b> Executable service files writable by standard user groups.</li>
</ul>

<h3>5.2. SMB Network Shares</h3>
<p>Click <b>Audit Network Shares (SMB)</b> to evaluate network shares via <code>NetShareGetInfo</code> (level 502):</p>
<ul>
  <li>Identifies administrative and open shares with <code>Everyone</code> granted Full Control;</li>
  <li><b>SMB vs NTFS Discrepancy Matrix:</b> Highlights dangerous mismatches where the network share appears restricted, but the underlying NTFS folder is world-writable, or vice versa.</li>
</ul>

<hr/>

<h2><a name="cap6">6. Historical Snapshots & Temporal Diff (SQLite)</a></h2>
<p>ACL Auditor features an embedded <b>SQLite3</b> database engine (stored in <code>%LOCALAPPDATA%/ACLAuditor/ACLAuditor/acl_auditor_snapshots.db</code>) for historical access baselines:</p>
<ol>
  <li><b>Capture Snapshot:</b> Click <b>Save Snapshot</b>. The engine records all DACLs, owners, and permissions, generating a cryptographic <b>SHA-256</b> hash for forensic custody.</li>
  <li><b>Temporal Diff:</b> On the <i>Temporal Comparison</i> tab, pick a Baseline Snapshot and a Target Snapshot.</li>
  <li><b>Calculate Diff:</b> The engine compares trees node-by-node, detecting:
    <ul>
      <li><span class="badge-high">New Access Granted:</span> Accounts newly added to DACL;</li>
      <li><span class="badge-low">Access Revoked:</span> Accounts or rights removed;</li>
      <li><span class="badge-crit">Privilege Escalation:</span> Standard accounts granted write or full control;</li>
      <li><span class="badge-med">Owner Change:</span> Alterations in object ownership;</li>
      <li><span class="badge-warn">Inheritance Toggle:</span> Blocking or unblocking of DACL inheritance.</li>
    </ul>
  </li>
</ol>

<hr/>

<h2><a name="cap7">7. Consulting Mode & Multi-Format Reports</a></h2>
<p>Designed for security consultancies, Managed Service Providers (MSP), and corporate compliance officers:</p>
<ol>
  <li>Navigate to <b>File → Export Executive Report...</b> (or <code>Ctrl+E</code>);</li>
  <li><b>Supported Formats:</b>
    <ul>
      <li><b>Interactive HTML:</b> Responsive, self-contained dashboard with charts, filterable risk tables, and visual tree maps, viewable in any modern browser;</li>
      <li><b>Executive PDF:</b> Polished A4 document with cover page, table of contents, executive summary, risk matrices, and formal auditor sign-off section;</li>
      <li><b>Structured CSV:</b> Tabular dataset with UTF-8 BOM encoding for direct import into Excel, Power BI, or SIEM platforms;</li>
      <li><b>Structured JSON:</b> Machine-readable format for automated DevSecOps pipelines.</li>
    </ul>
  </li>
  <li><b>Consultancy Branding:</b> Configure Consultancy Name, Client Name, Lead Auditor Name, and embed your corporate PNG/JPEG logo on the report cover.</li>
</ol>

<hr/>

<h2><a name="cap8">8. Safe Remediation & Rollback Guarantee</a></h2>
<p>ACL Auditor adheres strictly to the <b>Non-Destructive Principle</b>:</p>
<ul>
  <li>The software <b>never applies permission changes directly or automatically</b>;</li>
  <li>When generating remediation, both a <b>Fix Script</b> and a <b>Rollback Script</b> are produced;</li>
  <li><b>Mandatory DACL Pre-Backup:</b> Utilizes <code>icacls /save &lt;backup_file&gt; /t /c</code> before touching any ACE;</li>
  <li><b>Guaranteed Rollback:</b> Utilizes <code>icacls &lt;parent_dir&gt; /restore &lt;backup_file&gt;</code> to revert the file system immediately if required;</li>
  <li>Outputs both Windows Command Prompt (<code>.bat</code>) and PowerShell (<code>.ps1</code>) scripts.</li>
</ul>

<hr/>

<h2><a name="cap9">9. Windows Explorer Shell Integration</a></h2>
<p>For administrative productivity, ACL Auditor seamlessly integrates with the Windows Explorer shell:</p>
<ul>
  <li>From the top menu, select <b>Tools → Integrate with Windows Explorer</b>;</li>
  <li>Integration is written to Current User registry (<code>HKCU\Software\Classes\Directory\shell\AclAuditor</code>) — <b>no administrator elevation required</b>;</li>
  <li>Right-click any folder in Windows Explorer and select <b>Audit Permissions with ACL Auditor</b> to launch immediate inspection.</li>
</ul>

<hr/>

<h2><a name="cap10">10. CLI Arguments, Shortcuts & Troubleshooting</a></h2>

<h3>10.1. Command Line Interface & Automation (CLI)</h3>
<p>ACL Auditor delivers the dedicated standalone console tool <b><code>acl-auditor-cli.exe</code></b> for headless automation (CI/CD, Windows Server Core, PowerShell, and SIEM), alongside arguments for the graphical executable <b><code>ACLAuditor.exe</code></b>:</p>

<p><b>A. Standalone Headless Console Utility (<code>acl-auditor-cli.exe</code>):</b></p>
<pre>
acl-auditor-cli.exe &lt;command&gt; [options]

Commands:
  scan &lt;path&gt;                 Audits NTFS permissions, discovers vulnerabilities, and scores risk (0-100).
  access &lt;path&gt; --user &lt;u&gt;    Calculates and explains effective access rights for user/group on path.
  services                    Audits Windows services (Unquoted Service Paths &amp; writable binaries).
  shares                      Audits local SMB network shares.
  --version, -v               Displays official release version.
  --help, -h                  Displays terminal help message.

Options:
  --format &lt;text|json&gt;        Output format (default: text; use json for automation/pipes).
  --depth &lt;n&gt;                 Scan depth for directory tree recursion (default: 2).
  --risks-only                Restricts output strictly to identified security vulnerabilities.
  --no-banner                 Suppresses initial banner in text mode.
</pre>
<div class="card-tip">
  <b>Global Alias (Windows Terminal / MSIX):</b> When installed from the Microsoft Store with <code>AppExecutionAlias</code>, execute directly from any folder by typing <code>acl-auditor-cli</code>.
</div>

<p><b>B. Graphical Executable Arguments (<code>ACLAuditor.exe</code>):</b></p>
<pre>
ACLAuditor.exe [options] [path]

Options:
  path                        Launches GUI and initiates scan on specified path.
  --scan &lt;directory&gt;          Starts automated audit on target directory.
  --snapshot                  Captures and saves a baseline snapshot to local SQLite.
  --report &lt;format&gt; &lt;output&gt;  Generates executive report (html, pdf, csv, json).
  --help                      Displays CLI usage information.
  --version                   Prints official release version.
</pre>

<h3>10.2. Keyboard Shortcuts</h3>
<table>
  <tr><th>Shortcut</th><th>Action</th></tr>
  <tr><td><code>F5</code></td><td>Scan / Refresh directory permissions</td></tr>
  <tr><td><code>Ctrl+O</code></td><td>Browse for folder dialog</td></tr>
  <tr><td><code>Ctrl+E</code></td><td>Export Executive Report</td></tr>
  <tr><td><code>Ctrl+S</code></td><td>Save Compliance Snapshot</td></tr>
  <tr><td><code>Ctrl+F</code></td><td>Focus quick search bar</td></tr>
  <tr><td><code>F1</code></td><td>Open this Operations Manual</td></tr>
  <tr><td><code>Ctrl+Q</code></td><td>Exit application</td></tr>
</table>

<h3>10.3. Frequently Asked Questions (FAQ)</h3>
<p><b>Q: Why do certain system folders report "Access Denied"?</b><br/>
<i>A:</i> Protected system directories (e.g., <code>C:\System Volume Information</code>) require running ACL Auditor with elevated administrator privileges and the <code>SeSecurityPrivilege</code> token enabled.</p>

<p><b>Q: Does ACL Auditor send telemetry or data externally?</b><br/>
<i>A:</i> No. ACL Auditor is 100% offline and autonomous. It performs zero telemetry, installs no resident background services, and makes no network connections.</p>

<p><b>Q: Can I audit administrative hidden shares (e.g., \\Server\C$)?</b><br/>
<i>A:</i> Yes. Enter the full UNC path (<code>\\Server\C$\Data</code>) in the path field. Ensure your current Windows logon has administrative access to the remote server.</p>

<br/><hr/>
<p style="text-align: center; color: #64748b; font-size: 11px;">
ACL Auditor • NTFS Compliance & Audit Guide • © 2026 Fernando Nillsson Cidade
</p>
</body>
</html>
)");
}

} // namespace AclAuditor::UI
