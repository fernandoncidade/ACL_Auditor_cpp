#include "CompileTranslationsTerminal.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

QString normalize_target_id(QString value)
{
    value = value.trimmed().toLower();
    value.remove(QLatin1Char('_'));
    value.remove(QLatin1Char('-'));
    value.remove(QLatin1Char(' '));

    if (value.contains(QStringLiteral("aclauditor")) || value.contains(QStringLiteral("acl")) ||
        value.contains(QStringLiteral("auditor")) || value == QStringLiteral("host") || value == QStringLiteral("app")) {
        return QStringLiteral("aclauditor");
    }
    if (value == QStringLiteral("todos") || value == QStringLiteral("all")) {
        return QStringLiteral("all");
    }

    return value;
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

QString mock_windows_icon_name()
{
#if defined(MOCKS_WINDOWS_ICON_NAME)
    return QStringLiteral(MOCKS_WINDOWS_ICON_NAME);
#else
    return QStringLiteral("acl_auditor.ico");
#endif
}

QString find_mock_taskbar_icon_path()
{
#if defined(Q_OS_WIN)
    const QString primary_icon_name = mock_windows_icon_name();
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
    const QString project_root = find_project_root_from(app_dir.absolutePath());

    for (const QString &name : candidate_names) {
        if (name.isEmpty()) {
            continue;
        }
        QStringList candidates = {
            app_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            app_dir.filePath(name),
            current_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            current_dir.filePath(QStringLiteral("mocks/icons/%1").arg(name)),
            source_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            source_dir.filePath(QStringLiteral("../source/assets/icons/%1").arg(name)),
        };

        if (!project_root.isEmpty()) {
            candidates.prepend(QDir(project_root).filePath(QStringLiteral("source/assets/icons/%1").arg(name)));
            candidates.prepend(QDir(project_root).filePath(QStringLiteral("mocks/icons/%1").arg(name)));
        }

        for (const QString &candidate : candidates) {
            if (QFileInfo::exists(candidate)) {
                return QFileInfo(candidate).absoluteFilePath();
            }
        }
    }
#endif

    return {};
}

void apply_console_taskbar_icon()
{
#if defined(Q_OS_WIN)
    const QString icon_path = find_mock_taskbar_icon_path();
    if (icon_path.isEmpty()) {
        return;
    }

    HWND console_window = GetConsoleWindow();
    if (console_window == nullptr) {
        return;
    }

    const std::wstring native_path = QDir::toNativeSeparators(icon_path).toStdWString();
    HICON large_icon = static_cast<HICON>(LoadImageW(nullptr,
                                                     native_path.c_str(),
                                                     IMAGE_ICON,
                                                     GetSystemMetrics(SM_CXICON),
                                                     GetSystemMetrics(SM_CYICON),
                                                     LR_LOADFROMFILE));
    HICON small_icon = static_cast<HICON>(LoadImageW(nullptr,
                                                     native_path.c_str(),
                                                     IMAGE_ICON,
                                                     GetSystemMetrics(SM_CXSMICON),
                                                     GetSystemMetrics(SM_CYSMICON),
                                                     LR_LOADFROMFILE));

    if (large_icon != nullptr) {
        SendMessageW(console_window, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(large_icon));
    }
    if (small_icon != nullptr) {
        SendMessageW(console_window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small_icon));
    }
#endif
}

QString trim_process_output(const QString &text)
{
    const QString simplified = text.trimmed();
    return simplified.isEmpty() ? QStringLiteral("(sem saida)") : simplified;
}

}  // namespace

namespace mocks::translations {

int TranslationCompilerTerminal::run(const QStringList &arguments)
{
    apply_console_taskbar_icon();
    write_line(QStringLiteral("=== Compilador de Traduções Terminal - ACL Auditor ==="));

    const RunRequest request = parse_arguments(arguments);
    const QVector<TranslationTarget> targets = requested_targets(request);
    if (targets.isEmpty()) {
        write_error(QStringLiteral("Falha: módulo de tradução desconhecido."));
        return 1;
    }

    const QString project_root = locate_project_root();
    if (project_root.isEmpty()) {
        write_error(QStringLiteral("Falha: não foi possível localizar a raiz do projeto."));
        return 1;
    }

    const QString lrelease_executable = locate_lrelease_executable();
    if (lrelease_executable.isEmpty()) {
        write_error(QStringLiteral("Falha: não foi possível localizar o executável lrelease.exe."));
        return 1;
    }

    write_line(QStringLiteral("Raiz do projeto: %1").arg(QDir::toNativeSeparators(project_root)));
    write_line(QStringLiteral("Ferramenta lrelease: %1").arg(QDir::toNativeSeparators(lrelease_executable)));

    CompileSummary total;
    for (const TranslationTarget &target : targets) {
        const QString translations_dir = translations_directory(project_root, target);
        if (translations_dir.isEmpty() || !QDir(translations_dir).exists()) {
            ++total.failed_files;
            write_error(QStringLiteral("[%1] Diretório de traduções não encontrado: %2")
                            .arg(target.display_name, QDir::toNativeSeparators(translations_dir)));
            continue;
        }

        write_line(QStringLiteral("[%1] Diretório das traduções: %2")
                       .arg(target.display_name, QDir::toNativeSeparators(translations_dir)));

        const CompileSummary summary =
            compile_translation_files(target, translations_dir, lrelease_executable);
        total.discovered_files += summary.discovered_files;
        total.succeeded_files += summary.succeeded_files;
        total.failed_files += summary.failed_files;
    }

    const QString summary_text =
        QStringLiteral("Arquivos encontrados: %1 | Sucesso: %2 | Falhas: %3")
            .arg(total.discovered_files)
            .arg(total.succeeded_files)
            .arg(total.failed_files);

    if (total.discovered_files == 0 && total.failed_files == 0) {
        write_line(QStringLiteral("Nenhum arquivo .ts foi encontrado."));
        return 0;
    }

    if (total.failed_files > 0) {
        write_error(QStringLiteral("Compilação finalizada com falhas. %1").arg(summary_text));
        return 1;
    }

    write_line(QStringLiteral("Compilação finalizada com sucesso. %1").arg(summary_text));
    return 0;
}

TranslationCompilerTerminal::RunRequest TranslationCompilerTerminal::parse_arguments(const QStringList &arguments) const
{
    RunRequest request;
    for (int i = 1; i < arguments.size(); ++i) {
        const QString argument = arguments.at(i);
        if (argument == QStringLiteral("--all") || argument == QStringLiteral("-a")) {
            request.compile_all = true;
            request.requested_target_id.clear();
            continue;
        }
        if ((argument == QStringLiteral("--module") || argument == QStringLiteral("-m") ||
             argument == QStringLiteral("--target")) &&
            i + 1 < arguments.size()) {
            request.requested_target_id = normalize_target_id(arguments.at(++i));
            request.compile_all = request.requested_target_id == QStringLiteral("all");
            continue;
        }
        if (argument.startsWith(QStringLiteral("--module="))) {
            request.requested_target_id = normalize_target_id(argument.mid(QStringLiteral("--module=").size()));
            request.compile_all = request.requested_target_id == QStringLiteral("all");
            continue;
        }
        if (argument.startsWith(QStringLiteral("--target="))) {
            request.requested_target_id = normalize_target_id(argument.mid(QStringLiteral("--target=").size()));
            request.compile_all = request.requested_target_id == QStringLiteral("all");
        }
    }

    return request;
}

QVector<TranslationCompilerTerminal::TranslationTarget> TranslationCompilerTerminal::available_targets() const
{
    return {
        {QStringLiteral("aclauditor"),
         QStringLiteral("ACL Auditor (Principal)"),
         {QStringLiteral("aclauditor_*.ts")},
         QStringLiteral("source/language/translations")},
    };
}

QVector<TranslationCompilerTerminal::TranslationTarget> TranslationCompilerTerminal::requested_targets(
    const RunRequest &request) const
{
    const QVector<TranslationTarget> targets = available_targets();
    if (request.compile_all || request.requested_target_id.isEmpty() || request.requested_target_id == QStringLiteral("all")) {
        return targets;
    }

    QVector<TranslationTarget> result;
    for (const TranslationTarget &target : targets) {
        if (target.id == request.requested_target_id) {
            result.append(target);
            break;
        }
    }

    return result;
}

QString TranslationCompilerTerminal::locate_project_root() const
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

QString TranslationCompilerTerminal::locate_lrelease_executable() const
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

QString TranslationCompilerTerminal::translations_directory(const QString &project_root,
                                                            const TranslationTarget &target) const
{
    if (project_root.isEmpty()) {
        return {};
    }

    return QDir(project_root).filePath(target.translations_relative_path);
}

TranslationCompilerTerminal::CompileSummary TranslationCompilerTerminal::compile_translation_files(
    const TranslationTarget &target,
    const QString &translations_dir,
    const QString &lrelease_executable) const
{
    CompileSummary summary;
    const QDir dir(translations_dir);
    const QStringList patterns = target.file_patterns.isEmpty()
                                     ? QStringList{QStringLiteral("*.ts")}
                                     : target.file_patterns;
    const QFileInfoList ts_files =
        dir.entryInfoList(patterns, QDir::Files, QDir::Name | QDir::IgnoreCase);

    summary.discovered_files = ts_files.size();
    if (ts_files.isEmpty()) {
        write_line(QStringLiteral("[%1] Nenhum arquivo compatível com o padrão %2 foi localizado em %3.")
                       .arg(target.display_name,
                            patterns.join(QStringLiteral(", ")),
                            QDir::toNativeSeparators(translations_dir)));
        return summary;
    }

    for (const QFileInfo &ts_file_info : ts_files) {
        const QString ts_path = ts_file_info.absoluteFilePath();
        const QString qm_path = dir.filePath(ts_file_info.completeBaseName() + QStringLiteral(".qm"));

        write_line(QStringLiteral("[%1] Compilando %2...")
                       .arg(target.display_name, ts_file_info.fileName()));

        QProcess process;
        process.setProgram(lrelease_executable);
        process.setArguments({ts_path, QStringLiteral("-qm"), qm_path});
        process.start();

        if (!process.waitForStarted(5000)) {
            ++summary.failed_files;
            write_error(QStringLiteral("[%1] Erro ao iniciar lrelease: %2")
                            .arg(target.display_name, process.errorString()));
            continue;
        }

        if (!process.waitForFinished(30000)) {
            ++summary.failed_files;
            process.kill();
            write_error(QStringLiteral("[%1] Tempo limite esgotado ao compilar %2")
                            .arg(target.display_name, ts_file_info.fileName()));
            continue;
        }

        const int exit_code = process.exitCode();
        const QString stdout_output =
            trim_process_output(QString::fromLocal8Bit(process.readAllStandardOutput()));
        const QString stderr_output =
            trim_process_output(QString::fromLocal8Bit(process.readAllStandardError()));

        if (exit_code != 0) {
            ++summary.failed_files;
            write_error(QStringLiteral("[%1] Falha ao compilar %2 (código %3)")
                            .arg(target.display_name, ts_file_info.fileName(), QString::number(exit_code)));
            if (!stdout_output.isEmpty() && stdout_output != QStringLiteral("(sem saida)")) {
                write_line(QStringLiteral("  stdout: %1").arg(stdout_output));
            }
            if (!stderr_output.isEmpty() && stderr_output != QStringLiteral("(sem saida)")) {
                write_error(QStringLiteral("  stderr: %1").arg(stderr_output));
            }
            continue;
        }

        ++summary.succeeded_files;
        write_line(QStringLiteral("[%1] Gerado: %2")
                       .arg(target.display_name, QFileInfo(qm_path).fileName()));
        if (!stdout_output.isEmpty() && stdout_output != QStringLiteral("(sem saida)")) {
            write_line(QStringLiteral("  %1").arg(stdout_output));
        }
    }

    return summary;
}

void TranslationCompilerTerminal::write_line(const QString &message) const
{
    QTextStream(stdout) << message << Qt::endl;
}

void TranslationCompilerTerminal::write_error(const QString &message) const
{
    QTextStream(stderr) << message << Qt::endl;
}

}  // namespace mocks::translations

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("CompileTranslationsTerminal"));
    app.setApplicationVersion(QStringLiteral("2026.10.15.0"));

    mocks::translations::TranslationCompilerTerminal compiler;
    return compiler.run(app.arguments());
}
