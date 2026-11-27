cmake_minimum_required(VERSION 3.28)
project(ACLAuditor VERSION 2026.10.15.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
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

function(acl_resolve_qt_target_location TARGET_NAME OUT_VAR)
    set(${OUT_VAR} "" PARENT_SCOPE)
    if(NOT TARGET "${TARGET_NAME}")
        return()
    endif()
    foreach(_prop IN ITEMS
        IMPORTED_LOCATION_RELEASE
        IMPORTED_LOCATION_RELWITHDEBINFO
        IMPORTED_LOCATION
        IMPORTED_LOCATION_DEBUG
    )
        get_target_property(_loc "${TARGET_NAME}" "${_prop}")
        if(_loc AND NOT "${_loc}" STREQUAL "_loc-NOTFOUND")
            set(${OUT_VAR} "${_loc}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

if(BUILD_APP OR BUILD_TESTS OR BUILD_CLI)
    # ----------------------------------------------------------------------------
    # 1. Target Estático da Engine de Segurança e NTFS (C++23 / Win32 / Qt 6.12.0)
    # ----------------------------------------------------------------------------
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    find_package(Qt6 6.12 REQUIRED COMPONENTS Core Sql)
    if(BUILD_APP OR BUILD_TESTS)
        find_package(Qt6 6.12 REQUIRED COMPONENTS Gui Widgets PrintSupport)
        find_package(Qt6 6.12 QUIET COMPONENTS LinguistTools)
    endif()
    if(BUILD_TESTS)
        find_package(Qt6 6.12 REQUIRED COMPONENTS Test)
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
            "${CMAKE_CURRENT_SOURCE_DIR}/source/windows_app_icon.rc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/acl_auditor_app_icon.rc"
            @ONLY
        )
        set(ACL_AUDITOR_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/acl_auditor_app_icon.rc")
    else()
        set(ACL_AUDITOR_RC_FILE "")
    endif()

    acl_resolve_qt_target_location(Qt6::Core _ACL_QT_CORE_LOC)
    if(_ACL_QT_CORE_LOC)
        get_filename_component(_ACL_QT_CORE_DIR "${_ACL_QT_CORE_LOC}" DIRECTORY)
        get_filename_component(QT_PREFIX_DIR "${_ACL_QT_CORE_DIR}/.." ABSOLUTE)
    else()
        get_filename_component(QT_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
    endif()
    set(QT_BIN_DIR "${QT_PREFIX_DIR}/bin")
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")
    set(QTPATHS_EXECUTABLE "${QT_BIN_DIR}/qtpaths6.exe")
    if(NOT EXISTS "${QTPATHS_EXECUTABLE}")
        set(QTPATHS_EXECUTABLE "${QT_BIN_DIR}/qtpaths.exe")
    endif()
    if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    endif()
endif()

if(BUILD_APP OR BUILD_TESTS)
    # ----------------------------------------------------------------------------
    # 2. Compilação Automática de Traduções (.ts -> .qm)
    # ----------------------------------------------------------------------------
    acl_resolve_qt_target_location(Qt6::lrelease LRELEASE_EXECUTABLE)
    if(NOT LRELEASE_EXECUTABLE OR NOT EXISTS "${LRELEASE_EXECUTABLE}")
        set(LRELEASE_EXECUTABLE "${QT_BIN_DIR}/lrelease.exe")
    endif()
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

    # ----------------------------------------------------------------------------
    # 3. Biblioteca Estática da Camada de UI, Modelos, Utilitários e Idioma
    # ----------------------------------------------------------------------------
    add_library(ACLAuditorUI STATIC
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
    )

    target_link_libraries(ACLAuditorUI PUBLIC
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

    target_include_directories(ACLAuditorUI PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )

    add_dependencies(ACLAuditorUI ACLTranslations)
endif()

if(BUILD_APP)
    # ----------------------------------------------------------------------------
    # 4. Executável Gráfico Principal (ACL Auditor com Qt 6.12.0)
    # ----------------------------------------------------------------------------
    add_executable(ACLAuditor WIN32
        source/main.cpp
        ${ACL_AUDITOR_RC_FILE}
    )

    target_link_libraries(ACLAuditor PRIVATE
        ACLAuditorUI
    )

    add_dependencies(ACLAuditor ACLTranslations)

    set(_ACL_ASSET_DOC_FOLDERS
        ABOUT
        CLC
        COPYRIGHT
        EULA
        LICENSES
        NOTICES
        PRIVACY_POLICY
        RELEASE
    )
    set(_ACL_MINIMAL_APP_ICONS
        acl_auditor.ico
        acl_auditor.png
        acl_auditor_300-300.png
    )

    set(_ACL_COPY_ASSETS_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets/icons"
    )
    foreach(_doc_dir IN LISTS _ACL_ASSET_DOC_FOLDERS)
        list(APPEND _ACL_COPY_ASSETS_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/source/assets/${_doc_dir}"
                "${CMAKE_BINARY_DIR}/assets/${_doc_dir}"
        )
    endforeach()
    foreach(_ico_name IN LISTS _ACL_MINIMAL_APP_ICONS)
        list(APPEND _ACL_COPY_ASSETS_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/source/assets/icons/${_ico_name}"
                "${CMAKE_BINARY_DIR}/assets/icons/${_ico_name}"
        )
    endforeach()

    add_custom_command(TARGET ACLAuditor POST_BUILD
        ${_ACL_COPY_ASSETS_COMMANDS}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TRANSLATIONS_BIN_DIR}" "${CMAKE_BINARY_DIR}/translations"
        COMMENT "Copiando recursos minimos, icones e traducoes do ACL Auditor..."
    )

    set(_ACL_MINIMAL_PLUGINS "platforms/qwindows.dll,styles/qmodernwindowsstyle.dll,sqldrivers/qsqlite.dll,imageformats/qico.dll,imageformats/qjpeg.dll")
    set(_ACL_DEPLOY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy_optional_qt_binary.cmake")

    set(DEPLOY_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E echo "Deploying minimal pure Qt 6.12.0 dependencies for ACL Auditor..."
        COMMAND ${CMAKE_COMMAND}
            "-DWINDEPLOYQT_EXECUTABLE=${WINDEPLOYQT_EXECUTABLE}"
            "-DQTPATHS_EXECUTABLE=${QTPATHS_EXECUTABLE}"
            "-DTARGET_FILE=$<TARGET_FILE:ACLAuditor>"
            "-DRUNTIME_DIR=${CMAKE_BINARY_DIR}"
            "-DPLUGIN_DIR=${CMAKE_BINARY_DIR}/plugins"
            "-DTRANSLATION_DIR=${CMAKE_BINARY_DIR}/translations"
            "-DNO_COMPILER_RUNTIME=ON"
            "-DMINIMAL_DEPLOY=ON"
            "-DMINIMAL_PLUGINS=${_ACL_MINIMAL_PLUGINS}"
            "-DWRITE_QT_CONF=ON"
            -P "${_ACL_DEPLOY_SCRIPT}"
        ${_ACL_COPY_ASSETS_COMMANDS}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TRANSLATIONS_BIN_DIR}" "${CMAKE_BINARY_DIR}/translations"
    )

    if(MINGW)
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
    elseif(MSVC)
        set(_ACL_VC_CRT_DIR "")
        set(_ACL_CRT_CANDIDATES "")
        if(DEFINED ENV{VCToolsRedistDir} AND NOT "$ENV{VCToolsRedistDir}" STREQUAL "")
            file(TO_CMAKE_PATH "$ENV{VCToolsRedistDir}/x64/Microsoft.VC145.CRT" _env_crt_dir)
            list(APPEND _ACL_CRT_CANDIDATES "${_env_crt_dir}")
        endif()
        list(APPEND _ACL_CRT_CANDIDATES
            "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/VC/Redist/MSVC/14.52.37025/x64/Microsoft.VC145.CRT"
            "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Redist/MSVC/14.52.37025/x64/Microsoft.VC145.CRT"
        )
        foreach(_crt_cand IN LISTS _ACL_CRT_CANDIDATES)
            if(EXISTS "${_crt_cand}/msvcp140.dll")
                set(_ACL_VC_CRT_DIR "${_crt_cand}")
                break()
            endif()
        endforeach()
        if(NOT _ACL_VC_CRT_DIR)
            file(GLOB _glob_crt_dirs
                "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/VC/Redist/MSVC/14.52.*/x64/Microsoft.VC145.CRT"
                "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Redist/MSVC/14.52.*/x64/Microsoft.VC145.CRT"
            )
            foreach(_g IN LISTS _glob_crt_dirs)
                if(EXISTS "${_g}/msvcp140.dll")
                    set(_ACL_VC_CRT_DIR "${_g}")
                    break()
                endif()
            endforeach()
        endif()
        if(_ACL_VC_CRT_DIR)
            foreach(VC_DLL IN ITEMS msvcp140.dll msvcp140_1.dll msvcp140_2.dll vcruntime140.dll vcruntime140_1.dll)
                if(EXISTS "${_ACL_VC_CRT_DIR}/${VC_DLL}")
                    list(APPEND DEPLOY_COMMANDS
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${_ACL_VC_CRT_DIR}/${VC_DLL}"
                            "${CMAKE_BINARY_DIR}/${VC_DLL}"
                    )
                endif()
            endforeach()
        else()
            message(WARNING "Microsoft.VC145.CRT (14.52.37025) nao encontrado para deploy local MSVC.")
        endif()
    endif()

    add_custom_target(ACLAuditorDeploy
        ${DEPLOY_COMMANDS}
        DEPENDS ACLAuditor
        COMMENT "Deploying minimal pure dependencies for ACL Auditor"
        VERBATIM
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

if(BUILD_TESTS)
    # ----------------------------------------------------------------------------
    # 6. Suíte Completa de Testes (Qt6Test + CTest em build/build_suite_{mingw,msvc})
    # ----------------------------------------------------------------------------
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()

# ----------------------------------------------------------------------------
# 7. Mocks e Utilitários (Build Isolada)
# ----------------------------------------------------------------------------
if(BUILD_MOCKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mocks/CMakeLists.txt")
    add_subdirectory(mocks)
endif()
