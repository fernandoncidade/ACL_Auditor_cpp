#include "AboutDialog.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include "../utils/ApplicationPathUtils.hpp"
#include "../utils/LogManager.hpp"
#include "ThemeManager.hpp"
#include "Win32DwmHelper.hpp"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace AclAuditor::UI {

namespace {

QString loadAssetFile(const QString& relativeFolder, const QStringList& candidateNames) {
    const QString assetsDir = Utils::ApplicationPathUtils::getAssetsDirPath();
    const QString folderPath = assetsDir + QStringLiteral("/") + relativeFolder;

    for (const QString& name : candidateNames) {
        const QString fullPath = folderPath + QStringLiteral("/") + name;
        if (QFile::exists(fullPath)) {
            QFile file(fullPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return QString::fromUtf8(file.readAll());
            }
        }
    }
    return QString();
}

QString renderClickableBody(const QString& text) {
    if (text.isEmpty()) return QStringLiteral("<p><i>Informação não disponível.</i></p>");

    static const QRegularExpression urlRegex(QStringLiteral(R"(https?://[^\s<>"']+)"));

    QString html;
    int lastPos = 0;
    auto matchIt = urlRegex.globalMatch(text);
    while (matchIt.hasNext()) {
        auto match = matchIt.next();
        html += text.mid(lastPos, match.capturedStart() - lastPos).toHtmlEscaped();
        const QString url = match.captured(0);
        html += QStringLiteral("<a href=\"") + url.toHtmlEscaped() + QStringLiteral("\">") + url.toHtmlEscaped() + QStringLiteral("</a>");
        lastPos = match.capturedEnd();
    }
    html += text.mid(lastPos).toHtmlEscaped();

    return QStringLiteral("<div style=\"white-space: pre-wrap; font-family: 'Segoe UI', Tahoma, sans-serif; font-size: 10pt; line-height: 1.5;\">") + html + QStringLiteral("</div>");
}

} // namespace

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(Language::GerenciadorTraducao::trText("Sobre o ACL Auditor"));
    setWindowIcon(Utils::ApplicationPathUtils::getAppIcon());
    resize(860, 640);
    setMinimumSize(700, 500);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, true);

    Win32DwmHelper::applyMicaAndDarkMode(this);

    setupUi();
    carregarTextos(Language::GerenciadorTraducao::instance().obter_idioma_atual());
    retranslateUi();

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &AboutDialog::onIdiomaAlterado);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &AboutDialog::applyTheme);

    applyTheme(ThemeManager::instance().isDark());
}

void AboutDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(10);

    // 1. Cabeçalho Principal (Ícone + Título + Versão + Autores)
    auto* topLayout = new QHBoxLayout();
    auto* iconLabel = new QLabel(this);
    const QIcon icon = Utils::ApplicationPathUtils::getAppIcon();
    if (!icon.isNull()) {
        iconLabel->setPixmap(icon.pixmap(64, 64));
    }
    topLayout->addWidget(iconLabel);

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    m_titleLabel = new QLabel(QStringLiteral("ACL Auditor"), this);
    QFont fTitle = m_titleLabel->font();
    fTitle.setPointSize(16);
    fTitle.setBold(true);
    m_titleLabel->setFont(fTitle);
    titleLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setStyleSheet(QStringLiteral("font-weight: 500; font-size: 10pt;"));
    titleLayout->addWidget(m_subtitleLabel);

    m_versionLabel = new QLabel(this);
    m_versionLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text); font-size: 9pt;"));
    titleLayout->addWidget(m_versionLabel);

    topLayout->addLayout(titleLayout);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // 2. Barra de Busca Interna nos Documentos
    auto* searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setClearButtonEnabled(true);
    searchLayout->addWidget(m_searchEdit, 1);

    m_searchPrevBtn = new QToolButton(this);
    m_searchPrevBtn->setText(QStringLiteral("▲"));
    m_searchPrevBtn->setToolTip(Language::GerenciadorTraducao::trText("Anterior"));
    searchLayout->addWidget(m_searchPrevBtn);

    m_searchNextBtn = new QToolButton(this);
    m_searchNextBtn->setText(QStringLiteral("▼"));
    m_searchNextBtn->setToolTip(Language::GerenciadorTraducao::trText("Próximo"));
    searchLayout->addWidget(m_searchNextBtn);

    m_searchCountLabel = new QLabel(this);
    m_searchCountLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text); min-width: 80px;"));
    searchLayout->addWidget(m_searchCountLabel);

    mainLayout->addLayout(searchLayout);

    // 3. TabWidget com os documentos dos assets (uma aba dedicada para cada grupo documental)
    m_tabWidget = new QTabWidget(this);

    m_browserAbout = new QTextBrowser(this);
    configurarBrowser(m_browserAbout);
    m_tabWidget->addTab(m_browserAbout, Language::GerenciadorTraducao::trText("Sobre"));

    m_browserHistory = new QTextBrowser(this);
    configurarBrowser(m_browserHistory);
    m_tabWidget->addTab(m_browserHistory, Language::GerenciadorTraducao::trText("Histórico"));

    m_browserEula = new QTextBrowser(this);
    configurarBrowser(m_browserEula);
    m_tabWidget->addTab(m_browserEula, Language::GerenciadorTraducao::trText("EULA"));

    m_browserClc = new QTextBrowser(this);
    configurarBrowser(m_browserClc);
    m_tabWidget->addTab(m_browserClc, Language::GerenciadorTraducao::trText("Contrato Comercial"));

    m_browserCopyright = new QTextBrowser(this);
    configurarBrowser(m_browserCopyright);
    m_tabWidget->addTab(m_browserCopyright, Language::GerenciadorTraducao::trText("Copyright e Marcas"));

    m_browserNotices = new QTextBrowser(this);
    configurarBrowser(m_browserNotices);
    m_tabWidget->addTab(m_browserNotices, Language::GerenciadorTraducao::trText("Avisos"));

    m_browserPrivacy = new QTextBrowser(this);
    configurarBrowser(m_browserPrivacy);
    m_tabWidget->addTab(m_browserPrivacy, Language::GerenciadorTraducao::trText("Política de Privacidade"));

    m_browserRelease = new QTextBrowser(this);
    configurarBrowser(m_browserRelease);
    m_tabWidget->addTab(m_browserRelease, Language::GerenciadorTraducao::trText("Notas de Versão"));

    mainLayout->addWidget(m_tabWidget, 1);

    // 4. Rodapé
    auto* bottomLayout = new QHBoxLayout();
    auto* siteLabel = new QLabel(QStringLiteral("<a href=\"https://github.com/fernandoncidade/ACL_Auditor_cpp\">Site Oficial & Documentação</a>"), this);
    siteLabel->setOpenExternalLinks(true);
    bottomLayout->addWidget(siteLabel);
    bottomLayout->addStretch();

    m_closeBtn = new QPushButton(Language::GerenciadorTraducao::trText("Fechar"), this);
    m_closeBtn->setDefault(true);
    m_closeBtn->setMinimumWidth(100);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(bottomLayout);

    // Conexões de busca
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AboutDialog::executarBusca);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() { navegarBusca(1); });
    connect(m_searchPrevBtn, &QToolButton::clicked, this, [this]() { navegarBusca(-1); });
    connect(m_searchNextBtn, &QToolButton::clicked, this, [this]() { navegarBusca(1); });
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &AboutDialog::executarBusca);
}

void AboutDialog::configurarBrowser(QTextBrowser* browser) {
    if (!browser) return;
    browser->setReadOnly(true);
    browser->setOpenExternalLinks(true);
    browser->setAcceptRichText(true);
}

void AboutDialog::carregarTextos(const QString& idioma) {
    const bool isEn = idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    // 1. ABOUT & HISTORY
    m_textAbout = loadAssetFile(QStringLiteral("ABOUT"), isEn ?
        QStringList{QStringLiteral("ABOUT_en_US.txt"), QStringLiteral("ABOUT.txt")} :
        QStringList{QStringLiteral("ABOUT_pt_BR.txt"), QStringLiteral("ABOUT.txt")});

    m_textHistory = loadAssetFile(QStringLiteral("ABOUT"), isEn ?
        QStringList{QStringLiteral("History_APP_en_US.txt"), QStringLiteral("History_APP.txt")} :
        QStringList{QStringLiteral("History_APP_pt_BR.txt"), QStringLiteral("History_APP.txt")});

    // 2. EULA (Licença de Uso de Usuário Final — texto puro e isolado)
    m_textEula = loadAssetFile(QStringLiteral("EULA"), isEn ?
        QStringList{QStringLiteral("EULA_en_US.txt")} :
        QStringList{QStringLiteral("EULA_pt_BR.txt")});

    // 3. CLC (Contrato de Licenciamento Comercial — aba exclusiva)
    m_textClc = loadAssetFile(QStringLiteral("CLC"), isEn ?
        QStringList{QStringLiteral("CLC_en_US.txt")} :
        QStringList{QStringLiteral("CLC_pt_BR.txt")});

    // 4. COPYRIGHT & TRADEMARKS (Aviso de Copyright e Marca Registrada — aba exclusiva)
    m_textCopyright = loadAssetFile(QStringLiteral("COPYRIGHT"), isEn ?
        QStringList{QStringLiteral("COPYRIGHT AND TRADEMARK NOTICE_en_US.txt"), QStringLiteral("COPYRIGHT_en_US.txt")} :
        QStringList{QStringLiteral("AVISO DE COPYRIGHT E MARCA REGISTRADA_pt_BR.txt"), QStringLiteral("COPYRIGHT_pt_BR.txt")});

    // 5. NOTICES (Avisos de Dependências de Terceiros e Licenças)
    m_textNotices = loadAssetFile(QStringLiteral("NOTICES"), isEn ?
        QStringList{QStringLiteral("NOTICE_en_US.txt"), QStringLiteral("NOTICE.txt")} :
        QStringList{QStringLiteral("NOTICE_pt_BR.txt"), QStringLiteral("NOTICE.txt")});

    // 6. PRIVACY POLICY (Política de Privacidade)
    m_textPrivacy = loadAssetFile(QStringLiteral("PRIVACY_POLICY"), isEn ?
        QStringList{QStringLiteral("Privacy_Policy_en_US.txt"), QStringLiteral("Privacy_Policy.txt")} :
        QStringList{QStringLiteral("Privacy_Policy_pt_BR.txt"), QStringLiteral("Privacy_Policy.txt")});

    // 7. RELEASE NOTES (Notas de Versão)
    m_textRelease = loadAssetFile(QStringLiteral("RELEASE"), isEn ?
        QStringList{QStringLiteral("RELEASE NOTES_en_US.txt"), QStringLiteral("RELEASE_NOTES.txt")} :
        QStringList{QStringLiteral("RELEASE NOTES_pt_BR.txt"), QStringLiteral("RELEASE_NOTES.txt")});

    // Renderizar isoladamente para os browsers de cada aba
    aplicarTextoBrowser(m_browserAbout, m_textAbout);
    aplicarTextoBrowser(m_browserHistory, m_textHistory);
    aplicarTextoBrowser(m_browserEula, m_textEula);
    aplicarTextoBrowser(m_browserClc, m_textClc);
    aplicarTextoBrowser(m_browserCopyright, m_textCopyright);
    aplicarTextoBrowser(m_browserNotices, m_textNotices);
    aplicarTextoBrowser(m_browserPrivacy, m_textPrivacy);
    aplicarTextoBrowser(m_browserRelease, m_textRelease);
}

QString AboutDialog::renderizarDocumentoHtml(const QString& plainText, const QPalette& pal) {
    const QString bgColor = pal.color(QPalette::Base).name();
    const QString textColor = pal.color(QPalette::Text).name();
    const QString linkColor = pal.color(QPalette::Link).name();

    return QStringLiteral(
        "<html><head><style>"
        "body { background-color: %1; color: %2; margin: 10px; font-family: 'Segoe UI', Tahoma, sans-serif; }"
        "a { color: %3; text-decoration: underline; }"
        "</style></head><body>%4</body></html>"
    ).arg(bgColor, textColor, linkColor, renderClickableBody(plainText));
}

void AboutDialog::aplicarTextoBrowser(QTextBrowser* browser, const QString& texto) {
    if (!browser) return;
    const bool isDark = ThemeManager::instance().isDark();
    browser->setPalette(palette());
    if (browser->viewport()) {
        browser->viewport()->setPalette(palette());
    }
    browser->setStyleSheet(isDark ?
        QStringLiteral("QTextBrowser { background-color: #18191b; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; }") :
        QStringLiteral("QTextBrowser { background-color: #ffffff; color: #212529; border: 1px solid #ced4da; border-radius: 4px; }"));
    if (browser->document()) {
        browser->document()->clear();
    }
    browser->setHtml(renderizarDocumentoHtml(texto, palette()));
}

void AboutDialog::retranslateUi(const QString& idioma) {
    const QString lang = idioma.isEmpty() ? Language::GerenciadorTraducao::instance().obter_idioma_atual() : idioma;
    const bool isEn = lang.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    setWindowTitle(Language::GerenciadorTraducao::trText("Sobre o ACL Auditor"));
    m_subtitleLabel->setText(Language::GerenciadorTraducao::trText("ACL Auditor — Analisador Visual de Permissões e Conformidade NTFS"));
    m_versionLabel->setText(isEn ?
        QStringLiteral("Version 2026.11.6.0 (Win32 x64 / Qt 6.11.1) • Fernando Nillsson Cidade") :
        QStringLiteral("Versão 2026.11.6.0 (Win32 x64 / Qt 6.11.1) • Fernando Nillsson Cidade"));

    m_searchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Buscar nos documentos..."));
    m_searchPrevBtn->setToolTip(Language::GerenciadorTraducao::trText("Anterior"));
    m_searchNextBtn->setToolTip(Language::GerenciadorTraducao::trText("Próximo"));
    m_closeBtn->setText(Language::GerenciadorTraducao::trText("Fechar"));

    m_tabWidget->setTabText(0, Language::GerenciadorTraducao::trText("Sobre"));
    m_tabWidget->setTabText(1, Language::GerenciadorTraducao::trText("Histórico"));
    m_tabWidget->setTabText(2, Language::GerenciadorTraducao::trText("EULA"));
    m_tabWidget->setTabText(3, Language::GerenciadorTraducao::trText("Contrato Comercial"));
    m_tabWidget->setTabText(4, Language::GerenciadorTraducao::trText("Copyright e Marcas"));
    m_tabWidget->setTabText(5, Language::GerenciadorTraducao::trText("Avisos"));
    m_tabWidget->setTabText(6, Language::GerenciadorTraducao::trText("Política de Privacidade"));
    m_tabWidget->setTabText(7, Language::GerenciadorTraducao::trText("Notas de Versão"));
}

void AboutDialog::onIdiomaAlterado(const QString& idioma) {
    carregarTextos(idioma);
    retranslateUi(idioma);
}

void AboutDialog::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        carregarTextos(Language::GerenciadorTraducao::instance().obter_idioma_atual());
    }
    QDialog::changeEvent(event);
}

void AboutDialog::applyTheme(bool isDark) {
    Win32DwmHelper::applyTitleBarTheme(this, isDark);
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();
    setPalette(pal);

    if (m_titleLabel) {
        m_titleLabel->setStyleSheet(isDark ? QStringLiteral("color: #38bdf8; font-weight: bold; font-size: 16pt;")
                                           : QStringLiteral("color: #0284c7; font-weight: bold; font-size: 16pt;"));
    }
    if (m_subtitleLabel) {
        m_subtitleLabel->setStyleSheet(isDark ? QStringLiteral("color: #cbd5e1; font-weight: 500; font-size: 10pt;")
                                              : QStringLiteral("color: #334155; font-weight: 500; font-size: 10pt;"));
    }
    if (m_versionLabel) {
        m_versionLabel->setStyleSheet(isDark ? QStringLiteral("color: #94a3b8; font-size: 9pt;")
                                             : QStringLiteral("color: #64748b; font-size: 9pt;"));
    }
    if (m_searchEdit) {
        m_searchEdit->setStyleSheet(isDark ?
            QStringLiteral("QLineEdit { background-color: #18191b; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px 8px; }") :
            QStringLiteral("QLineEdit { background-color: #ffffff; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px 8px; }"));
    }
    if (m_searchCountLabel) {
        m_searchCountLabel->setStyleSheet(isDark ? QStringLiteral("color: #94a3b8;") : QStringLiteral("color: #64748b;"));
    }
    if (m_closeBtn) {
        m_closeBtn->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #2d3034; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 6px 18px; font-weight: bold; } QPushButton:hover { background-color: #383c42; }") :
            QStringLiteral("QPushButton { background-color: #e9ecef; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 6px 18px; font-weight: bold; } QPushButton:hover { background-color: #dee2e6; }"));
    }

    aplicarTextoBrowser(m_browserAbout, m_textAbout);
    aplicarTextoBrowser(m_browserHistory, m_textHistory);
    aplicarTextoBrowser(m_browserEula, m_textEula);
    aplicarTextoBrowser(m_browserClc, m_textClc);
    aplicarTextoBrowser(m_browserCopyright, m_textCopyright);
    aplicarTextoBrowser(m_browserNotices, m_textNotices);
    aplicarTextoBrowser(m_browserPrivacy, m_textPrivacy);
    aplicarTextoBrowser(m_browserRelease, m_textRelease);

    executarBusca();
}

void AboutDialog::resetarBusca() {
    m_searchMatches.clear();
    m_currentMatchIndex = -1;
    m_searchCountLabel->setText(QString());

    auto limparSelecoes = [](QTextBrowser* browser) {
        if (browser) browser->setExtraSelections({});
    };
    limparSelecoes(m_browserAbout);
    limparSelecoes(m_browserHistory);
    limparSelecoes(m_browserEula);
    limparSelecoes(m_browserClc);
    limparSelecoes(m_browserCopyright);
    limparSelecoes(m_browserNotices);
    limparSelecoes(m_browserPrivacy);
    limparSelecoes(m_browserRelease);
}

void AboutDialog::executarBusca() {
    resetarBusca();

    const QString query = m_searchEdit->text().trimmed();
    if (query.isEmpty()) return;

    QWidget* currentWidget = m_tabWidget->currentWidget();
    auto* currentBrowser = qobject_cast<QTextBrowser*>(currentWidget);
    if (!currentBrowser || !currentBrowser->document()) return;

    QList<QTextEdit::ExtraSelection> selections;
    QTextCursor cursor(currentBrowser->document());

    const QColor hlColor = palette().color(QPalette::Highlight);
    const QColor hlText = palette().color(QPalette::HighlightedText);

    while (true) {
        cursor = currentBrowser->document()->find(query, cursor);
        if (cursor.isNull()) break;

        SearchMatch match;
        match.browser = currentBrowser;
        match.start = cursor.selectionStart();
        match.length = cursor.selectionEnd() - cursor.selectionStart();
        m_searchMatches.append(match);

        QTextEdit::ExtraSelection sel;
        sel.cursor = cursor;
        QColor bg = hlColor;
        bg.setAlpha(120);
        sel.format.setBackground(bg);
        sel.format.setForeground(hlText);
        selections.append(sel);
    }

    currentBrowser->setExtraSelections(selections);

    if (!m_searchMatches.isEmpty()) {
        m_currentMatchIndex = 0;
        m_searchCountLabel->setText(QStringLiteral("1 / %1").arg(m_searchMatches.size()));
        navegarBusca(0);
    } else {
        m_searchCountLabel->setText(QStringLiteral("0 / 0"));
    }
}

void AboutDialog::navegarBusca(int direcao) {
    if (m_searchMatches.isEmpty()) return;

    m_currentMatchIndex += direcao;
    if (m_currentMatchIndex >= m_searchMatches.size()) {
        m_currentMatchIndex = 0;
    } else if (m_currentMatchIndex < 0) {
        m_currentMatchIndex = m_searchMatches.size() - 1;
    }

    const auto& match = m_searchMatches.at(m_currentMatchIndex);
    if (match.browser && match.browser->document()) {
        QTextCursor c(match.browser->document());
        c.setPosition(match.start);
        c.setPosition(match.start + match.length, QTextCursor::KeepAnchor);
        match.browser->setTextCursor(c);
        match.browser->ensureCursorVisible();
        m_searchCountLabel->setText(QStringLiteral("%1 / %2").arg(m_currentMatchIndex + 1).arg(m_searchMatches.size()));
    }
}

} // namespace AclAuditor::UI
