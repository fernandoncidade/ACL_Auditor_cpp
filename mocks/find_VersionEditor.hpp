#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;

namespace mocks::version_editor {

struct FileSpec {
    QString key;
    QString relative_path;
    QString content_key;
    QString group;
};

struct FileStatus {
    QString key;
    QString group;
    QString relative_path;
    QString path;
    bool exists = false;
    QStringList found;
    QStringList missing;
};

struct UpdateResult {
    QString key;
    QString group;
    QString relative_path;
    bool ok = false;
    QString message;
};

struct UpdateContext {
    QString pt_version;
    QString pt_date;
    QString en_version;
    QString en_date;
    QString pt_readme_version;
    QString en_readme_version;
};

/// Retorna a lista de especificações dos 27 arquivos monitorados e sincronizados pelo
/// VersionEditor do projeto ACL Auditor (com suporte bilíngue completo PT-BR e EN-US
/// para código C++, CLI headless, licenças EULA, contratos comerciais CLC, avisos NOTICE,
/// políticas de privacidade, documentação completa README.md, MANUAL.md, OBSERVACAO.md,
/// especificações modulares em docs/ e instaladores Inno Setup .iss):
///  1. source/assets/ABOUT/ABOUT_pt_BR.txt
///  2. source/assets/ABOUT/ABOUT_en_US.txt
///  3. source/assets/CLC/CLC_pt_BR.txt
///  4. source/assets/CLC/CLC_en_US.txt
///  5. source/assets/EULA/EULA_en_US.txt
///  6. source/assets/EULA/EULA_pt_BR.txt
///  7. source/assets/NOTICES/NOTICE_en_US.txt
///  8. source/assets/NOTICES/NOTICE_pt_BR.txt
///  9. source/assets/PRIVACY_POLICY/Privacy_Policy_pt_BR.txt
/// 10. source/assets/PRIVACY_POLICY/Privacy_Policy_en_US.txt
/// 11. main.cpp (setApplicationVersion e log inicial)
/// 12. source/main.cpp (setApplicationVersion e log inicial)
/// 13. source/cli/main_cli.cpp (banner, setApplicationVersion e CLI --version)
/// 14. source/ui/AboutDialog.cpp (versão em inglês e português na janela Sobre)
/// 15. README.md (badges de release, links de release notes e metadados de revisão técnica)
/// 16. MANUAL.md (versão e data em Português BR e English US)
/// 17. OBSERVACAO.md (versão documentada, snapshot técnico e cabeçalhos em PT/EN)
/// 18. docs/architecture.md (arquitetura do sistema em PT/EN)
/// 19. docs/cli-reference.md (referência completa da CLI em PT/EN)
/// 20. docs/compliance.md (mapeamentos de conformidade regulatória em PT/EN)
/// 21. docs/licensing.md (termos MIT e modelo comercial da Microsoft Store em PT/EN)
/// 22. docs/security-model.md (modelo de ameaças e integridade em PT/EN)
/// 23. docs/user-guide.md (guia de operações do usuário em PT/EN)
/// 24. docs/release-v*.md (notas oficiais de lançamento com hashes e tags em PT/EN)
/// 25. repo/ACL_Auditor_MSVC.iss (MyAppVersion e OutputBaseFilename)
/// 26. repo/ACL_Auditor.iss (MyAppVersion e OutputBaseFilename)
/// 27. repo/ACL_Auditor_MinGW.iss (MyAppVersion e OutputBaseFilename)
const QList<FileSpec> &file_specs();
QList<FileSpec> file_specs(const QString &base_dir);
QList<FileStatus> check_expected_lines(const QString &base_dir);
QList<UpdateResult> apply_updates(const UpdateContext &context, const QString &base_dir);
bool apply_line_updates(const QString &key, QStringList *lines, const UpdateContext &context);
QString detect_project_root();
QString normalize_dir(const QString &path);

class VersionEditorWidget : public QWidget {
    Q_OBJECT
public:
    explicit VersionEditorWidget(QWidget *parent = nullptr);

private:
    void selectBaseDirectory();
    void checkFiles();
    void saveChanges();
    void setStatusLines(const QStringList &lines);
    void setStatusGroups(const QList<QPair<QString, QStringList>> &groups);

    QString selected_base_dir_;

    QLineEdit *folder_path_ = nullptr;

    QLineEdit *pt_version_ = nullptr;
    QSpinBox *pt_day_ = nullptr;
    QComboBox *pt_month_ = nullptr;
    QSpinBox *pt_year_ = nullptr;

    QLineEdit *en_version_ = nullptr;
    QSpinBox *en_day_ = nullptr;
    QComboBox *en_month_ = nullptr;
    QSpinBox *en_year_ = nullptr;

    QPushButton *select_folder_btn_ = nullptr;
    QPushButton *check_btn_ = nullptr;
    QPushButton *save_btn_ = nullptr;
    QPushButton *refresh_btn_ = nullptr;

    QTabWidget *status_tabs_ = nullptr;
};

}  // namespace mocks::version_editor
