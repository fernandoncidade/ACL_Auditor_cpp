cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(ACLAuditor_Mocks LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 6.12 REQUIRED COMPONENTS Core Gui Widgets Sql)

if(WIN32)
    enable_language(RC)
endif()

find_package(Qt6 6.12 QUIET COMPONENTS LinguistTools)

function(mocks_resolve_qt_target_location TARGET_NAME OUT_VAR)
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

if(WIN32)
    if(MSVC)
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "msvc")
    elseif(MINGW)
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "mingw")
    elseif(DEFINED ACL_AUDITOR_BUILD_TOOLCHAIN_SUFFIX)
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "${ACL_AUDITOR_BUILD_TOOLCHAIN_SUFFIX}")
    else()
        set(MOCKS_DETECTED_TOOLCHAIN_SUFFIX "windows")
    endif()

    set(MOCKS_DEFAULT_EXPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/build_Mocks_${MOCKS_DETECTED_TOOLCHAIN_SUFFIX}/export")
else()
    set(MOCKS_DEFAULT_EXPORT_DIR "")
endif()

if(WIN32)
    set(MOCKS_WINDOWS_ICON_NAME "acl_auditor.ico")
    set(MOCKS_WINDOWS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icons/${MOCKS_WINDOWS_ICON_NAME}")
    if(NOT EXISTS "${MOCKS_WINDOWS_ICON}")
        set(MOCKS_WINDOWS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../source/assets/icons/${MOCKS_WINDOWS_ICON_NAME}")
    endif()
endif()

set(
    MOCKS_EXPORT_DIR
    "${MOCKS_DEFAULT_EXPORT_DIR}"
    CACHE PATH
    "Diretorio que recebe o runtime implantado dos mocks"
)

if(MOCKS_EXPORT_DIR)
    file(TO_CMAKE_PATH "${MOCKS_EXPORT_DIR}" MOCKS_EXPORT_DIR_NORMALIZED)
else()
    set(MOCKS_EXPORT_DIR_NORMALIZED "")
endif()

set(MOCKS_MINIMAL_ICON_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/icons/acl_auditor.ico"
    "${CMAKE_CURRENT_SOURCE_DIR}/icons/acl_auditor.png"
)

function(mocks_add_windows_icon TARGET_NAME)
    if(NOT WIN32)
        return()
    endif()

    if(NOT EXISTS "${MOCKS_WINDOWS_ICON}")
        message(WARNING "Icone do mock nao encontrado em ${MOCKS_WINDOWS_ICON}. ${TARGET_NAME} sera compilado sem icone.")
        return()
    endif()

    file(TO_NATIVE_PATH "${MOCKS_WINDOWS_ICON}" ACL_AUDITOR_APP_ICON_RC_PATH_NATIVE)
    string(REPLACE "\\" "\\\\" ACL_AUDITOR_APP_ICON_RC_PATH "${ACL_AUDITOR_APP_ICON_RC_PATH_NATIVE}")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../source/windows_app_icon.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_app_icon.rc"
        @ONLY
    )
    target_sources(
        "${TARGET_NAME}"
        PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_app_icon.rc"
    )

    target_compile_definitions(
        "${TARGET_NAME}"
        PRIVATE
            "MOCKS_WINDOWS_ICON_NAME=\"${MOCKS_WINDOWS_ICON_NAME}\""
    )

    set(_icon_copy_cmds
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/icons"
    )
    foreach(_ico IN LISTS MOCKS_MINIMAL_ICON_FILES)
        if(EXISTS "${_ico}")
            get_filename_component(_ico_name "${_ico}" NAME)
            list(APPEND _icon_copy_cmds
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_ico}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>/icons/${_ico_name}"
            )
        endif()
    endforeach()

    add_custom_command(
        TARGET "${TARGET_NAME}"
        POST_BUILD
        ${_icon_copy_cmds}
        VERBATIM
    )
endfunction()

function(mocks_apply_common_options TARGET_NAME)
    set_target_properties(
        "${TARGET_NAME}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    target_include_directories(
        "${TARGET_NAME}"
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )

    target_compile_definitions(
        "${TARGET_NAME}"
        PRIVATE
            $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:Release>:QT_NO_INFO_OUTPUT>
            $<$<CONFIG:RelWithDebInfo>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:RelWithDebInfo>:QT_NO_INFO_OUTPUT>
    )

    if(MSVC)
        set_property(TARGET "${TARGET_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        target_compile_options(
            "${TARGET_NAME}"
            PRIVATE
                $<$<CONFIG:Release>:/O2>
                $<$<CONFIG:Release>:/Oi>
                $<$<CONFIG:Release>:/Ot>
                $<$<CONFIG:Release>:/fp:fast>
                $<$<CONFIG:RelWithDebInfo>:/O2>
                $<$<CONFIG:RelWithDebInfo>:/Oi>
                $<$<CONFIG:RelWithDebInfo>:/Ot>
                $<$<CONFIG:RelWithDebInfo>:/fp:fast>
        )
    elseif(MINGW)
        target_compile_options(
            "${TARGET_NAME}"
            PRIVATE
                $<$<CONFIG:Release>:-O3>
                $<$<CONFIG:Release>:-ffunction-sections>
                $<$<CONFIG:Release>:-fdata-sections>
                $<$<CONFIG:Release>:-fomit-frame-pointer>
                $<$<CONFIG:RelWithDebInfo>:-O3>
                $<$<CONFIG:RelWithDebInfo>:-ffunction-sections>
                $<$<CONFIG:RelWithDebInfo>:-fdata-sections>
                $<$<CONFIG:RelWithDebInfo>:-fomit-frame-pointer>
        )
        target_link_options(
            "${TARGET_NAME}"
            PRIVATE
                $<$<CONFIG:Release>:-Wl,--gc-sections>
                $<$<CONFIG:Release>:-Wl,-O1>
                $<$<CONFIG:RelWithDebInfo>:-Wl,--gc-sections>
                $<$<CONFIG:RelWithDebInfo>:-Wl,-O1>
        )
    endif()
endfunction()

function(_mocks_collect_compiler_runtime_files OUT_LIST)
    set(_files "")
    if(MINGW)
        get_filename_component(_mingw_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(_dll IN ITEMS libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
            if(EXISTS "${_mingw_bin}/${_dll}")
                list(APPEND _files "${_mingw_bin}/${_dll}")
            endif()
        endforeach()
    elseif(MSVC)
        set(_crt_dir "")
        set(_cand_crt_dirs "")
        if(DEFINED ENV{VCToolsRedistDir} AND NOT "$ENV{VCToolsRedistDir}" STREQUAL "")
            file(TO_CMAKE_PATH "$ENV{VCToolsRedistDir}/x64/Microsoft.VC145.CRT" _env_crt)
            list(APPEND _cand_crt_dirs "${_env_crt}")
        endif()
        list(APPEND _cand_crt_dirs
            "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/VC/Redist/MSVC/14.52.37025/x64/Microsoft.VC145.CRT"
            "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Redist/MSVC/14.52.37025/x64/Microsoft.VC145.CRT"
        )
        foreach(_c IN LISTS _cand_crt_dirs)
            if(EXISTS "${_c}/msvcp140.dll")
                set(_crt_dir "${_c}")
                break()
            endif()
        endforeach()
        if(NOT _crt_dir)
            file(GLOB _glob_crt
                "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/VC/Redist/MSVC/14.52.*/x64/Microsoft.VC145.CRT"
                "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Redist/MSVC/14.52.*/x64/Microsoft.VC145.CRT"
            )
            foreach(_g IN LISTS _glob_crt)
                if(EXISTS "${_g}/msvcp140.dll")
                    set(_crt_dir "${_g}")
                    break()
                endif()
            endforeach()
        endif()
        if(_crt_dir)
            foreach(_dll IN ITEMS msvcp140.dll msvcp140_1.dll msvcp140_2.dll vcruntime140.dll vcruntime140_1.dll)
                if(EXISTS "${_crt_dir}/${_dll}")
                    list(APPEND _files "${_crt_dir}/${_dll}")
                endif()
            endforeach()
        endif()
    endif()
    set(${OUT_LIST} "${_files}" PARENT_SCOPE)
endfunction()

function(mocks_add_deploy_target TARGET_NAME)
    if(NOT WIN32)
        return()
    endif()

    mocks_resolve_qt_target_location(Qt6::Core _qt6_core_loc)
    if(_qt6_core_loc)
        get_filename_component(_qt6_core_dir "${_qt6_core_loc}" DIRECTORY)
        get_filename_component(MOCKS_QT_PREFIX_DIR "${_qt6_core_dir}/.." ABSOLUTE)
    else()
        get_filename_component(MOCKS_QT_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
    endif()

    set(MOCKS_WINDEPLOYQT_EXECUTABLE "${MOCKS_QT_PREFIX_DIR}/bin/windeployqt.exe")
    set(MOCKS_QTPATHS_EXECUTABLE "${MOCKS_QT_PREFIX_DIR}/bin/qtpaths6.exe")
    if(NOT EXISTS "${MOCKS_WINDEPLOYQT_EXECUTABLE}" OR NOT EXISTS "${MOCKS_QTPATHS_EXECUTABLE}")
        message(WARNING "windeployqt.exe ou qtpaths6.exe nao encontrado. ${TARGET_NAME} nao sera implantado automaticamente.")
        return()
    endif()

    set(MOCKS_DEPLOY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/deploy_optional_qt_binary.cmake")
    set(_mocks_minimal_plugins "platforms/qwindows.dll,styles/qmodernwindowsstyle.dll,imageformats/qico.dll")
    _mocks_collect_compiler_runtime_files(_runtime_dll_files)

    set(DEPLOY_COMMANDS
        COMMAND "${CMAKE_COMMAND}"
            "-DWINDEPLOYQT_EXECUTABLE=${MOCKS_WINDEPLOYQT_EXECUTABLE}"
            "-DQTPATHS_EXECUTABLE=${MOCKS_QTPATHS_EXECUTABLE}"
            "-DTARGET_FILE=$<TARGET_FILE:${TARGET_NAME}>"
            "-DRUNTIME_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>"
            "-DPLUGIN_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>/plugins"
            "-DTRANSLATION_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>/translations"
            "-DNO_COMPILER_RUNTIME=ON"
            "-DMINIMAL_DEPLOY=ON"
            "-DMINIMAL_PLUGINS=${_mocks_minimal_plugins}"
            "-DWRITE_QT_CONF=ON"
            -P "${MOCKS_DEPLOY_SCRIPT}"
    )

    foreach(_rdll IN LISTS _runtime_dll_files)
        get_filename_component(_rdll_name "${_rdll}" NAME)
        list(APPEND DEPLOY_COMMANDS
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_rdll}"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/${_rdll_name}"
        )
    endforeach()

    if(MOCKS_EXPORT_DIR_NORMALIZED)
        list(APPEND DEPLOY_COMMANDS
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${MOCKS_EXPORT_DIR_NORMALIZED}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${MOCKS_EXPORT_DIR_NORMALIZED}/icons"
        )
        foreach(_ico IN LISTS MOCKS_MINIMAL_ICON_FILES)
            if(EXISTS "${_ico}")
                get_filename_component(_ico_name "${_ico}" NAME)
                list(APPEND DEPLOY_COMMANDS
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                        "${_ico}"
                        "${MOCKS_EXPORT_DIR_NORMALIZED}/icons/${_ico_name}"
                )
            endif()
        endforeach()

        list(APPEND DEPLOY_COMMANDS
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:${TARGET_NAME}>"
                "${MOCKS_EXPORT_DIR_NORMALIZED}/"
            COMMAND "${CMAKE_COMMAND}"
                "-DWINDEPLOYQT_EXECUTABLE=${MOCKS_WINDEPLOYQT_EXECUTABLE}"
                "-DQTPATHS_EXECUTABLE=${MOCKS_QTPATHS_EXECUTABLE}"
                "-DTARGET_FILE=${MOCKS_EXPORT_DIR_NORMALIZED}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
                "-DRUNTIME_DIR=${MOCKS_EXPORT_DIR_NORMALIZED}"
                "-DPLUGIN_DIR=${MOCKS_EXPORT_DIR_NORMALIZED}/plugins"
                "-DTRANSLATION_DIR=${MOCKS_EXPORT_DIR_NORMALIZED}/translations"
                "-DNO_COMPILER_RUNTIME=ON"
                "-DMINIMAL_DEPLOY=ON"
                "-DMINIMAL_PLUGINS=${_mocks_minimal_plugins}"
                "-DWRITE_QT_CONF=ON"
                -P "${MOCKS_DEPLOY_SCRIPT}"
        )

        foreach(_rdll IN LISTS _runtime_dll_files)
            get_filename_component(_rdll_name "${_rdll}" NAME)
            list(APPEND DEPLOY_COMMANDS
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_rdll}"
                    "${MOCKS_EXPORT_DIR_NORMALIZED}/${_rdll_name}"
            )
        endforeach()
    endif()

    add_custom_target(
        "${TARGET_NAME}Deploy"
        ALL
        DEPENDS "${TARGET_NAME}"
        ${DEPLOY_COMMANDS}
        COMMENT "Deploying ${TARGET_NAME} minimal pure runtime"
        VERBATIM
    )

    if(MOCKS_LAST_DEPLOY_TARGET)
        add_dependencies("${TARGET_NAME}Deploy" "${MOCKS_LAST_DEPLOY_TARGET}")
    endif()
    set(MOCKS_LAST_DEPLOY_TARGET "${TARGET_NAME}Deploy" PARENT_SCOPE)
endfunction()

function(mocks_add_gui_run_target TARGET_NAME TOOL_TARGET)
    if(NOT WIN32)
        return()
    endif()

    add_custom_target(
        "${TARGET_NAME}"
        DEPENDS "${TOOL_TARGET}Deploy"
        COMMAND "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>/plugins"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>/plugins/platforms"
            cmd /c start "" /b "$<TARGET_FILE:${TOOL_TARGET}>" ${ARGN}
        VERBATIM
        USES_TERMINAL
    )
endfunction()

function(mocks_add_terminal_run_target TARGET_NAME TOOL_TARGET)
    if(NOT WIN32)
        return()
    endif()

    add_custom_target(
        "${TARGET_NAME}"
        DEPENDS "${TOOL_TARGET}Deploy"
        COMMAND "${CMAKE_COMMAND}" -E chdir "$<TARGET_FILE_DIR:${TOOL_TARGET}>"
            "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>/plugins"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:${TOOL_TARGET}>/plugins/platforms"
            "$<TARGET_FILE:${TOOL_TARGET}>"
            ${ARGN}
        VERBATIM
        USES_TERMINAL
    )
endfunction()

# ----------------------------------------------------------------------------
# 1. CompileTranslationsGUI
# ----------------------------------------------------------------------------
add_executable(
    CompileTranslationsGUI
    WIN32
    CompileTranslationsGUI.cpp
    CompileTranslationsGUI.hpp
)
target_link_libraries(CompileTranslationsGUI PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
mocks_add_windows_icon(CompileTranslationsGUI)
mocks_apply_common_options(CompileTranslationsGUI)
mocks_add_deploy_target(CompileTranslationsGUI)

# ----------------------------------------------------------------------------
# 2. CompileTranslationsTerminal
# ----------------------------------------------------------------------------
add_executable(
    CompileTranslationsTerminal
    CompileTranslationsTerminal.cpp
    CompileTranslationsTerminal.hpp
)
target_link_libraries(CompileTranslationsTerminal PRIVATE Qt6::Core)
if(WIN32)
    target_link_libraries(CompileTranslationsTerminal PRIVATE User32)
endif()
mocks_add_windows_icon(CompileTranslationsTerminal)
mocks_apply_common_options(CompileTranslationsTerminal)
mocks_add_deploy_target(CompileTranslationsTerminal)

# ----------------------------------------------------------------------------
# 3. find_VersionEditor
# ----------------------------------------------------------------------------
add_executable(
    find_VersionEditor
    WIN32
    find_VersionEditor.cpp
    find_VersionEditor.hpp
)
target_link_libraries(find_VersionEditor PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_compile_features(find_VersionEditor PRIVATE cxx_std_23)
mocks_add_windows_icon(find_VersionEditor)
mocks_apply_common_options(find_VersionEditor)
mocks_add_deploy_target(find_VersionEditor)

# ----------------------------------------------------------------------------
# 4. Alvo Unificado de Deploy e Execução dos Mocks
# ----------------------------------------------------------------------------
add_custom_target(
    MocksDeploy
    ALL
    DEPENDS
        CompileTranslationsGUIDeploy
        CompileTranslationsTerminalDeploy
        find_VersionEditorDeploy
    COMMENT "Deploying all ACL Auditor Mocks runtimes"
)

if(WIN32)
    set(MOCKS_DETACHED_GUI_LAUNCHER "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/launch_detached_gui.cmake")

    add_custom_target(
        MocksRun
        DEPENDS MocksDeploy
        COMMAND "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:find_VersionEditor>/plugins"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:find_VersionEditor>/plugins/platforms"
            "${CMAKE_COMMAND}"
            -DLAUNCH_FILE=$<TARGET_FILE:find_VersionEditor>
            -DLAUNCH_WORKING_DIR=$<TARGET_FILE_DIR:find_VersionEditor>
            -P "${MOCKS_DETACHED_GUI_LAUNCHER}"
        COMMAND "${CMAKE_COMMAND}" -E env
            "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:CompileTranslationsGUI>/plugins"
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:CompileTranslationsGUI>/plugins/platforms"
            "${CMAKE_COMMAND}"
            -DLAUNCH_FILE=$<TARGET_FILE:CompileTranslationsGUI>
            -DLAUNCH_WORKING_DIR=$<TARGET_FILE_DIR:CompileTranslationsGUI>
            -P "${MOCKS_DETACHED_GUI_LAUNCHER}"
        COMMENT "Launching mocks runtime tools"
        VERBATIM
        USES_TERMINAL
    )

    mocks_add_gui_run_target(find_VersionEditorRun find_VersionEditor)
    mocks_add_gui_run_target(CompileTranslationsGUIRunAll CompileTranslationsGUI --all)
    mocks_add_gui_run_target(CompileTranslationsGUIRunACLAuditor CompileTranslationsGUI --module aclauditor)

    mocks_add_terminal_run_target(CompileTranslationsTerminalRunAll CompileTranslationsTerminal --all)
    mocks_add_terminal_run_target(CompileTranslationsTerminalRunACLAuditor CompileTranslationsTerminal --module aclauditor)
endif()
