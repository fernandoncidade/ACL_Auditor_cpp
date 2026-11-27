#pragma once

#include <QHash>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTranslator>

namespace AclAuditor::Language {

enum class Idioma {
    PortuguesBrasil,
    InglesEstadosUnidos
};

/**
 * @brief Gerenciador central de internacionalização e traduções dinâmicas do ACL Auditor.
 * Permite alternar instantaneamente entre pt_BR e en_US em tempo de execução sem reiniciar a aplicação.
 */
class GerenciadorTraducao : public QObject {
    Q_OBJECT

public:
    static GerenciadorTraducao& instance();

    void inicializar();
    bool definir_idioma(const QString& codigo_idioma);
    void mudarIdioma(Idioma novoIdioma);

    QString obter_idioma_atual() const { return m_idiomaAtual; }
    QString codigoIdiomaAtual() const { return m_idiomaAtual; }
    Idioma idiomaAtual() const;

    const QMap<QString, QString>& idiomas_disponiveis() const { return m_idiomasDisponiveis; }

    static QString trText(const char* sourceText);
    static QString trText(const QString& sourceText);

signals:
    void idioma_alterado(const QString& codigo_idioma);
    void idiomaAlterado(Idioma novoIdioma);

private:
    explicit GerenciadorTraducao(QObject* parent = nullptr);
    ~GerenciadorTraducao() override;

    bool aplicar_traducao();
    void carregar_configuracao_idioma();
    void salvar_configuracao_idioma() const;
    void remover_tradutores();

    QString m_idiomaAtual{QStringLiteral("pt_BR")};
    QString m_idiomaPadrao{QStringLiteral("pt_BR")};
    QMap<QString, QString> m_idiomasDisponiveis;

    QTranslator* m_tradutorApp{nullptr};
    QTranslator* m_tradutorQt{nullptr};
    QHash<QString, QTranslator*> m_appTranslatorsCache;
    QHash<QString, QTranslator*> m_qtTranslatorsCache;
};

} // namespace AclAuditor::Language
