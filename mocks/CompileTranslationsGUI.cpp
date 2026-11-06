#include "CompileTranslationsGUI.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScopeGuard>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStringList>
#include <QTextCursor>
#include <QVBoxLayout>

namespace {

QString normalize_target_id(QString value)
{
    value = value.trimmed().toLower();
    value.remove(QLatin1Char('_'));
    value.remove(QLatin1Char('-'));
    value.remove(QLatin1Char(' '));

    if (value.contains(QStringLiteral("aclauditor")) || value.contains(QStringLiteral("acl"))
        || value.contains(QStringLiteral("auditor")) || value == QStringLiteral("host") || value == QStringLiteral("app")) {
        return QStringLiteral("aclauditor");
    }
    if (value == QStringLiteral("todos") || value == QStringLiteral("all")) {
        return QStringLiteral("all");
    }

    return value;
}

QString argument_target_id(const QStringList &arguments)
{
    for (int i = 1; i < arguments.size(); ++i) {
        const QString argument = arguments.at(i);
        if (argument == QStringLiteral("--all") || argument == QStringLiteral("-a")) {
            return QStringLiteral("all");
        }
        if ((argument == QStringLiteral("--module") || argument == QStringLiteral("-m") ||
             argument == QStringLiteral("--target")) &&
            i + 1 < arguments.size()) {
            return normalize_target_id(arguments.at(i + 1));
        }
        if (argument.startsWith(QStringLiteral("--module="))) {
            return normalize_target_id(argument.mid(QStringLiteral("--module=").size()));
        }
        if (argument.startsWith(QStringLiteral("--target="))) {
            return normalize_target_id(argument.mid(QStringLiteral("--target=").size()));
        }
    }

    return {};
}

QVector<mocks::translations::TranslationTarget> available_targets()
{
    return {
        {QStringLiteral("all"),
         QStringLiteral("Todos os Módulos (source/language/translations)"),
         {QStringLiteral("aclauditor_*.ts")},
         QStringLiteral("source/language/translations")},
        {QStringLiteral("aclauditor"),
         QStringLiteral("ACL Auditor (Principal)"),
         {QStringLiteral("aclauditor_*.ts")},
         QStringLiteral("source/language/translations")},
    };
}

QString find_project_root_from(const QString &start_path)
{
    QFileInfo info(start_path);
    QDir dir(info.isDir() ? info.absoluteFilePath() : info.absolutePath());

    while (true) {
        const bool has_project_files =
            dir.exists(QStringLiteral("CMakeLists.txt")) && dir.exists(QStringLiteral("mocks"));
        const bool has_translations = dir.exists(QStringLiteral("source/language/translations"));

        if (has_project_files && has_translations) {
            return dir.absolutePath();
        }

        if (!dir.cdUp()) {
            break;
        }
    }

    return {};
}

QString trim_process_output(const QString &text)
{
    const QString simplified = text.trimmed();
    return simplified.isEmpty() ? QStringLiteral("(sem saida)") : simplified;
}

QString clean_path_from_ui(const QString &path)
{
    const QString normalized = QDir::fromNativeSeparators(path.trimmed());
    return normalized.isEmpty() ? QString() : QDir::cleanPath(normalized);
}

QString display_path_or_status(const QString &path)
{
    return path.trimmed().isEmpty() ? QStringLiteral("Nao localizado") : QDir::toNativeSeparators(path);
}

QString mock_windows_icon_name()
{
#if defined(MOCKS_WINDOWS_ICON_NAME)
    return QStringLiteral(MOCKS_WINDOWS_ICON_NAME);
#else
    return QStringLiteral("acl_auditor.ico");
#endif
}

QString find_mock_icon_path(const QString &project_root, const QString &preferred_name = QString())
{
#if defined(Q_OS_WIN)
    const QString primary_icon_name = preferred_name.trimmed().isEmpty() ? mock_windows_icon_name() : preferred_name.trimmed();
    const QStringList candidate_names = {
        primary_icon_name,
        QStringLiteral("acl_auditor.ico"),
        QStringLiteral("acl_auditor.png"),
        QStringLiteral("munin.ico"),
        QStringLiteral("munin.png")
    };
    const QDir app_dir(QCoreApplication::applicationDirPath());
    const QDir current_dir(QDir::currentPath());
    const QDir source_dir(QFileInfo(QString::fromLocal8Bit(__FILE__)).absolutePath());

    for (const QString &name : candidate_names) {
        if (name.isEmpty()) {
            continue;
        }
        QStringList paths = {
            app_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            app_dir.filePath(name),
            current_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            current_dir.filePath(QStringLiteral("mocks/icons/%1").arg(name)),
            source_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            source_dir.filePath(QStringLiteral("../source/assets/icons/%1").arg(name)),
        };
        if (!project_root.isEmpty()) {
            paths.prepend(QDir(project_root).filePath(QStringLiteral("source/assets/icons/%1").arg(name)));
            paths.prepend(QDir(project_root).filePath(QStringLiteral("mocks/icons/%1").arg(name)));
        }

        for (const QString &candidate : paths) {
            if (QFileInfo::exists(candidate)) {
                return QFileInfo(candidate).absoluteFilePath();
            }
        }
    }
#else
    Q_UNUSED(project_root);
    Q_UNUSED(preferred_name);
#endif

    return {};
}

QString find_mock_taskbar_icon_path(const QString &project_root)
{
    return find_mock_icon_path(project_root);
}

void apply_mock_taskbar_icon(QWidget *window, const QString &project_root)
{
    const QString icon_path = find_mock_taskbar_icon_path(project_root);
    if (icon_path.isEmpty()) {
        return;
    }

    const QIcon icon(icon_path);
    QApplication::setWindowIcon(icon);
    if (window != nullptr) {
        window->setWindowIcon(icon);
    }
}

}  // namespace

namespace mocks::translations {

TranslationCompileDialog::TranslationCompileDialog(QWidget *parent, const QString &initial_target_id)
    : QDialog(parent),
      targets_(available_targets())
{
    project_root_ = locate_project_root();
    lrelease_executable_ = locate_lrelease_executable();
    setup_ui();
    populate_targets(initial_target_id);
    sync_path_fields_from_target();
    refresh_context_labels();
    apply_mock_taskbar_icon(this, project_root_);
}

void TranslationCompileDialog::setup_ui()
{
    setWindowTitle(QStringLiteral("Compilador de Traduções - ACL Auditor"));
    resize(900, 640);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    status_label_ = new QLabel(
        QStringLiteral("Pronto. Selecione o módulo e clique em Compilar Traduções."),
        this);
    status_label_->setWordWrap(true);
    layout->addWidget(status_label_);

    auto *target_label = new QLabel(QStringLiteral("Módulo selecionado:"), this);
    layout->addWidget(target_label);

    target_combo_ = new QComboBox(this);
    layout->addWidget(target_combo_);

    auto *source_label = new QLabel(QStringLiteral("Pasta de origem dos arquivos .ts:"), this);
    layout->addWidget(source_label);

    auto *source_row = new QHBoxLayout();
    source_dir_edit_ = new QLineEdit(this);
    source_dir_edit_->setPlaceholderText(QStringLiteral("Use o caminho padrão do módulo selecionado"));
    auto *browse_source_button = new QPushButton(QStringLiteral("Procurar..."), this);
    auto *reset_source_button = new QPushButton(QStringLiteral("Padrão"), this);
    source_row->addWidget(source_dir_edit_, 1);
    source_row->addWidget(browse_source_button);
    source_row->addWidget(reset_source_button);
    layout->addLayout(source_row);

    auto *output_label = new QLabel(QStringLiteral("Pasta de destino dos arquivos .qm:"), this);
    layout->addWidget(output_label);

    auto *output_row = new QHBoxLayout();
    output_dir_edit_ = new QLineEdit(this);
    output_dir_edit_->setPlaceholderText(QStringLiteral("Use a mesma pasta dos arquivos .ts"));
    auto *browse_output_button = new QPushButton(QStringLiteral("Escolher..."), this);
    auto *reset_output_button = new QPushButton(QStringLiteral("Mesmo local"), this);
    output_row->addWidget(output_dir_edit_, 1);
    output_row->addWidget(browse_output_button);
    output_row->addWidget(reset_output_button);
    layout->addLayout(output_row);

    translations_dir_label_ = new QLabel(this);
    translations_dir_label_->setWordWrap(true);
    layout->addWidget(translations_dir_label_);

    lrelease_label_ = new QLabel(this);
    lrelease_label_->setWordWrap(true);
    layout->addWidget(lrelease_label_);

    log_output_ = new QPlainTextEdit(this);
    log_output_->setReadOnly(true);
    log_output_->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(log_output_, 1);

    auto *buttons = new QDialogButtonBox(Qt::Horizontal, this);
    compile_button_ = buttons->addButton(QStringLiteral("Compilar Traduções"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);
    layout->addWidget(buttons);

    const QString compile_icon_path = find_mock_icon_path(project_root_, QStringLiteral("acl_auditor.png"));
    if (!compile_icon_path.isEmpty()) {
        const QIcon icon(compile_icon_path);
        compile_button_->setIcon(icon);
        browse_source_button->setIcon(icon);
        browse_output_button->setIcon(icon);
    }

    QObject::connect(compile_button_, &QPushButton::clicked, this, [this] { run_compilation(); });
    QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QObject::connect(target_combo_, &QComboBox::currentIndexChanged, this, [this] {
        sync_path_fields_from_target();
        refresh_context_labels();
    });
    QObject::connect(source_dir_edit_, &QLineEdit::textEdited, this, [this] {
        source_dir_custom_ = !clean_path_from_ui(source_dir_edit_->text()).isEmpty();
        if (!output_dir_custom_) {
            sync_path_fields_from_target();
        }
        refresh_context_labels();
    });
    QObject::connect(output_dir_edit_, &QLineEdit::textEdited, this, [this] {
        output_dir_custom_ = !clean_path_from_ui(output_dir_edit_->text()).isEmpty();
        refresh_context_labels();
    });
    QObject::connect(browse_source_button, &QPushButton::clicked, this, [this] { browse_source_directory(); });
    QObject::connect(browse_output_button, &QPushButton::clicked, this, [this] { browse_output_directory(); });
    QObject::connect(reset_source_button, &QPushButton::clicked, this, [this] { reset_source_directory(); });
    QObject::connect(reset_output_button, &QPushButton::clicked, this, [this] { reset_output_directory(); });
}

void TranslationCompileDialog::populate_targets(const QString &initial_target_id)
{
    const QSignalBlocker blocker(target_combo_);
    target_combo_->clear();

    const QString normalized_initial_id = normalize_target_id(initial_target_id);
    int selected_index = 0;

    for (int i = 0; i < targets_.size(); ++i) {
        const auto &target = targets_.at(i);
        target_combo_->addItem(target.display_name, target.id);
        if (!normalized_initial_id.isEmpty() && target.id == normalized_initial_id) {
            selected_index = i;
        }
    }

    target_combo_->setCurrentIndex(selected_index);
}

void TranslationCompileDialog::sync_path_fields_from_target()
{
    const QString default_dir = current_translations_directory();

    if (!source_dir_custom_) {
        const QSignalBlocker blocker(source_dir_edit_);
        source_dir_edit_->setText(default_dir.isEmpty() ? QString() : QDir::toNativeSeparators(default_dir));
    }

    if (!output_dir_custom_) {
        const QString base_source = effective_source_directory();
        const QString resolved_output = base_source.isEmpty() ? default_dir : base_source;
        const QSignalBlocker blocker(output_dir_edit_);
        output_dir_edit_->setText(resolved_output.isEmpty() ? QString() : QDir::toNativeSeparators(resolved_output));
    }
}

void TranslationCompileDialog::refresh_context_labels()
{
    const QString source_dir = effective_source_directory();
    const QString output_dir = effective_output_directory();

    translations_dir_label_->setText(
        QStringLiteral("Origem (.ts): %1\nDestino (.qm): %2")
            .arg(display_path_or_status(source_dir), display_path_or_status(output_dir)));

    lrelease_label_->setText(
        QStringLiteral("Ferramenta lrelease: %1").arg(display_path_or_status(lrelease_executable_)));
}

void TranslationCompileDialog::browse_source_directory()
{
    const QString current = effective_source_directory();
    const QString start_dir = current.isEmpty() ? project_root_ : current;
    const QString chosen = QFileDialog::getExistingDirectory(
        this,
        QStringLiteral("Selecione a pasta de origem dos arquivos .ts"),
        start_dir);

    if (chosen.isEmpty()) {
        return;
    }

    source_dir_custom_ = true;
    source_dir_edit_->setText(QDir::toNativeSeparators(chosen));

    if (!output_dir_custom_) {
        sync_path_fields_from_target();
    }

    refresh_context_labels();
}

void TranslationCompileDialog::browse_output_directory()
{
    const QString current = effective_output_directory();
    const QString start_dir = current.isEmpty() ? effective_source_directory() : current;
    const QString chosen = QFileDialog::getExistingDirectory(
        this,
        QStringLiteral("Selecione a pasta de destino dos arquivos .qm"),
        start_dir.isEmpty() ? project_root_ : start_dir);

    if (chosen.isEmpty()) {
        return;
    }

    output_dir_custom_ = true;
    output_dir_edit_->setText(QDir::toNativeSeparators(chosen));
    refresh_context_labels();
}

void TranslationCompileDialog::reset_source_directory()
{
    source_dir_custom_ = false;
    sync_path_fields_from_target();
    refresh_context_labels();
}

void TranslationCompileDialog::reset_output_directory()
{
    output_dir_custom_ = false;
    sync_path_fields_from_target();
    refresh_context_labels();
}

TranslationTarget TranslationCompileDialog::current_target() const
{
    const int index = target_combo_->currentIndex();
    if (index >= 0 && index < targets_.size()) {
        return targets_.at(index);
    }
    return {};
}

QString TranslationCompileDialog::current_translations_directory() const
{
    if (project_root_.isEmpty()) {
        return {};
    }

    const auto target = current_target();
    if (target.translations_relative_path.isEmpty()) {
        return {};
    }

    return QDir(project_root_).filePath(target.translations_relative_path);
}

QString TranslationCompileDialog::effective_source_directory() const
{
    const QString custom = clean_path_from_ui(source_dir_edit_->text());
    return custom.isEmpty() ? current_translations_directory() : custom;
}

QString TranslationCompileDialog::effective_output_directory() const
{
    const QString custom = clean_path_from_ui(output_dir_edit_->text());
    return custom.isEmpty() ? effective_source_directory() : custom;
}

QString TranslationCompileDialog::locate_project_root() const
{
    const QStringList candidates = {
        QCoreApplication::applicationDirPath(),
        QDir::currentPath(),
        QFileInfo(QString::fromLocal8Bit(__FILE__)).absolutePath(),
    };

    for (const QString &candidate : candidates) {
        const QString root = find_project_root_from(candidate);
        if (!root.isEmpty()) {
            return root;
        }
    }

    return {};
}

QString TranslationCompileDialog::locate_lrelease_executable() const
{
    const QString qt_bin = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    const QStringList candidates = {
        QDir(qt_bin).filePath(QStringLiteral("lrelease.exe")),
        QDir(qt_bin).filePath(QStringLiteral("lrelease")),
        QStandardPaths::findExecutable(QStringLiteral("lrelease.exe")),
        QStandardPaths::findExecutable(QStringLiteral("lrelease")),
        QStringLiteral("C:\\Qt\\6.11.1\\mingw_64\\bin\\lrelease.exe"),
        QStringLiteral("C:\\Qt\\6.11.1\\msvc2022_64\\bin\\lrelease.exe"),
    };

    for (const QString &candidate : candidates) {
        if (!candidate.isEmpty() && QFileInfo::exists(candidate)) {
            return QDir::toNativeSeparators(candidate);
        }
    }

    return {};
}

void TranslationCompileDialog::append_log(const QString &message)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
    log_output_->appendPlainText(QStringLiteral("[%1] %2").arg(timestamp, message));
    log_output_->moveCursor(QTextCursor::End);
}

TranslationCompileDialog::CompileSummary TranslationCompileDialog::compile_current_target(
    const QString &source_directory,
    const QString &output_directory)
{
    CompileSummary summary;
    const auto target = current_target();
    const QDir source_dir(source_directory);
    const QDir output_dir(output_directory);

    if (!output_dir.exists()) {
        if (!QDir().mkpath(output_directory)) {
            append_log(QStringLiteral("ERRO: Nao foi possivel criar a pasta de destino: %1")
                           .arg(QDir::toNativeSeparators(output_directory)));
            summary.failed_files = 1;
            return summary;
        }
    }

    const QStringList patterns = target.file_patterns.isEmpty()
                                     ? QStringList{QStringLiteral("*.ts")}
                                     : target.file_patterns;
    const QFileInfoList ts_files =
        source_dir.entryInfoList(patterns, QDir::Files, QDir::Name | QDir::IgnoreCase);

    summary.discovered_files = ts_files.size();
    if (ts_files.isEmpty()) {
        append_log(QStringLiteral("AVISO: Nenhum arquivo compatível com %1 em %2")
                       .arg(patterns.join(QStringLiteral(", ")), QDir::toNativeSeparators(source_directory)));
        return summary;
    }

    for (const QFileInfo &ts_file_info : ts_files) {
        const QString ts_path = ts_file_info.absoluteFilePath();
        const QString qm_path = output_dir.filePath(ts_file_info.completeBaseName() + QStringLiteral(".qm"));

        append_log(QStringLiteral("Compilando %1 -> %2...")
                       .arg(ts_file_info.fileName(), QFileInfo(qm_path).fileName()));

        QProcess process;
        process.setProgram(lrelease_executable_);
        process.setArguments({ts_path, QStringLiteral("-qm"), qm_path});
        process.start();

        if (!process.waitForStarted(5000)) {
            ++summary.failed_files;
            append_log(QStringLiteral("ERRO ao iniciar lrelease: %1").arg(process.errorString()));
            continue;
        }

        if (!process.waitForFinished(30000)) {
            ++summary.failed_files;
            process.kill();
            append_log(QStringLiteral("TEMPO LIMITE ao compilar %1").arg(ts_file_info.fileName()));
            continue;
        }

        const int exit_code = process.exitCode();
        const QString stdout_output =
            trim_process_output(QString::fromLocal8Bit(process.readAllStandardOutput()));
        const QString stderr_output =
            trim_process_output(QString::fromLocal8Bit(process.readAllStandardError()));

        if (exit_code != 0) {
            ++summary.failed_files;
            append_log(QStringLiteral("FALHA na compilação de %1 (código %2)")
                           .arg(ts_file_info.fileName(), QString::number(exit_code)));
            if (!stdout_output.isEmpty() && stdout_output != QStringLiteral("(sem saida)")) {
                append_log(QStringLiteral("stdout: %1").arg(stdout_output));
            }
            if (!stderr_output.isEmpty() && stderr_output != QStringLiteral("(sem saida)")) {
                append_log(QStringLiteral("stderr: %1").arg(stderr_output));
            }
            continue;
        }

        ++summary.succeeded_files;
        append_log(QStringLiteral("SUCESSO: %1 gerado.").arg(QFileInfo(qm_path).fileName()));
        if (!stdout_output.isEmpty() && stdout_output != QStringLiteral("(sem saida)")) {
            append_log(QStringLiteral("  %1").arg(stdout_output));
        }
    }

    return summary;
}

void TranslationCompileDialog::run_compilation()
{
    compile_button_->setEnabled(false);
    auto restore_compile_button = qScopeGuard([this] { compile_button_->setEnabled(true); });

    refresh_context_labels();

    const QString source_dir = effective_source_directory();
    const QString output_dir = effective_output_directory();

    if (source_dir.isEmpty() || !QDir(source_dir).exists()) {
        status_label_->setText(QStringLiteral("Erro: pasta de origem dos arquivos .ts inexistente."));
        append_log(QStringLiteral("ERRO: pasta de origem inexistente: %1")
                       .arg(display_path_or_status(source_dir)));
        return;
    }

    if (lrelease_executable_.isEmpty() || !QFileInfo::exists(lrelease_executable_)) {
        status_label_->setText(QStringLiteral("Erro: ferramenta lrelease.exe não encontrada."));
        append_log(QStringLiteral("ERRO: lrelease não encontrado."));
        return;
    }

    status_label_->setText(QStringLiteral("Compilando traduções..."));
    append_log(QStringLiteral("Iniciando compilação do módulo: %1").arg(current_target().display_name));
    append_log(QStringLiteral("Origem: %1").arg(QDir::toNativeSeparators(source_dir)));
    append_log(QStringLiteral("Destino: %1").arg(QDir::toNativeSeparators(output_dir)));

    const CompileSummary summary = compile_current_target(source_dir, output_dir);

    const QString summary_text =
        QStringLiteral("Arquivos encontrados: %1 | Sucesso: %2 | Falhas: %3")
            .arg(summary.discovered_files)
            .arg(summary.succeeded_files)
            .arg(summary.failed_files);

    if (summary.discovered_files == 0 && summary.failed_files == 0) {
        status_label_->setText(QStringLiteral("Nenhum arquivo .ts encontrado."));
        append_log(QStringLiteral("Compilação concluída: nenhum arquivo correspondente."));
        return;
    }

    if (summary.has_failures()) {
        status_label_->setText(QStringLiteral("Compilação concluída com falhas."));
        append_log(QStringLiteral("Compilação concluída com erros. %1").arg(summary_text));
        return;
    }

    status_label_->setText(QStringLiteral("Compilação concluída com sucesso."));
    append_log(QStringLiteral("Compilação concluída com sucesso. %1").arg(summary_text));
}

}  // namespace mocks::translations

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("CompileTranslationsGUI"));
    app.setApplicationDisplayName(QStringLiteral("Compilador de Traduções - ACL Auditor"));
    app.setApplicationVersion(QStringLiteral("2026.10.15.0"));

    const QString target_id = argument_target_id(app.arguments());
    mocks::translations::TranslationCompileDialog dialog(nullptr, target_id);
    return dialog.exec();
}
