#pragma once

#include <QDialog>
#include <QHash>
#include <QPalette>
#include <QString>
#include <QVector>

class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QTextBrowser;
class QToolButton;

namespace AclAuditor::UI {

struct DocumentoTexto {
    QString titulo;
    QString conteudo;
};

/**
 * @brief Diálogo 'Sobre' completo e interativo do ACL Auditor.
 * Apresenta e permite buscar nos arquivos de RELEASE, ABOUT, EULA, LICENSES, NOTICES e PRIVACY_POLICY,
 * com suporte completo a tradução dinâmica e tema Windows 11.
 */
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() override = default;

    void retranslateUi(const QString& idioma = QString());

public slots:
    void applyTheme(bool isDark);
    void onIdiomaAlterado(const QString& idioma);

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void carregarTextos(const QString& idioma);
    void configurarBrowser(QTextBrowser* browser);
    void aplicarTextoBrowser(QTextBrowser* browser, const QString& texto);
    QString renderizarDocumentoHtml(const QString& plainText, const QPalette& pal);

    // Busca interna nos textos
    void executarBusca();
    void navegarBusca(int direcao);
    void resetarBusca();

    QLabel* m_titleLabel{nullptr};
    QLabel* m_subtitleLabel{nullptr};
    QLabel* m_versionLabel{nullptr};
    QTabWidget* m_tabWidget{nullptr};

    // Text browsers para cada aba (um para cada grupo/diretório documental)
    QTextBrowser* m_browserAbout{nullptr};
    QTextBrowser* m_browserHistory{nullptr};
    QTextBrowser* m_browserEula{nullptr};
    QTextBrowser* m_browserClc{nullptr};
    QTextBrowser* m_browserCopyright{nullptr};
    QTextBrowser* m_browserNotices{nullptr};
    QTextBrowser* m_browserPrivacy{nullptr};
    QTextBrowser* m_browserRelease{nullptr};

    // Barra de busca
    QLineEdit* m_searchEdit{nullptr};
    QToolButton* m_searchPrevBtn{nullptr};
    QToolButton* m_searchNextBtn{nullptr};
    QLabel* m_searchCountLabel{nullptr};
    QPushButton* m_closeBtn{nullptr};

    // Cache de textos carregados
    QString m_textAbout;
    QString m_textHistory;
    QString m_textEula;
    QString m_textClc;
    QString m_textCopyright;
    QString m_textNotices;
    QString m_textPrivacy;
    QString m_textRelease;

    struct SearchMatch {
        QTextBrowser* browser{nullptr};
        int start{-1};
        int length{0};
    };
    QVector<SearchMatch> m_searchMatches;
    int m_currentMatchIndex{-1};
};

} // namespace AclAuditor::UI
