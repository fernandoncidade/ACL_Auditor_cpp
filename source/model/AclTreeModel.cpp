#include "AclTreeModel.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include <QColor>
#include <QIcon>

namespace AclAuditor::Model {

AclTreeModel::AclTreeModel(QObject* parent) : QAbstractItemModel(parent) {
}

void AclTreeModel::clear() {
    beginResetModel();
    m_rootItem.reset();
    endResetModel();
}

void AclTreeModel::buildSubtree(AclTreeItem* parentItem, const Engine::FileSecurityNode& node) {
    for (const auto& childNode : node.children) {
        auto childItem = std::make_unique<AclTreeItem>(childNode, parentItem);
        buildSubtree(childItem.get(), childNode);
        parentItem->appendChild(std::move(childItem));
    }
}

void AclTreeModel::setRootNode(const Engine::FileSecurityNode& rootNode) {
    beginResetModel();
    Engine::FileSecurityNode dummyRoot;
    dummyRoot.fileName = L"Root";
    m_rootItem = std::make_unique<AclTreeItem>(dummyRoot);

    auto topItem = std::make_unique<AclTreeItem>(rootNode, m_rootItem.get());
    buildSubtree(topItem.get(), rootNode);
    m_rootItem->appendChild(std::move(topItem));
    endResetModel();
}

const Engine::FileSecurityNode* AclTreeModel::getNodeAt(const QModelIndex& index) const {
    if (!index.isValid()) return nullptr;
    auto* item = static_cast<AclTreeItem*>(index.internalPointer());
    return item ? &item->data : nullptr;
}

QModelIndex AclTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();

    AclTreeItem* parentItem = nullptr;
    if (!parent.isValid()) {
        parentItem = m_rootItem.get();
    } else {
        parentItem = static_cast<AclTreeItem*>(parent.internalPointer());
    }

    if (!parentItem) return QModelIndex();

    AclTreeItem* childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex AclTreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) return QModelIndex();

    auto* childItem = static_cast<AclTreeItem*>(index.internalPointer());
    if (!childItem) return QModelIndex();

    AclTreeItem* parentItem = childItem->parentItem;
    if (!parentItem || parentItem == m_rootItem.get()) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int AclTreeModel::rowCount(const QModelIndex& parent) const {
    if (!m_rootItem) return 0;

    AclTreeItem* parentItem = nullptr;
    if (parent.column() > 0) return 0;

    if (!parent.isValid()) {
        parentItem = m_rootItem.get();
    } else {
        parentItem = static_cast<AclTreeItem*>(parent.internalPointer());
    }

    return parentItem ? parentItem->childCount() : 0;
}

int AclTreeModel::columnCount(const QModelIndex& /*parent*/) const {
    return 6;
}

QVariant AclTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    auto* item = static_cast<AclTreeItem*>(index.internalPointer());
    if (!item) return QVariant();

    const auto& node = item->data;

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return QString::fromStdWString(node.fileName);
            case 1: return node.isDirectory ? Language::GerenciadorTraducao::trText("Pasta") : Language::GerenciadorTraducao::trText("Arquivo");
            case 2: return QString::fromStdWString(node.security.owner.getDisplayName());
            case 3: return node.security.isInheritanceBlocked ?
                Language::GerenciadorTraducao::trText("Bloqueada (Protegida)") :
                Language::GerenciadorTraducao::trText("Herdada");
            case 4: {
                if (node.riskScore >= 70) return Language::GerenciadorTraducao::trText("Crítico (%1)").arg(node.riskScore);
                if (node.riskScore >= 40) return Language::GerenciadorTraducao::trText("Alto (%1)").arg(node.riskScore);
                if (node.riskScore >= 20) return Language::GerenciadorTraducao::trText("Médio (%1)").arg(node.riskScore);
                if (node.riskScore > 0)   return Language::GerenciadorTraducao::trText("Baixo (%1)").arg(node.riskScore);
                return Language::GerenciadorTraducao::trText("Conforme (0)");
            }
            case 5: return QString::number(node.security.dacl.size());
            default: break;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 4) {
            if (node.riskScore >= 70) return QColor(220, 50, 50);
            if (node.riskScore >= 40) return QColor(230, 120, 20);
            if (node.riskScore >= 20) return QColor(210, 180, 20);
            return QColor(40, 160, 60);
        } else if (index.column() == 3 && node.security.isInheritanceBlocked) {
            return QColor(200, 100, 30);
        }
    } else if (role == Qt::ToolTipRole) {
        return QStringLiteral("%1: %2\n%3: %4\n%5: %6\n%7: %8\n%9: %10")
            .arg(Language::GerenciadorTraducao::trText("Caminho"), QString::fromStdWString(node.path))
            .arg(Language::GerenciadorTraducao::trText("Proprietário"), QString::fromStdWString(node.security.owner.getDisplayName()))
            .arg(Language::GerenciadorTraducao::trText("Herança"), node.security.isInheritanceBlocked ? Language::GerenciadorTraducao::trText("Bloqueada") : Language::GerenciadorTraducao::trText("Herdada"))
            .arg(Language::GerenciadorTraducao::trText("Pontuação de Risco"), QString::number(node.riskScore))
            .arg(Language::GerenciadorTraducao::trText("Total de ACEs"), QString::number(node.security.dacl.size()));
    }

    return QVariant();
}

QVariant AclTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return Language::GerenciadorTraducao::trText("Nome do Objeto");
            case 1: return Language::GerenciadorTraducao::trText("Tipo");
            case 2: return Language::GerenciadorTraducao::trText("Proprietário");
            case 3: return Language::GerenciadorTraducao::trText("Herança");
            case 4: return Language::GerenciadorTraducao::trText("Nível de Risco");
            case 5: return Language::GerenciadorTraducao::trText("Regras (ACEs)");
            default: break;
        }
    }
    return QVariant();
}

} // namespace AclAuditor::Model
