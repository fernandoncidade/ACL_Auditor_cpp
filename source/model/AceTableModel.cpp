#include "AceTableModel.hpp"
#include "../engine/NtfsSecurityUtils.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include <QColor>

namespace AclAuditor::Model {

AceTableModel::AceTableModel(QObject* parent) : QAbstractTableModel(parent) {
}

void AceTableModel::clear() {
    beginResetModel();
    m_aces.clear();
    endResetModel();
}

void AceTableModel::setAces(const std::vector<Engine::AceEntry>& aces) {
    beginResetModel();
    m_aces = aces;
    endResetModel();
}

const Engine::AceEntry* AceTableModel::getAceAt(int row) const {
    if (row < 0 || row >= static_cast<int>(m_aces.size())) return nullptr;
    return &m_aces[row];
}

int AceTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_aces.size());
}

int AceTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 6;
}

static QString getRightsSummaryLocalized(const Engine::AceEntry& ace) {
    if (ace.fullControl) return Language::GerenciadorTraducao::trText("Controle Total");
    if (ace.modify) return Language::GerenciadorTraducao::trText("Modificar");
    if (ace.readAndExecute && ace.write) return Language::GerenciadorTraducao::trText("Leitura, Execução e Gravação");
    if (ace.readAndExecute) return Language::GerenciadorTraducao::trText("Leitura e Execução");
    if (ace.write) return Language::GerenciadorTraducao::trText("Gravação");
    if (ace.read) return Language::GerenciadorTraducao::trText("Somente Leitura");
    return Language::GerenciadorTraducao::trText("Permissões Especiais");
}

static QString getScopeDescriptionLocalized(const Engine::AceEntry& ace) {
    if (ace.isInherited) {
        if (ace.inheritedFromPath.empty()) {
            return Language::GerenciadorTraducao::trText("Herdado da pasta pai");
        }
        return Language::GerenciadorTraducao::trText("Herdado de: %1").arg(QString::fromStdWString(ace.inheritedFromPath));
    }
    if (ace.appliesToCurrent && ace.appliesToSubfolders && ace.appliesToFiles) {
        return Language::GerenciadorTraducao::trText("Esta pasta, subpastas e arquivos");
    }
    if (ace.appliesToCurrent && ace.appliesToSubfolders) {
        return Language::GerenciadorTraducao::trText("Esta pasta e subpastas");
    }
    if (ace.appliesToCurrent && ace.appliesToFiles) {
        return Language::GerenciadorTraducao::trText("Esta pasta e arquivos");
    }
    if (ace.appliesToSubfolders && ace.appliesToFiles) {
        return Language::GerenciadorTraducao::trText("Apenas subpastas e arquivos");
    }
    return Language::GerenciadorTraducao::trText("Apenas esta pasta");
}

static QString formatAccessMaskDetailedLocalized(DWORD mask, bool isDirectory) {
    if ((mask & FILE_ALL_ACCESS) == FILE_ALL_ACCESS) {
        return Language::GerenciadorTraducao::trText("Controle Total (Full Control)");
    }

    QStringList rights;
    if (isDirectory) {
        if (mask & FILE_LIST_DIRECTORY)   rights.append(Language::GerenciadorTraducao::trText("Listar Diretório / Ler Dados"));
        if (mask & FILE_ADD_FILE)         rights.append(Language::GerenciadorTraducao::trText("Criar Arquivos / Gravar Dados"));
        if (mask & FILE_ADD_SUBDIRECTORY) rights.append(Language::GerenciadorTraducao::trText("Criar Subpastas"));
        if (mask & FILE_TRAVERSE)         rights.append(Language::GerenciadorTraducao::trText("Atravessar Pastas / Executar"));
        if (mask & FILE_DELETE_CHILD)     rights.append(Language::GerenciadorTraducao::trText("Excluir Subpastas e Arquivos"));
    } else {
        if (mask & FILE_READ_DATA)        rights.append(Language::GerenciadorTraducao::trText("Ler Dados"));
        if (mask & FILE_WRITE_DATA)       rights.append(Language::GerenciadorTraducao::trText("Gravar Dados"));
        if (mask & FILE_APPEND_DATA)      rights.append(Language::GerenciadorTraducao::trText("Acrescentar Dados"));
        if (mask & FILE_EXECUTE)          rights.append(Language::GerenciadorTraducao::trText("Executar Arquivo"));
    }

    if (mask & DELETE)                    rights.append(Language::GerenciadorTraducao::trText("Excluir"));
    if (mask & READ_CONTROL)              rights.append(Language::GerenciadorTraducao::trText("Ler Permissões"));
    if (mask & WRITE_DAC)                 rights.append(Language::GerenciadorTraducao::trText("Alterar Permissões (WRITE_DAC)"));
    if (mask & WRITE_OWNER)               rights.append(Language::GerenciadorTraducao::trText("Apropriar-se (WRITE_OWNER)"));
    if (mask & SYNCHRONIZE)               rights.append(Language::GerenciadorTraducao::trText("Sincronizar"));

    if (rights.isEmpty()) {
        return QString::fromStdWString(Engine::NtfsSecurityUtils::formatAccessMaskHex(mask));
    }

    return rights.join(QStringLiteral(", "));
}

QVariant AceTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_aces.size())) return QVariant();

    const auto& ace = m_aces[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return (ace.type == Engine::AccessType::Allow ?
                Language::GerenciadorTraducao::trText("Permitir (Allow)") :
                Language::GerenciadorTraducao::trText("Negar (Deny)"));
            case 1: return QString::fromStdWString(ace.trustee.getDisplayName());
            case 2: return getRightsSummaryLocalized(ace);
            case 3: return ace.isInherited ? getScopeDescriptionLocalized(ace) : Language::GerenciadorTraducao::trText("Explícito (Objeto Atual)");
            case 4: return getScopeDescriptionLocalized(ace);
            case 5: return QString::fromStdWString(Engine::NtfsSecurityUtils::formatAccessMaskHex(ace.accessMask));
            default: break;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 0) {
            return (ace.type == Engine::AccessType::Deny ? QColor(220, 50, 50) : QColor(40, 160, 60));
        }
        if (ace.trustee.isOrphaned) {
            return QColor(220, 100, 20);
        }
    } else if (role == Qt::ToolTipRole) {
        return QStringLiteral("%1: %2\nSID: %3\n%4: %5\n%6: %7\n%8: %9")
            .arg(Language::GerenciadorTraducao::trText("Conta"), QString::fromStdWString(ace.trustee.getDisplayName()))
            .arg(QString::fromStdWString(ace.trustee.sidString))
            .arg(Language::GerenciadorTraducao::trText("Tipo"), ace.type == Engine::AccessType::Allow ? Language::GerenciadorTraducao::trText("Permitir") : Language::GerenciadorTraducao::trText("Negar"))
            .arg(Language::GerenciadorTraducao::trText("Direitos de Acesso"), formatAccessMaskDetailedLocalized(ace.accessMask, ace.appliesToSubfolders));
    } else if (role == Qt::UserRole) {
        if (index.column() == 0) {
            return (ace.type == Engine::AccessType::Deny ? 0 : 1);
        }
        if (index.column() == 5) {
            return static_cast<qulonglong>(ace.accessMask);
        }
    }

    return QVariant();
}

QVariant AceTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return Language::GerenciadorTraducao::trText("Tipo");
            case 1: return Language::GerenciadorTraducao::trText("Principal / Identidade");
            case 2: return Language::GerenciadorTraducao::trText("Direitos de Acesso");
            case 3: return Language::GerenciadorTraducao::trText("Origem");
            case 4: return Language::GerenciadorTraducao::trText("Escopo de Aplicação");
            case 5: return Language::GerenciadorTraducao::trText("Máscara Hex");
            default: break;
        }
    }
    return QVariant();
}

} // namespace AclAuditor::Model
