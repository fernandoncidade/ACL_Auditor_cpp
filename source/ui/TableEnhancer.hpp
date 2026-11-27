#pragma once

#include <QObject>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QVector>

class QAbstractItemView;
class QTableView;
class QTableWidget;
class QHeaderView;
class QPoint;

namespace AclAuditor::UI {

/**
 * @brief Proxy model avançado para tabelas de auditoria.
 * Permite filtragem de texto através de múltiplas colunas simultaneamente
 * e ordenação inteligente (priorizando Qt::UserRole quando presente).
 */
class MultiColumnSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit MultiColumnSortFilterProxyModel(QObject* parent = nullptr);
    ~MultiColumnSortFilterProxyModel() override = default;

    void setFilterPattern(const QString& text);
    QString filterPattern() const { return m_filterPattern; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QString m_filterPattern;
};

/**
 * @brief Gerenciador e aprimorador de tabelas (QTableView e QTableWidget) no padrão de excelência corporativo.
 * Oferece:
 * - Redimensionamento interativo e movimentação de colunas por arraste
 * - Menus de contexto de células (Copiar Célula, Copiar Linhas, Copiar Tabela, Exportar CSV, etc.)
 * - Menus de contexto de cabeçalho (Ocultar/Reexibir colunas, Ajustar ao Conteúdo, Restaurar Padrão)
 * - Atalhos de teclado (Ctrl+C para copiar TSV compatível com Excel, Ctrl+Shift+C para cabeçalhos)
 * - Exportação de dados para CSV compatível diretamente com Excel (UTF-8 com BOM e delimitador ;)
 * - Ajuste automático e restauração de larguras padrão
 */
class TableEnhancer : public QObject {
    Q_OBJECT

public:
    explicit TableEnhancer(
        QAbstractItemView* view,
        const QString& exportBaseName,
        const QVector<int>& defaultWidths = {},
        QObject* parent = nullptr
    );
    ~TableEnhancer() override = default;

    static TableEnhancer* enhanceTableView(
        QTableView* tableView,
        MultiColumnSortFilterProxyModel* proxyModel,
        const QString& exportBaseName,
        const QVector<int>& defaultWidths = {}
    );

    static TableEnhancer* enhanceTableWidget(
        QTableWidget* tableWidget,
        const QString& exportBaseName = QStringLiteral("tabela_dados"),
        const QVector<int>& defaultWidths = {}
    );

    static void copySelectionToClipboard(QAbstractItemView* view, bool includeHeaders = false);
    static void copyAllToClipboard(QAbstractItemView* view, bool includeHeaders = true);
    static void exportToCsv(QAbstractItemView* view, const QString& defaultFileName, QWidget* parent = nullptr);
    static void autoFitColumns(QAbstractItemView* view);
    static void resetDefaultWidths(QAbstractItemView* view, const QVector<int>& defaultWidths);

    void applyDefaultWidths();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onHeaderContextMenuRequested(const QPoint& pos);

private:
    QPointer<QAbstractItemView> m_view;
    QString m_exportBaseName;
    QVector<int> m_defaultWidths;
};

} // namespace AclAuditor::UI
