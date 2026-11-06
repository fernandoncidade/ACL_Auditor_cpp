#include "DiffTableModel.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include <QColor>

namespace AclAuditor::Model {

DiffTableModel::DiffTableModel(QObject* parent) : QAbstractTableModel(parent) {
}

void DiffTableModel::clear() {
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

void DiffTableModel::refreshTranslations() {
    emit layoutChanged();
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}

void DiffTableModel::setDiff(const Engine::SnapshotDiffResult& diff) {
    beginResetModel();
    m_entries = diff.changes;
    endResetModel();
}

void DiffTableModel::setDiff(Engine::SnapshotDiffResult&& diff) {
    beginResetModel();
    m_entries = std::move(diff.changes);
    endResetModel();
}

static QString formatSummaryLocalized(const std::wstring& rawSummary) {
    QString s = QString::fromStdWString(rawSummary);
    const QString prefixAceAdded = QStringLiteral("Nova permissão concedida para: ");
    if (s.startsWith(prefixAceAdded)) {
        return Language::GerenciadorTraducao::trText("Nova permissão concedida para: %1")
            .arg(Language::GerenciadorTraducao::trText(s.mid(prefixAceAdded.length())));
    }
    const QString prefixAceRemoved = QStringLiteral("Permissão revogada para: ");
    if (s.startsWith(prefixAceRemoved)) {
        return Language::GerenciadorTraducao::trText("Permissão revogada para: %1")
            .arg(Language::GerenciadorTraducao::trText(s.mid(prefixAceRemoved.length())));
    }
    const QString prefixEscalated = QStringLiteral("Elevação de permissão para: ");
    if (s.startsWith(prefixEscalated)) {
        return Language::GerenciadorTraducao::trText("Elevação de permissão para: %1")
            .arg(Language::GerenciadorTraducao::trText(s.mid(prefixEscalated.length())));
    }
    const QString prefixReduced = QStringLiteral("Redução de permissão para: ");
    if (s.startsWith(prefixReduced)) {
        return Language::GerenciadorTraducao::trText("Redução de permissão para: %1")
            .arg(Language::GerenciadorTraducao::trText(s.mid(prefixReduced.length())));
    }
    return Language::GerenciadorTraducao::trText(s);
}

static QString translateValueLocalized(const std::wstring& rawValue) {
    if (rawValue.empty()) return QString();
    QString qs = QString::fromStdWString(rawValue);
    if (qs.contains(QStringLiteral(", "))) {
        const QStringList parts = qs.split(QStringLiteral(", "));
        QStringList trParts;
        trParts.reserve(parts.size());
        for (const QString& p : parts) {
            trParts.append(Language::GerenciadorTraducao::trText(p.trimmed()));
        }
        return trParts.join(QStringLiteral(", "));
    }
    return Language::GerenciadorTraducao::trText(qs);
}

int DiffTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_entries.size());
}

int DiffTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 5;
}

QVariant DiffTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_entries.size())) return QVariant();

    const auto& entry = m_entries[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: {
                switch (entry.changeType) {
                    case Engine::DiffChangeType::ObjectAdded:         return Language::GerenciadorTraducao::trText("+ Objeto Adicionado");
                    case Engine::DiffChangeType::ObjectRemoved:       return Language::GerenciadorTraducao::trText("- Objeto Removido");
                    case Engine::DiffChangeType::OwnerChanged:         return Language::GerenciadorTraducao::trText("~ Proprietário Alterado");
                    case Engine::DiffChangeType::InheritanceChanged:   return Language::GerenciadorTraducao::trText("~ Herança Alterada");
                    case Engine::DiffChangeType::AceAdded:             return Language::GerenciadorTraducao::trText("+ Nova Permissão (ACE)");
                    case Engine::DiffChangeType::AceRemoved:           return Language::GerenciadorTraducao::trText("- Permissão Removida");
                    case Engine::DiffChangeType::RightsEscalated:      return Language::GerenciadorTraducao::trText("! Elevação de Direitos");
                    case Engine::DiffChangeType::RightsReduced:        return Language::GerenciadorTraducao::trText("~ Redução de Direitos");
                    case Engine::DiffChangeType::RiskIncreased:        return Language::GerenciadorTraducao::trText("! Risco Aumentado");
                    case Engine::DiffChangeType::RiskDecreased:        return Language::GerenciadorTraducao::trText("~ Risco Mitigado");
                }
                return Language::GerenciadorTraducao::trText("Alteração");
            }
            case 1: return QString::fromStdWString(entry.path);
            case 2: return formatSummaryLocalized(entry.summary);
            case 3: return translateValueLocalized(entry.oldValue);
            case 4: return translateValueLocalized(entry.newValue);
            default: break;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 0) {
            switch (entry.changeType) {
                case Engine::DiffChangeType::RightsEscalated:
                case Engine::DiffChangeType::RiskIncreased:
                    return QColor(220, 40, 40);
                case Engine::DiffChangeType::AceAdded:
                    return QColor(40, 160, 60);
                case Engine::DiffChangeType::AceRemoved:
                case Engine::DiffChangeType::ObjectRemoved:
                    return QColor(160, 100, 100);
                default:
                    return QColor(200, 140, 30);
            }
        }
    }

    return QVariant();
}

QVariant DiffTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return Language::GerenciadorTraducao::trText("Tipo de Mudança");
            case 1: return Language::GerenciadorTraducao::trText("Caminho");
            case 2: return Language::GerenciadorTraducao::trText("Resumo da Alteração");
            case 3: return Language::GerenciadorTraducao::trText("Snapshot Anterior");
            case 4: return Language::GerenciadorTraducao::trText("Snapshot Atual");
            default: break;
        }
    }
    return QVariant();
}

} // namespace AclAuditor::Model
