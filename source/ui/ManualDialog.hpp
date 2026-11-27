#pragma once

#include <QDialog>
#include <QList>
#include <QString>
#include <QTextCursor>
#include <QVector>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTextBrowser;
class QToolButton;

namespace AclAuditor::UI {

struct ManualSearchState {
    QString query;
    QVector<QTextCursor> matches;
    int current_index = -1;
};

/**
 * @brief Diálogo corporativo para exibição do Manual de Instruções e Guia de Conformidade NTFS.
 * Interface moderna padronizada idêntica ao padrão de excelência de interface,
 * com índice lateral (QListWidget), barra de busca com navegação bidirecional e realce por seleção extra,
 * sincronização recíproca entre índice e texto e suporte dinâmico a temas e idiomas.
 */
class ManualDialog : public QDialog {
    Q_OBJECT

public:
    explicit ManualDialog(QWidget* parent = nullptr);
    ~ManualDialog() override = default;

public slots:
    void applyTheme(bool isDark);
    void onIdiomaAlterado(const QString& idioma);

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void retranslateUi();
    void updateManualHtml();
    QString buildManualHtmlPt(bool isDark) const;
    QString buildManualHtmlEn(bool isDark) const;

    // Métodos de busca e navegação
    void rebuildSearch(const QString& text);
    void navigateSearch(int step);
    void updateSearchHighlights();
    void updateSearchButtons();
    void scrollToAnchor(const QString& anchorName);
    int findRowForAnchor(const QString& anchorName) const;

    // Componentes de busca
    QLabel* m_searchLabel{nullptr};
    QLineEdit* m_searchEdit{nullptr};
    QToolButton* m_searchPrevBtn{nullptr};
    QToolButton* m_searchNextBtn{nullptr};

    // Corpo
    QListWidget* m_list{nullptr};
    QTextBrowser* m_browser{nullptr};

    // Rodapé
    QPushButton* m_btnClose{nullptr};

    ManualSearchState m_searchState;
    QString m_activeLanguage{QStringLiteral("pt_BR")};
};

} // namespace AclAuditor::UI
