#include "GerenciadorTraducao.hpp"
#include "../utils/ApplicationPathUtils.hpp"
#include "../utils/LogManager.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibraryInfo>
#include <QLocale>
#include <QStandardPaths>

namespace AclAuditor::Language {

GerenciadorTraducao& GerenciadorTraducao::instance() {
    static GerenciadorTraducao inst;
    return inst;
}

GerenciadorTraducao::GerenciadorTraducao(QObject* parent)
    : QObject(parent) {
    m_idiomasDisponiveis.insert(QStringLiteral("pt_BR"), QStringLiteral("Português (Brasil)"));
    m_idiomasDisponiveis.insert(QStringLiteral("en_US"), QStringLiteral("English (United States)"));
}

GerenciadorTraducao::~GerenciadorTraducao() {
    remover_tradutores();
    qDeleteAll(m_appTranslatorsCache);
    m_appTranslatorsCache.clear();
    qDeleteAll(m_qtTranslatorsCache);
    m_qtTranslatorsCache.clear();
}

void GerenciadorTraducao::inicializar() {
    carregar_configuracao_idioma();
    aplicar_traducao();
}

Idioma GerenciadorTraducao::idiomaAtual() const {
    if (m_idiomaAtual.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)) {
        return Idioma::InglesEstadosUnidos;
    }
    return Idioma::PortuguesBrasil;
}

void GerenciadorTraducao::mudarIdioma(Idioma novoIdioma) {
    if (novoIdioma == Idioma::InglesEstadosUnidos) {
        definir_idioma(QStringLiteral("en_US"));
    } else {
        definir_idioma(QStringLiteral("pt_BR"));
    }
}

bool GerenciadorTraducao::definir_idioma(const QString& codigo_idioma) {
    if (!m_idiomasDisponiveis.contains(codigo_idioma)) {
        return false;
    }

    if (m_idiomaAtual == codigo_idioma && m_tradutorApp != nullptr) {
        return true;
    }

    m_idiomaAtual = codigo_idioma;
    salvar_configuracao_idioma();
    const bool res = aplicar_traducao();

    emit idioma_alterado(m_idiomaAtual);
    emit idiomaAlterado(idiomaAtual());

    return res;
}

QString GerenciadorTraducao::trText(const char* sourceText) {
    return QCoreApplication::translate("InterfaceGrafica", sourceText);
}

QString GerenciadorTraducao::trText(const QString& sourceText) {
    return QCoreApplication::translate("InterfaceGrafica", sourceText.toUtf8().constData());
}

void GerenciadorTraducao::remover_tradutores() {
    auto* app = QCoreApplication::instance();
    if (!app) return;

    if (m_tradutorApp) {
        app->removeTranslator(m_tradutorApp);
        m_tradutorApp = nullptr;
    }
    if (m_tradutorQt) {
        app->removeTranslator(m_tradutorQt);
        m_tradutorQt = nullptr;
    }
}

bool GerenciadorTraducao::aplicar_traducao() {
    auto* app = QCoreApplication::instance();
    if (!app) return false;

    remover_tradutores();

    const QString locale_code = QString(m_idiomaAtual).replace(QLatin1Char('_'), QLatin1Char('-'));
    QLocale::setDefault(QLocale(locale_code));

    // 1. Carregar tradutor do próprio Qt (se disponível)
    if (m_qtTranslatorsCache.contains(m_idiomaAtual)) {
        m_tradutorQt = m_qtTranslatorsCache.value(m_idiomaAtual);
        if (m_tradutorQt) {
            app->installTranslator(m_tradutorQt);
        }
    } else {
        auto* qtTrans = new QTranslator(this);
        const QString qtTransPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
        if (qtTrans->load(QLocale(locale_code), QStringLiteral("qtbase"), QStringLiteral("_"), qtTransPath) ||
            qtTrans->load(QStringLiteral("qt_") + m_idiomaAtual, qtTransPath)) {
            m_tradutorQt = qtTrans;
            app->installTranslator(m_tradutorQt);
            m_qtTranslatorsCache.insert(m_idiomaAtual, m_tradutorQt);
        } else {
            delete qtTrans;
            m_qtTranslatorsCache.insert(m_idiomaAtual, nullptr);
        }
    }

    // 2. Procurar arquivo de tradução da aplicação (.qm)
    if (m_appTranslatorsCache.contains(m_idiomaAtual)) {
        m_tradutorApp = m_appTranslatorsCache.value(m_idiomaAtual);
        if (m_tradutorApp) {
            app->installTranslator(m_tradutorApp);
            QCoreApplication::postEvent(app, new QEvent(QEvent::LanguageChange));
            return true;
        }
    }

    const QString appDir = Utils::ApplicationPathUtils::getApplicationDirPath();
    const QString transDir = Utils::ApplicationPathUtils::getTranslationsDirPath();

    const QStringList candidateDirectories = {
        transDir,
        appDir + QStringLiteral("/translations"),
        appDir + QStringLiteral("/../../source/language/translations"),
        QDir::currentPath() + QStringLiteral("/source/language/translations"),
        QDir::currentPath() + QStringLiteral("/translations")
    };

    const QStringList candidateFileNames = {
        QStringLiteral("aclauditor_%1.qm").arg(m_idiomaAtual),
        QStringLiteral("acl_auditor_%1.qm").arg(m_idiomaAtual),
        QStringLiteral("aclauditor_%1.qm").arg(locale_code),
        QStringLiteral("%1.qm").arg(m_idiomaAtual)
    };

    bool loaded = false;
    for (const QString& dirPath : candidateDirectories) {
        if (!QDir(dirPath).exists()) continue;

        for (const QString& fileName : candidateFileNames) {
            const QString fullPath = QDir(dirPath).filePath(fileName);
            if (QFile::exists(fullPath)) {
                auto* newTrans = new QTranslator(this);
                if (newTrans->load(fullPath)) {
                    m_tradutorApp = newTrans;
                    app->installTranslator(m_tradutorApp);
                    m_appTranslatorsCache.insert(m_idiomaAtual, m_tradutorApp);
                    loaded = true;
                    Utils::LogManager::info(QStringLiteral("Tradução carregada com sucesso: ") + fullPath);
                    break;
                } else {
                    delete newTrans;
                }
            }
        }
        if (loaded) break;
    }

    if (!loaded && !m_appTranslatorsCache.contains(m_idiomaAtual)) {
        m_appTranslatorsCache.insert(m_idiomaAtual, nullptr);
    }

    // Dispara evento LanguageChange para atualizar dinamicamente todas as telas
    QCoreApplication::postEvent(app, new QEvent(QEvent::LanguageChange));

    return loaded || (m_idiomaAtual == m_idiomaPadrao);
}

void GerenciadorTraducao::carregar_configuracao_idioma() {
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString configFile = configDir + QStringLiteral("/language.json");

    if (QFile::exists(configFile)) {
        QFile file(configFile);
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isObject()) {
                const QString lang = doc.object().value(QStringLiteral("language")).toString();
                if (m_idiomasDisponiveis.contains(lang)) {
                    m_idiomaAtual = lang;
                    return;
                }
            }
        }
    }

    // Fallback: detect system UI language
    const QString sysLocale = QLocale::system().name();
    if (sysLocale.startsWith(QStringLiteral("pt"), Qt::CaseInsensitive)) {
        m_idiomaAtual = QStringLiteral("pt_BR");
    } else {
        m_idiomaAtual = QStringLiteral("en_US");
    }
}

void GerenciadorTraducao::salvar_configuracao_idioma() const {
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(configDir);

    const QString configFile = configDir + QStringLiteral("/language.json");
    QFile file(configFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonObject root;
        root[QStringLiteral("language")] = m_idiomaAtual;
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }
}

} // namespace AclAuditor::Language
