#include "RiskTableModel.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include <QColor>

namespace AclAuditor::Model {

RiskTableModel::RiskTableModel(QObject* parent) : QAbstractTableModel(parent) {
}

void RiskTableModel::clear() {
    beginResetModel();
    m_findings.clear();
    endResetModel();
}

void RiskTableModel::setFindings(const std::vector<Engine::RiskFinding>& findings) {
    beginResetModel();
    m_findings = findings;
    endResetModel();
}

void RiskTableModel::refreshTranslations() {
    emit layoutChanged();
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}

const Engine::RiskFinding* RiskTableModel::getFindingAt(int row) const {
    if (row < 0 || row >= static_cast<int>(m_findings.size())) return nullptr;
    return &m_findings[row];
}

int RiskTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_findings.size());
}

int RiskTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 5;
}

static QString translateRightsDetailed(const QString& raw) {
    if (raw.contains(QStringLiteral(", "))) {
        const QStringList parts = raw.split(QStringLiteral(", "));
        QStringList trParts;
        trParts.reserve(parts.size());
        for (const QString& p : parts) {
            trParts.append(Language::GerenciadorTraducao::trText(p.trimmed()));
        }
        return trParts.join(QStringLiteral(", "));
    }
    return Language::GerenciadorTraducao::trText(raw);
}

QString RiskTableModel::formatFindingDescriptionLocalized(const Engine::RiskFinding& f) {
    QString raw = QString::fromStdWString(f.description);
    if (raw.isEmpty()) return QString();

    const QString prefixOrphan = QStringLiteral("Foi identificado um SID na lista de controle de acesso correspondente a uma conta excluída ou inexistente: ");
    if (raw.startsWith(prefixOrphan)) {
        return Language::GerenciadorTraducao::trText("Foi identificado um SID na lista de controle de acesso correspondente a uma conta excluída ou inexistente: %1")
            .arg(raw.mid(prefixOrphan.length()));
    }

    if (raw.startsWith(QStringLiteral("O grupo amplo '"))) {
        int endQuote = raw.indexOf(QStringLiteral("' possui permissão de gravação/criação de arquivos nesta pasta."));
        if (endQuote != -1) {
            QString group = raw.mid(15, endQuote - 15);
            return Language::GerenciadorTraducao::trText("O grupo amplo '%1' possui permissão de gravação/criação de arquivos nesta pasta. Isso permite que qualquer usuário padrão modifique ou plante arquivos.")
                .arg(Language::GerenciadorTraducao::trText(group));
        }
    }

    if (raw.startsWith(QStringLiteral("O executável ou script '"))) {
        int midQuote = raw.indexOf(QStringLiteral("' pode ser sobrescrito ou modificado pelo grupo '"));
        if (midQuote != -1) {
            QString fileName = raw.mid(24, midQuote - 24);
            int startGroup = midQuote + 49;
            int endGroup = raw.indexOf(QStringLiteral("'. Um atacante"), startGroup);
            if (endGroup != -1) {
                QString group = raw.mid(startGroup, endGroup - startGroup);
                return Language::GerenciadorTraducao::trText("O executável ou script '%1' pode ser sobrescrito ou modificado pelo grupo '%2'. Um atacante ou malware pode substituir este binário para elevar privilégios.")
                    .arg(fileName, Language::GerenciadorTraducao::trText(group));
            }
        }
    }

    if (raw.startsWith(QStringLiteral("A conta ou grupo '"))) {
        int endQuote = raw.indexOf(QStringLiteral("' possui direitos para alterar permissões (WRITE_DAC) ou apropriar-se do objeto (WRITE_OWNER)."));
        if (endQuote != -1) {
            QString principal = raw.mid(18, endQuote - 18);
            return Language::GerenciadorTraducao::trText("A conta ou grupo '%1' possui direitos para alterar permissões (WRITE_DAC) ou apropriar-se do objeto (WRITE_OWNER). Isso permite que essa conta conceda a si mesma Controle Total.")
                .arg(Language::GerenciadorTraducao::trText(principal));
        }
    }

    if (raw == QStringLiteral("A pasta está configurada como protegida (SE_DACL_PROTECTED). Ela não herda mais permissões do diretório pai, podendo ocultar ou despadronizar o modelo de governança.")) {
        return Language::GerenciadorTraducao::trText("A pasta está configurada como protegida (SE_DACL_PROTECTED). Ela não herda mais permissões do diretório pai, podendo ocultar ou despadronizar o modelo de governança.");
    }

    if (raw.startsWith(QStringLiteral("O objeto '"))) {
        int midQuote = raw.indexOf(QStringLiteral("' possui indícios de conter dados sensíveis ou corporativos restritos, porém concede acesso a '"));
        if (midQuote != -1) {
            QString fileName = raw.mid(10, midQuote - 10);
            int startPrincipal = midQuote + 96;
            int endPrincipal = raw.indexOf(QStringLiteral("'."), startPrincipal);
            if (endPrincipal != -1) {
                QString principal = raw.mid(startPrincipal, endPrincipal - startPrincipal);
                return Language::GerenciadorTraducao::trText("O objeto '%1' possui indícios de conter dados sensíveis ou corporativos restritos, porém concede acesso a '%2'.")
                    .arg(fileName, Language::GerenciadorTraducao::trText(principal));
            }
        }
    }

    if (raw.startsWith(QStringLiteral("O serviço '"))) {
        int midQuote = raw.indexOf(QStringLiteral("' possui caminho executável sem aspas: "));
        if (midQuote != -1) {
            QString serviceName = raw.mid(11, midQuote - 11);
            int startBin = midQuote + 39;
            int endBin = raw.indexOf(QStringLiteral(". O Windows tentará executar"), startBin);
            if (endBin != -1) {
                QString binPath = raw.mid(startBin, endBin - startBin);
                return Language::GerenciadorTraducao::trText("O serviço '%1' possui caminho executável sem aspas: %2. O Windows tentará executar binários intermediários nos espaços ao iniciar o serviço.")
                    .arg(serviceName, binPath);
            }
        }
    }

    if (raw.startsWith(QStringLiteral("O binário do serviço '"))) {
        int endQuote = raw.indexOf(QStringLiteral("' permite modificação por usuários comuns ou Everyone. Um atacante pode substituir este executável para obter execução como SYSTEM."));
        if (endQuote != -1) {
            QString binPath = raw.mid(22, endQuote - 22);
            return Language::GerenciadorTraducao::trText("O binário do serviço '%1' permite modificação por usuários comuns ou Everyone. Um atacante pode substituir este executável para obter execução como SYSTEM.")
                .arg(binPath);
        }
    }

    return Language::GerenciadorTraducao::trText(raw);
}

QString RiskTableModel::formatFindingTechnicalDetailLocalized(const Engine::RiskFinding& f) {
    QString raw = QString::fromStdWString(f.technicalDetail);
    if (raw.isEmpty()) return QString();

    if (raw.startsWith(QStringLiteral("Tipo: "))) {
        int maskPos = raw.indexOf(QStringLiteral(", Máscara: "));
        if (maskPos != -1) {
            QString typeStr = raw.mid(6, maskPos - 6);
            QString maskStr = raw.mid(maskPos + 11);
            return Language::GerenciadorTraducao::trText("Tipo: %1, Máscara: %2")
                .arg(Language::GerenciadorTraducao::trText(typeStr), maskStr);
        }
    }

    if (raw.startsWith(QStringLiteral("Direitos detectados: "))) {
        int parenOpen = raw.indexOf(QLatin1Char('('));
        int parenClose = raw.lastIndexOf(QLatin1Char(')'));
        if (parenOpen != -1 && parenClose > parenOpen) {
            QString summary = raw.mid(21, parenOpen - 21).trimmed();
            QString detailed = raw.mid(parenOpen + 1, parenClose - parenOpen - 1).trimmed();
            return Language::GerenciadorTraducao::trText("Direitos detectados: %1 (%2)")
                .arg(Language::GerenciadorTraducao::trText(summary), translateRightsDetailed(detailed));
        } else {
            QString summary = raw.mid(21).trimmed();
            return Language::GerenciadorTraducao::trText("Direitos detectados: %1")
                .arg(Language::GerenciadorTraducao::trText(summary));
        }
    }

    if (raw.startsWith(QStringLiteral("Arquivo: "))) {
        int permPos = raw.indexOf(QStringLiteral(", Permissão: "));
        if (permPos != -1) {
            QString file = raw.mid(9, permPos - 9);
            QString perm = raw.mid(permPos + 13);
            return Language::GerenciadorTraducao::trText("Arquivo: %1, Permissão: %2")
                .arg(file, Language::GerenciadorTraducao::trText(perm));
        }
    }

    if (raw.startsWith(QStringLiteral("Máscara: "))) {
        return Language::GerenciadorTraducao::trText("Máscara: %1").arg(raw.mid(9));
    }

    if (raw == QStringLiteral("Flag SE_DACL_PROTECTED detectada no Security Descriptor.")) {
        return Language::GerenciadorTraducao::trText("Flag SE_DACL_PROTECTED detectada no Security Descriptor.");
    }

    if (raw.startsWith(QStringLiteral("Permissão concedida: "))) {
        return Language::GerenciadorTraducao::trText("Permissão concedida: %1")
            .arg(Language::GerenciadorTraducao::trText(raw.mid(21)));
    }

    if (raw.startsWith(QStringLiteral("Tipo de inicialização: "))) {
        return Language::GerenciadorTraducao::trText("Tipo de inicialização: %1")
            .arg(Language::GerenciadorTraducao::trText(raw.mid(23)));
    }

    if (raw.startsWith(QStringLiteral("Conta do serviço: "))) {
        return Language::GerenciadorTraducao::trText("Conta do serviço: %1")
            .arg(raw.mid(18));
    }

    return Language::GerenciadorTraducao::trText(raw);
}

QVariant RiskTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_findings.size())) return QVariant();

    const auto& f = m_findings[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: {
                switch (f.severity) {
                    case Engine::RiskSeverity::Critical: return Language::GerenciadorTraducao::trText("CRÍTICO");
                    case Engine::RiskSeverity::High:     return Language::GerenciadorTraducao::trText("ALTO");
                    case Engine::RiskSeverity::Medium:   return Language::GerenciadorTraducao::trText("MÉDIO");
                    case Engine::RiskSeverity::Low:      return Language::GerenciadorTraducao::trText("BAIXO");
                    case Engine::RiskSeverity::Info:     return Language::GerenciadorTraducao::trText("INFO");
                }
                return Language::GerenciadorTraducao::trText("DESCONHECIDO");
            }
            case 1: return Language::GerenciadorTraducao::trText(QString::fromStdWString(f.title));
            case 2: return QString::fromStdWString(f.affectedPath);
            case 3: return Language::GerenciadorTraducao::trText(QString::fromStdWString(f.affectedPrincipal));
            case 4: return Language::GerenciadorTraducao::trText(QString::fromStdWString(f.remediationAdvice));
            default: break;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 0) {
            switch (f.severity) {
                case Engine::RiskSeverity::Critical: return QColor(220, 40, 40);
                case Engine::RiskSeverity::High:     return QColor(230, 110, 20);
                case Engine::RiskSeverity::Medium:   return QColor(200, 160, 20);
                case Engine::RiskSeverity::Low:      return QColor(60, 140, 200);
                case Engine::RiskSeverity::Info:     return QColor(100, 180, 80);
            }
        }
    } else if (role == Qt::ToolTipRole) {
        return QStringLiteral("%1:\n%2\n\n%3:\n%4\n\n%5:\n%6")
            .arg(Language::GerenciadorTraducao::trText("Descrição"), formatFindingDescriptionLocalized(f))
            .arg(Language::GerenciadorTraducao::trText("Detalhe Técnico"), formatFindingTechnicalDetailLocalized(f))
            .arg(Language::GerenciadorTraducao::trText("Comando icacls"), QString::fromStdWString(f.icaclsCommand));
    } else if (role == Qt::UserRole) {
        if (index.column() == 0) {
            switch (f.severity) {
                case Engine::RiskSeverity::Critical: return 4;
                case Engine::RiskSeverity::High:     return 3;
                case Engine::RiskSeverity::Medium:   return 2;
                case Engine::RiskSeverity::Low:      return 1;
                case Engine::RiskSeverity::Info:     return 0;
            }
        }
    }

    return QVariant();
}

QVariant RiskTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return Language::GerenciadorTraducao::trText("Severidade");
            case 1: return Language::GerenciadorTraducao::trText("Vulnerabilidade Identificada");
            case 2: return Language::GerenciadorTraducao::trText("Caminho Afetado");
            case 3: return Language::GerenciadorTraducao::trText("Principal / Alvo");
            case 4: return Language::GerenciadorTraducao::trText("Ação Recomendada");
            default: break;
        }
    }
    return QVariant();
}

} // namespace AclAuditor::Model
