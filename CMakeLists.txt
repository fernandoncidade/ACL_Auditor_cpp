cmake_minimum_required(VERSION 3.21)
project(ACLAuditor VERSION 2026.10.15.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_APP "Compilar aplicativo principal ACL Auditor" ON)
option(BUILD_TESTS "Compilar suite de testes ACL Auditor" ON)
option(BUILD_CLI "Compilar utilitario de linha de comando acl-auditor-cli" ON)
option(BUILD_MOCKS "Compilar utilitarios e mocks do projeto" OFF)

# Optimization flags
if(MSVC)
    add_compile_options(/O2 /Oi /Ot /Gy /W3 /permissive- /Zc:__cplusplus /utf-8)
    add_compile_options("$<$<CONFIG:Release>:/GL>")
    add_link_options("$<$<CONFIG:Release>:/LTCG>")
else()
    add_compile_options(-O3 -Wall -Wextra)
endif()

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

if(BUILD_APP OR BUILD_TESTS OR BUILD_CLI)
    # ----------------------------------------------------------------------------
    # 1. Target Estático da Engine de Segurança e NTFS (C++20 / Win32)
    # ----------------------------------------------------------------------------
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Sql)
    if(BUILD_APP)
        find_package(Qt6 REQUIRED COMPONENTS Gui Widgets PrintSupport)
    endif()

    add_library(ACLAuditorEngine STATIC
        source/engine/AclTypes.hpp
        source/engine/NtfsSecurityUtils.hpp
        source/engine/NtfsSecurityUtils.cpp
        source/engine/EffectiveAccessCalculator.hpp
        source/engine/EffectiveAccessCalculator.cpp
        source/engine/RiskScanner.hpp
        source/engine/RiskScanner.cpp
        source/engine/ServiceAuditor.hpp
        source/engine/ServiceAuditor.cpp
        source/engine/SmbShareAuditor.hpp
        source/engine/SmbShareAuditor.cpp
        source/engine/SnapshotEngine.hpp
        source/engine/SnapshotEngine.cpp
        source/engine/RemediationPlanner.hpp
        source/engine/RemediationPlanner.cpp
    )

    target_include_directories(ACLAuditorEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )

    target_link_libraries(ACLAuditorEngine PUBLIC
        Qt6::Core
        Qt6::Sql
        advapi32
        authz
        netapi32
        bcrypt
        shlwapi
    )

    if(WIN32)
        enable_language(RC)
        set(ACL_AUDITOR_APP_ICON_RC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/assets/icons/acl_auditor.ico")
        file(TO_NATIVE_PATH "${ACL_AUDITOR_APP_ICON_RC_PATH}" ACL_AUDITOR_APP_ICON_RC_PATH_NATIVE)
        string(REPLACE "\\" "\\\\" ACL_AUDITOR_APP_ICON_RC_PATH_ESCAPED "${ACL_AUDITOR_APP_ICON_RC_PATH_NATIVE}")
        set(ACL_AUDITOR_APP_ICON_RC_PATH "${ACL_AUDITOR_APP_ICON_RC_PATH_ESCAPED}")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_app_icon.rc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/acl_auditor_app_icon.rc"
            @ONLY
        )
        set(ACL_AUDITOR_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/acl_auditor_app_icon.rc")
    else()
        set(ACL_AUDITOR_RC_FILE "")
    endif()

    get_filename_component(QT_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
    set(QT_BIN_DIR "${QT_PREFIX_DIR}/bin")
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")
endif()

if(BUILD_APP)
    # ----------------------------------------------------------------------------
    # 2. Executável Gráfico Principal (ACL Auditor com Qt6)
    # ----------------------------------------------------------------------------
    add_executable(ACLAuditor WIN32
        source/model/AclTreeModel.hpp
        source/model/AclTreeModel.cpp
        source/model/AceTableModel.hpp
        source/model/AceTableModel.cpp
        source/model/RiskTableModel.hpp
        source/model/RiskTableModel.cpp
        source/model/DiffTableModel.hpp
        source/model/DiffTableModel.cpp
        source/utils/LogManager.hpp
        source/utils/LogManager.cpp
        source/utils/ApplicationPathUtils.hpp
        source/utils/ApplicationPathUtils.cpp
        source/utils/ExplorerIntegration.hpp
        source/utils/ExplorerIntegration.cpp
        source/language/GerenciadorTraducao.hpp
        source/language/GerenciadorTraducao.cpp
        source/ui/Win32DwmHelper.hpp
        source/ui/Win32DwmHelper.cpp
        source/ui/ThemeManager.hpp
        source/ui/ThemeManager.cpp
        source/ui/TableEnhancer.hpp
        source/ui/TableEnhancer.cpp
        source/ui/VisualExplanationWidget.hpp
        source/ui/VisualExplanationWidget.cpp
        source/ui/RiskDashboardWidget.hpp
        source/ui/RiskDashboardWidget.cpp
        source/ui/ServicesSharesWidget.hpp
        source/ui/ServicesSharesWidget.cpp
        source/ui/SnapshotDiffWidget.hpp
        source/ui/SnapshotDiffWidget.cpp
        source/ui/ReportExportDialog.hpp
        source/ui/ReportExportDialog.cpp
        source/ui/RemediationDialog.hpp
        source/ui/RemediationDialog.cpp
        source/ui/AboutDialog.hpp
        source/ui/AboutDialog.cpp
        source/ui/ManualDialog.hpp
        source/ui/ManualDialog.cpp
        source/ui/MainWindow.hpp
        source/ui/MainWindow.cpp
        source/main.cpp
        ${ACL_AUDITOR_RC_FILE}
    )

    target_link_libraries(ACLAuditor PRIVATE
        ACLAuditorEngine
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
        Qt6::PrintSupport
        advapi32
        authz
        netapi32
        bcrypt
        dwmapi
        psapi
        shlwapi
    )

    target_include_directories(ACLAuditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )

    # ----------------------------------------------------------------------------
    # Compilação Automática de Traduções (.ts -> .qm)
    # ----------------------------------------------------------------------------
    set(LRELEASE_EXECUTABLE "${QT_BIN_DIR}/lrelease.exe")
    set(TRANSLATIONS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations")
    set(TRANSLATIONS_BIN_DIR "${CMAKE_BINARY_DIR}/translations")

    add_custom_target(ACLTranslations ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TRANSLATIONS_BIN_DIR}"
        COMMAND "${LRELEASE_EXECUTABLE}" "${TRANSLATIONS_SRC_DIR}/aclauditor_pt_BR.ts" -qm "${TRANSLATIONS_BIN_DIR}/aclauditor_pt_BR.qm"
        COMMAND "${LRELEASE_EXECUTABLE}" "${TRANSLATIONS_SRC_DIR}/aclauditor_en_US.ts" -qm "${TRANSLATIONS_BIN_DIR}/aclauditor_en_US.qm"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TRANSLATIONS_BIN_DIR}/aclauditor_pt_BR.qm" "${TRANSLATIONS_SRC_DIR}/aclauditor_pt_BR.qm"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TRANSLATIONS_BIN_DIR}/aclauditor_en_US.qm" "${TRANSLATIONS_SRC_DIR}/aclauditor_en_US.qm"
        COMMENT "Compilando arquivos de tradução (.ts -> .qm) com lrelease..."
    )
    add_dependencies(ACLAuditor ACLTranslations)

    add_custom_command(TARGET ACLAuditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/source/assets" "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TRANSLATIONS_BIN_DIR}" "${CMAKE_BINARY_DIR}/translations"
        COMMENT "Copiando recursos, icones e traduções do ACL Auditor..."
    )

    set(DEPLOY_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E echo "Deploying Qt dependencies, styles, platforms, and assets for ACL Auditor..."
        COMMAND ${WINDEPLOYQT_EXECUTABLE} "$<TARGET_FILE:ACLAuditor>" --dir "${CMAKE_BINARY_DIR}" --plugindir "${CMAKE_BINARY_DIR}/plugins" --add-plugin-types styles,iconengines,imageformats,platforms,sqldrivers --no-translations --no-compiler-runtime
        COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/vc_redist.x64.exe" "${CMAKE_BINARY_DIR}/dxcompiler.dll" "${CMAKE_BINARY_DIR}/dxil.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PREFIX_DIR}/plugins/platforms/qwindows.dll" "${CMAKE_BINARY_DIR}/plugins/platforms/qwindows.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins/styles"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PREFIX_DIR}/plugins/styles/qmodernwindowsstyle.dll" "${CMAKE_BINARY_DIR}/plugins/styles/qmodernwindowsstyle.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins/sqldrivers"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PREFIX_DIR}/plugins/sqldrivers/qsqlite.dll" "${CMAKE_BINARY_DIR}/plugins/sqldrivers/qsqlite.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/source/assets" "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TRANSLATIONS_BIN_DIR}" "${CMAKE_BINARY_DIR}/translations"
    )

    if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(GCC_RUNTIMES libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        foreach(GCC_DLL IN LISTS GCC_RUNTIMES)
            if(EXISTS "${MINGW_BIN_DIR}/${GCC_DLL}")
                list(APPEND DEPLOY_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/${GCC_DLL}"
                        "${CMAKE_BINARY_DIR}/${GCC_DLL}"
                )
            elseif(EXISTS "${QT_BIN_DIR}/${GCC_DLL}")
                list(APPEND DEPLOY_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${QT_BIN_DIR}/${GCC_DLL}"
                        "${CMAKE_BINARY_DIR}/${GCC_DLL}"
                )
            endif()
        endforeach()
    endif()

    add_custom_target(ACLAuditorDeploy
        ${DEPLOY_COMMANDS}
        DEPENDS ACLAuditor
        COMMENT "Deploying dependencies for ACL Auditor"
    )
endif()

if(BUILD_TESTS)
    # ----------------------------------------------------------------------------
    # 3. Executável de Testes e Validação da Engine
    # ----------------------------------------------------------------------------
    add_executable(ACLAuditorTest
        mocks/teste.cpp
        ${ACL_AUDITOR_RC_FILE}
    )

    target_link_libraries(ACLAuditorTest PRIVATE
        ACLAuditorEngine
        Qt6::Core
        Qt6::Sql
        advapi32
        authz
        netapi32
        bcrypt
        psapi
        shlwapi
    )

    target_include_directories(ACLAuditorTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    )

    # ----------------------------------------------------------------------------
    # 4. Deployment para Suíte de Testes (ACLAuditorTestDeploy)
    # ----------------------------------------------------------------------------
    set(TEST_DEPLOY_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E echo "Deploying Qt dependencies and plugins for ACL Auditor Test..."
        COMMAND ${WINDEPLOYQT_EXECUTABLE} "$<TARGET_FILE:ACLAuditorTest>" --dir "${CMAKE_BINARY_DIR}" --plugindir "${CMAKE_BINARY_DIR}/plugins" --add-plugin-types sqldrivers --no-translations --no-compiler-runtime
        COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/vc_redist.x64.exe" "${CMAKE_BINARY_DIR}/dxcompiler.dll" "${CMAKE_BINARY_DIR}/dxil.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins/sqldrivers"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PREFIX_DIR}/plugins/sqldrivers/qsqlite.dll" "${CMAKE_BINARY_DIR}/plugins/sqldrivers/qsqlite.dll"
    )

    if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(GCC_RUNTIMES libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        foreach(GCC_DLL IN LISTS GCC_RUNTIMES)
            if(EXISTS "${MINGW_BIN_DIR}/${GCC_DLL}")
                list(APPEND TEST_DEPLOY_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/${GCC_DLL}"
                        "${CMAKE_BINARY_DIR}/${GCC_DLL}"
                )
            elseif(EXISTS "${QT_BIN_DIR}/${GCC_DLL}")
                list(APPEND TEST_DEPLOY_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${QT_BIN_DIR}/${GCC_DLL}"
                        "${CMAKE_BINARY_DIR}/${GCC_DLL}"
                )
            endif()
        endforeach()
    endif()

    add_custom_target(ACLAuditorTestDeploy
        ${TEST_DEPLOY_COMMANDS}
        DEPENDS ACLAuditorTest
        COMMENT "Deploying dependencies for ACL Auditor Test Suite"
    )
endif()

if(BUILD_CLI)
    # ----------------------------------------------------------------------------
    # 5. Executável Standalone de Linha de Comando (acl-auditor-cli)
    # ----------------------------------------------------------------------------
    add_executable(acl-auditor-cli
        source/cli/main_cli.cpp
        ${ACL_AUDITOR_RC_FILE}
    )

    target_link_libraries(acl-auditor-cli PRIVATE
        ACLAuditorEngine
        Qt6::Core
        Qt6::Sql
        advapi32
        authz
        netapi32
        bcrypt
        psapi
        shlwapi
    )

    target_include_directories(acl-auditor-cli PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )

    if(TARGET ACLAuditorDeploy)
        add_dependencies(ACLAuditorDeploy acl-auditor-cli)
    endif()
endif()

# ----------------------------------------------------------------------------
# 6. Mocks e Utilitários (Build Isolada)
# ----------------------------------------------------------------------------
if(BUILD_MOCKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mocks/CMakeLists.txt")
    add_subdirectory(mocks)
endif()
