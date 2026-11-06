#include "TableEnhancer.hpp"
#include "../language/GerenciadorTraducao.hpp"

#include <QAbstractItemView>
#include <QTableView>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QSet>
#include <QMap>
#include <QStringConverter>

namespace AclAuditor::UI {

// ============================================================================
// MultiColumnSortFilterProxyModel
// ============================================================================

MultiColumnSortFilterProxyModel::MultiColumnSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

void MultiColumnSortFilterProxyModel::setFilterPattern(const QString& text) {
    m_filterPattern = text.trimmed();
    invalidate();
}

bool MultiColumnSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    if (m_filterPattern.isEmpty()) return true;
    if (!sourceModel()) return true;

    const int cols = sourceModel()->columnCount(sourceParent);
    for (int c = 0; c < cols; ++c) {
        const QModelIndex idx = sourceModel()->index(sourceRow, c, sourceParent);
        const QString data = sourceModel()->data(idx, Qt::DisplayRole).toString();
        if (data.contains(m_filterPattern, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool MultiColumnSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    if (!sourceModel()) return false;
    const QVariant leftVal = sourceModel()->data(left, Qt::UserRole);
    const QVariant rightVal = sourceModel()->data(right, Qt::UserRole);
    if (leftVal.isValid() && rightVal.isValid()) {
        auto ord = QVariant::compare(leftVal, rightVal);
        if (ord == QPartialOrdering::Less) return true;
        if (ord == QPartialOrdering::Greater) return false;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

// ============================================================================
// TableEnhancer
// ============================================================================

TableEnhancer::TableEnhancer(
    QAbstractItemView* view,
    const QString& exportBaseName,
    const QVector<int>& defaultWidths,
    QObject* parent
) : QObject(parent), m_view(view), m_exportBaseName(exportBaseName), m_defaultWidths(defaultWidths) {
    if (m_view) {
        m_view->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_view, &QWidget::customContextMenuRequested, this, &TableEnhancer::onCustomContextMenuRequested);

        m_view->installEventFilter(this);
        if (m_view->viewport()) {
            m_view->viewport()->installEventFilter(this);
        }

        QHeaderView* hHeader = nullptr;
        if (auto* tv = qobject_cast<QTableView*>(m_view)) {
            hHeader = tv->horizontalHeader();
        } else if (auto* tw = qobject_cast<QTableWidget*>(m_view)) {
            hHeader = tw->horizontalHeader();
        }

        if (hHeader) {
            hHeader->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(hHeader, &QWidget::customContextMenuRequested, this, &TableEnhancer::onHeaderContextMenuRequested);
        }
    }
}

TableEnhancer* TableEnhancer::enhanceTableView(
    QTableView* tableView,
    MultiColumnSortFilterProxyModel* proxyModel,
    const QString& exportBaseName,
    const QVector<int>& defaultWidths
) {
    if (!tableView) return nullptr;

    if (proxyModel) {
        tableView->setModel(proxyModel);
    }

    tableView->setSortingEnabled(true);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QHeaderView* hHeader = tableView->horizontalHeader();
    if (hHeader) {
        hHeader->setSectionsMovable(true);
        hHeader->setHighlightSections(true);
        hHeader->setSectionResizeMode(QHeaderView::Interactive);
    }

    auto* enhancer = new TableEnhancer(tableView, exportBaseName, defaultWidths, tableView);
    enhancer->applyDefaultWidths();
    return enhancer;
}

TableEnhancer* TableEnhancer::enhanceTableWidget(
    QTableWidget* tableWidget,
    const QString& exportBaseName,
    const QVector<int>& defaultWidths
) {
    if (!tableWidget) return nullptr;

    tableWidget->setSortingEnabled(true);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QHeaderView* hHeader = tableWidget->horizontalHeader();
    if (hHeader) {
        hHeader->setSectionsMovable(true);
        hHeader->setHighlightSections(true);
        hHeader->setSectionResizeMode(QHeaderView::Interactive);
    }

    auto* enhancer = new TableEnhancer(tableWidget, exportBaseName, defaultWidths, tableWidget);
    enhancer->applyDefaultWidths();
    return enhancer;
}

void TableEnhancer::applyDefaultWidths() {
    resetDefaultWidths(m_view, m_defaultWidths);
}

bool TableEnhancer::eventFilter(QObject* watched, QEvent* event) {
    if (m_view && (watched == m_view || watched == m_view->viewport())) {
        if (event->type() == QEvent::KeyPress) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->matches(QKeySequence::Copy) || (keyEvent->key() == Qt::Key_C && (keyEvent->modifiers() & Qt::ControlModifier))) {
                const bool withHeaders = (keyEvent->modifiers() & Qt::ShiftModifier);
                copySelectionToClipboard(m_view, withHeaders);
                return true;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void TableEnhancer::copySelectionToClipboard(QAbstractItemView* view, bool includeHeaders) {
    if (!view || !view->model()) return;

    QItemSelectionModel* selModel = view->selectionModel();
    if (!selModel || !selModel->hasSelection()) return;

    const QModelIndexList selected = selModel->selectedIndexes();
    if (selected.isEmpty()) return;

    auto* tv = qobject_cast<QTableView*>(view);
    QMap<int, QMap<int, QString>> grid;
    QSet<int> selectedCols;

    for (const QModelIndex& idx : selected) {
        if (!idx.isValid()) continue;
        if (tv && (tv->isRowHidden(idx.row()) || tv->isColumnHidden(idx.column()))) continue;
        grid[idx.row()][idx.column()] = idx.data(Qt::DisplayRole).toString();
        selectedCols.insert(idx.column());
    }

    if (grid.isEmpty()) return;

    // Se apenas 1 celula foi selecionada, copiar valor puro direto
    if (grid.size() == 1 && grid.begin().value().size() == 1 && !includeHeaders) {
        QGuiApplication::clipboard()->setText(grid.begin().value().begin().value());
        return;
    }

    QList<int> sortedCols = selectedCols.values();
    std::sort(sortedCols.begin(), sortedCols.end());

    QString text;
    QTextStream out(&text);

    if (includeHeaders) {
        QStringList hList;
        for (int c : sortedCols) {
            hList.append(view->model()->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString());
        }
        out << hList.join(QLatin1Char('\t')) << QStringLiteral("\r\n");
    }

    for (auto rowIt = grid.constBegin(); rowIt != grid.constEnd(); ++rowIt) {
        QStringList rowCells;
        for (int c : sortedCols) {
            rowCells.append(rowIt.value().value(c, QString()));
        }
        out << rowCells.join(QLatin1Char('\t')) << QStringLiteral("\r\n");
    }

    QGuiApplication::clipboard()->setText(text);
}

void TableEnhancer::copyAllToClipboard(QAbstractItemView* view, bool includeHeaders) {
    if (!view || !view->model()) return;
    QAbstractItemModel* model = view->model();
    const int cols = model->columnCount();
    const int rows = model->rowCount();
    auto* tv = qobject_cast<QTableView*>(view);

    QString text;
    QTextStream out(&text);

    if (includeHeaders) {
        QStringList hList;
        for (int c = 0; c < cols; ++c) {
            if (!tv || !tv->isColumnHidden(c)) {
                hList.append(model->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString());
            }
        }
        out << hList.join(QLatin1Char('\t')) << QStringLiteral("\r\n");
    }

    for (int r = 0; r < rows; ++r) {
        if (tv && tv->isRowHidden(r)) continue;
        QStringList rowCells;
        for (int c = 0; c < cols; ++c) {
            if (!tv || !tv->isColumnHidden(c)) {
                QModelIndex idx = model->index(r, c);
                rowCells.append(idx.data(Qt::DisplayRole).toString());
            }
        }
        out << rowCells.join(QLatin1Char('\t')) << QStringLiteral("\r\n");
    }

    QGuiApplication::clipboard()->setText(text);
}

void TableEnhancer::autoFitColumns(QAbstractItemView* view) {
    if (!view || !view->model()) return;
    if (auto* tv = qobject_cast<QTableView*>(view)) {
        tv->resizeColumnsToContents();
        for (int c = 0; c < tv->model()->columnCount(); ++c) {
            if (tv->columnWidth(c) < 60) {
                tv->setColumnWidth(c, 60);
            }
        }
    }
}

void TableEnhancer::resetDefaultWidths(QAbstractItemView* view, const QVector<int>& defaultWidths) {
    if (!view || defaultWidths.isEmpty()) return;
    if (auto* tv = qobject_cast<QTableView*>(view)) {
        for (int c = 0; c < defaultWidths.size() && c < tv->model()->columnCount(); ++c) {
            if (defaultWidths[c] > 0) {
                tv->setColumnWidth(c, defaultWidths[c]);
                tv->setColumnHidden(c, false);
            }
        }
    }
}

void TableEnhancer::exportToCsv(QAbstractItemView* view, const QString& defaultFileName, QWidget* parent) {
    if (!view || !view->model()) return;
    QWidget* parentWidget = parent ? parent : view;

    QString defaultPath = QDir::homePath();
    QString initialFilePath = defaultPath + QStringLiteral("/") + defaultFileName;

    QString filePath = QFileDialog::getSaveFileName(
        parentWidget,
        Language::GerenciadorTraducao::trText("Exportar Tabela para CSV"),
        initialFilePath,
        Language::GerenciadorTraducao::trText("Arquivos CSV (*.csv);;Todos os Arquivos (*.*)")
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(
            parentWidget,
            Language::GerenciadorTraducao::trText("Erro"),
            Language::GerenciadorTraducao::trText("Erro ao salvar arquivo CSV:\n%1").arg(file.errorString())
        );
        return;
    }

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#endif
    out.setGenerateByteOrderMark(true);

    QAbstractItemModel* model = view->model();
    const int cols = model->columnCount();
    const int rows = model->rowCount();

    auto escapeCsv = [](const QString& field) -> QString {
        QString res = field;
        if (res.contains(QLatin1Char('"')) || res.contains(QLatin1Char(';')) || res.contains(QLatin1Char('\n')) || res.contains(QLatin1Char('\r'))) {
            res.replace(QLatin1Char('"'), QStringLiteral("\"\""));
            return QStringLiteral("\"%1\"").arg(res);
        }
        return res;
    };

    auto* tv = qobject_cast<QTableView*>(view);

    // 1. Cabeçalho
    QStringList headerRow;
    for (int c = 0; c < cols; ++c) {
        if (!tv || !tv->isColumnHidden(c)) {
            headerRow.append(escapeCsv(model->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString()));
        }
    }
    out << headerRow.join(QStringLiteral(";")) << QStringLiteral("\r\n");

    // 2. Linhas
    int exportedCount = 0;
    for (int r = 0; r < rows; ++r) {
        if (tv && tv->isRowHidden(r)) continue;
        QStringList lineCells;
        for (int c = 0; c < cols; ++c) {
            if (!tv || !tv->isColumnHidden(c)) {
                QModelIndex idx = model->index(r, c);
                lineCells.append(escapeCsv(idx.data(Qt::DisplayRole).toString()));
            }
        }
        out << lineCells.join(QStringLiteral(";")) << QStringLiteral("\r\n");
        exportedCount++;
    }

    file.close();

    QMessageBox::information(
        parentWidget,
        Language::GerenciadorTraducao::trText("Sucesso"),
        Language::GerenciadorTraducao::trText("Tabela exportada com sucesso!\n%1 registros salvos em:\n%2")
            .arg(exportedCount)
            .arg(QDir::toNativeSeparators(filePath))
    );
}

void TableEnhancer::onCustomContextMenuRequested(const QPoint& pos) {
    if (!m_view || !m_view->model()) return;

    QMenu menu(m_view);

    QItemSelectionModel* selModel = m_view->selectionModel();
    const bool hasSelection = (selModel && selModel->hasSelection());

    if (hasSelection) {
        const QModelIndexList selectedIndexes = selModel->selectedIndexes();
        if (selectedIndexes.size() == 1) {
            auto* actCopyCell = menu.addAction(Language::GerenciadorTraducao::trText("Copiar Célula"));
            connect(actCopyCell, &QAction::triggered, this, [this]() {
                copySelectionToClipboard(m_view, false);
            });
        }

        auto* actCopyRows = menu.addAction(Language::GerenciadorTraducao::trText("Copiar Linhas Selecionadas"));
        actCopyRows->setShortcut(QKeySequence::Copy);
        connect(actCopyRows, &QAction::triggered, this, [this]() {
            copySelectionToClipboard(m_view, false);
        });

        auto* actCopyHeaders = menu.addAction(Language::GerenciadorTraducao::trText("Copiar com Cabeçalhos"));
        actCopyHeaders->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+C")));
        connect(actCopyHeaders, &QAction::triggered, this, [this]() {
            copySelectionToClipboard(m_view, true);
        });

        menu.addSeparator();
    }

    auto* actCopyAll = menu.addAction(Language::GerenciadorTraducao::trText("Copiar Tabela Inteira"));
    connect(actCopyAll, &QAction::triggered, this, [this]() {
        copyAllToClipboard(m_view, true);
    });

    auto* actExportCsv = menu.addAction(Language::GerenciadorTraducao::trText("Exportar Dados para CSV..."));
    connect(actExportCsv, &QAction::triggered, this, [this]() {
        exportToCsv(m_view, m_exportBaseName + QStringLiteral(".csv"), m_view);
    });

    menu.addSeparator();

    auto* actFitAll = menu.addAction(Language::GerenciadorTraducao::trText("Ajustar Todas as Colunas ao Conteúdo"));
    connect(actFitAll, &QAction::triggered, this, [this]() {
        autoFitColumns(m_view);
    });

    if (!m_defaultWidths.isEmpty()) {
        auto* actResetWidths = menu.addAction(Language::GerenciadorTraducao::trText("Restaurar Larguras Padrão"));
        connect(actResetWidths, &QAction::triggered, this, [this]() {
            resetDefaultWidths(m_view, m_defaultWidths);
        });
    }

    menu.exec(m_view->viewport()->mapToGlobal(pos));
}

void TableEnhancer::onHeaderContextMenuRequested(const QPoint& pos) {
    if (!m_view) return;
    QHeaderView* header = nullptr;
    if (auto* tv = qobject_cast<QTableView*>(m_view)) {
        header = tv->horizontalHeader();
    } else if (auto* tw = qobject_cast<QTableWidget*>(m_view)) {
        header = tw->horizontalHeader();
    }
    if (!header) return;

    int logicalIndex = header->logicalIndexAt(pos);

    QMenu menu(header);
    if (logicalIndex >= 0 && m_view->model()) {
        const QString colName = m_view->model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
        auto* actHide = menu.addAction(Language::GerenciadorTraducao::trText("Ocultar Coluna '%1'").arg(colName));
        connect(actHide, &QAction::triggered, [header, logicalIndex]() {
            header->setSectionHidden(logicalIndex, true);
        });

        auto* actFitCol = menu.addAction(Language::GerenciadorTraducao::trText("Ajustar Coluna ao Conteúdo"));
        connect(actFitCol, &QAction::triggered, this, [this, logicalIndex]() {
            if (auto* tv = qobject_cast<QTableView*>(m_view)) {
                tv->resizeColumnToContents(logicalIndex);
                if (tv->columnWidth(logicalIndex) < 60) {
                    tv->setColumnWidth(logicalIndex, 60);
                }
            }
        });
        menu.addSeparator();
    }

    auto* actShowAll = menu.addAction(Language::GerenciadorTraducao::trText("Reexibir Todas as Colunas"));
    connect(actShowAll, &QAction::triggered, [header]() {
        for (int c = 0; c < header->count(); ++c) {
            header->setSectionHidden(c, false);
        }
    });

    menu.addSeparator();

    auto* actAutoFit = menu.addAction(Language::GerenciadorTraducao::trText("Ajustar Todas as Colunas ao Conteúdo"));
    connect(actAutoFit, &QAction::triggered, this, [this]() {
        autoFitColumns(m_view);
    });

    if (!m_defaultWidths.isEmpty()) {
        auto* actReset = menu.addAction(Language::GerenciadorTraducao::trText("Restaurar Larguras Padrão"));
        connect(actReset, &QAction::triggered, this, [this]() {
            resetDefaultWidths(m_view, m_defaultWidths);
        });
    }

    menu.exec(header->mapToGlobal(pos));
}

} // namespace AclAuditor::UI
