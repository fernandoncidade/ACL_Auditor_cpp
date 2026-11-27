cmake_minimum_required(VERSION 3.28)

# ------------------------------------------------------------------------------
# Ativos de runtime locais no diretório de build da suíte (sem distribuição externa)
# ------------------------------------------------------------------------------
set(_TEST_RUNTIME_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/source/assets" "${CMAKE_BINARY_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${TRANSLATIONS_BIN_DIR}" "${CMAKE_BINARY_DIR}/translations"
)

add_custom_target(ACLAuditorTestRuntimeAssets
    ${_TEST_RUNTIME_COMMANDS}
    DEPENDS ACLTranslations
    COMMENT "Preparando assets e traduções localmente no build da suíte (sem distribuição externa)..."
)

set(ACL_TEST_TARGETS
    test_01_FrameworkAndPlugins
    test_02_NtfsSecurityAndCrypto
    test_03_EffectiveAccessEngine
    test_04_RiskScannerAndRemediation
    test_05_ServicesAndSmbAudit
    test_06_SnapshotEngineAndDiff
    test_07_ModelsUtilsAndI18n
    test_08_GuiWidgetsDialogsAndMainWindow
    test_09_CliEndToEnd
    test_10_MocksAssetsAndProjectIntegrity
    test_11_EngineEndToEndRegression
)

set(_ACL_QT_PLUGINS_DIR "${QT_PREFIX_DIR}/plugins")
file(TO_NATIVE_PATH "${_ACL_QT_PLUGINS_DIR}" _ACL_QT_PLUGINS_NATIVE)
file(TO_NATIVE_PATH "${QT_BIN_DIR}" _ACL_QT_BIN_NATIVE)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}" _ACL_BINARY_NATIVE)

set(_TEST_ENV
    "QT_QPA_PLATFORM=offscreen"
    "QT_PLUGIN_PATH=${_ACL_QT_PLUGINS_NATIVE}"
)
set(_TEST_ENV_MOD
    "PATH=path_list_prepend:${_ACL_QT_BIN_NATIVE}"
    "PATH=path_list_prepend:${_ACL_BINARY_NATIVE}"
)
if(MINGW AND DEFINED MINGW_BIN_DIR AND NOT "${MINGW_BIN_DIR}" STREQUAL "")
    file(TO_NATIVE_PATH "${MINGW_BIN_DIR}" _ACL_MINGW_BIN_NATIVE)
    list(APPEND _TEST_ENV_MOD "PATH=path_list_prepend:${_ACL_MINGW_BIN_NATIVE}")
endif()

foreach(T_NAME IN LISTS ACL_TEST_TARGETS)
    add_executable(${T_NAME} ${T_NAME}.cpp)

    set_target_properties(${T_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    target_include_directories(${T_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/source"
    )

    if(T_NAME STREQUAL "test_10_MocksAssetsAndProjectIntegrity")
        target_sources(${T_NAME} PRIVATE
            "${CMAKE_SOURCE_DIR}/mocks/find_VersionEditor.cpp"
            "${CMAKE_SOURCE_DIR}/mocks/find_VersionEditor.hpp"
            "${CMAKE_SOURCE_DIR}/mocks/CompileTranslationsTerminal.cpp"
            "${CMAKE_SOURCE_DIR}/mocks/CompileTranslationsTerminal.hpp"
        )
        target_include_directories(${T_NAME} PRIVATE
            "${CMAKE_SOURCE_DIR}/mocks"
        )
        target_compile_definitions(${T_NAME} PRIVATE
            ACL_AUDITOR_MOCKS_AS_LIBRARY=1
        )
    endif()

    target_link_libraries(${T_NAME} PRIVATE
        ACLAuditorUI
        ACLAuditorEngine
        Qt6::Test
    )

    target_compile_definitions(${T_NAME} PRIVATE
        ACL_AUDITOR_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        ACL_AUDITOR_BINARY_DIR="${CMAKE_BINARY_DIR}"
        ACL_AUDITOR_QT_PREFIX_DIR="${QT_PREFIX_DIR}"
        ACL_AUDITOR_QT_BIN_DIR="${QT_BIN_DIR}"
        ACL_AUDITOR_QT_PLUGINS_DIR="${_ACL_QT_PLUGINS_DIR}"
        ACL_AUDITOR_LRELEASE_EXECUTABLE="${QT_BIN_DIR}/lrelease.exe"
        ACL_AUDITOR_PROJECT_VERSION="${PROJECT_VERSION}"
    )

    if(TARGET acl-auditor-cli)
        target_compile_definitions(${T_NAME} PRIVATE
            ACL_AUDITOR_CLI_EXECUTABLE="$<TARGET_FILE:acl-auditor-cli>"
        )
        if(T_NAME STREQUAL "test_09_CliEndToEnd")
            add_dependencies(${T_NAME} acl-auditor-cli)
        endif()
    endif()

    add_dependencies(${T_NAME} ACLAuditorTestRuntimeAssets)

    add_test(NAME ${T_NAME} COMMAND $<TARGET_FILE:${T_NAME}>)
    set_tests_properties(${T_NAME} PROPERTIES
        ENVIRONMENT "${_TEST_ENV}"
        ENVIRONMENT_MODIFICATION "${_TEST_ENV_MOD}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        TIMEOUT 120
    )
endforeach()

set(_SUITE_DEPS ACLAuditorTestRuntimeAssets ${ACL_TEST_TARGETS})
if(TARGET acl-auditor-cli)
    list(APPEND _SUITE_DEPS acl-auditor-cli)
endif()

add_custom_target(ACLAuditorTestSuite ALL
    DEPENDS ${_SUITE_DEPS}
    COMMENT "Compilando toda a suíte profissional de testes do ACL Auditor (sem distribuição externa)..."
)
