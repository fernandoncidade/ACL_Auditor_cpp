#pragma once

#include "../engine/AclTypes.hpp"
#include <QAbstractItemModel>
#include <vector>
#include <memory>

namespace AclAuditor::Model {

struct AclTreeItem {
    Engine::FileSecurityNode data;
    AclTreeItem* parentItem{nullptr};
    std::vector<std::unique_ptr<AclTreeItem>> childItems;

    explicit AclTreeItem(Engine::FileSecurityNode nodeData, AclTreeItem* parent = nullptr)
        : data(std::move(nodeData)), parentItem(parent) {}

    void appendChild(std::unique_ptr<AclTreeItem> child) {
        childItems.push_back(std::move(child));
    }

    AclTreeItem* child(int row) {
        if (row < 0 || row >= static_cast<int>(childItems.size())) return nullptr;
        return childItems[row].get();
    }

    int childCount() const { return static_cast<int>(childItems.size()); }
    int row() const {
        if (parentItem) {
            for (size_t i = 0; i < parentItem->childItems.size(); ++i) {
                if (parentItem->childItems[i].get() == this) return static_cast<int>(i);
            }
        }
        return 0;
    }
};

class AclTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit AclTreeModel(QObject* parent = nullptr);
    ~AclTreeModel() override = default;

    void setRootNode(const Engine::FileSecurityNode& rootNode);
    void clear();

    const Engine::FileSecurityNode* getNodeAt(const QModelIndex& index) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void refreshTranslations() {
        emit layoutChanged();
        emit headerDataChanged(Qt::Horizontal, 0, 5);
    }

private:
    std::unique_ptr<AclTreeItem> m_rootItem;
    void buildSubtree(AclTreeItem* parentItem, const Engine::FileSecurityNode& node);
};

} // namespace AclAuditor::Model
