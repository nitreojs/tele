cmake_minimum_required(VERSION 3.25)
cmake_policy(SET CMP0091 NEW)
project(tele_sealed_tests LANGUAGES CXX)

set(TDESKTOP_SOURCE_DIR "" CACHE PATH "tdesktop checkout with the sealed-message patch applied")
set(OPENSSL_RUNTIME_DLL "" CACHE FILEPATH "OpenSSL crypto runtime for standalone Windows tests")
set(SEALED_QT_PLUGIN_DIR "" CACHE PATH "Qt image plugin directory for JPEG tests")

if(NOT EXISTS "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_crypto.cpp")
    message(FATAL_ERROR "Set TDESKTOP_SOURCE_DIR to a patched tdesktop checkout")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)
if(NOT SEALED_QT_PLUGIN_DIR)
    get_filename_component(qt_plugin_candidate "${Qt6_DIR}/../../../plugins" ABSOLUTE)
    if(EXISTS "${qt_plugin_candidate}/imageformats")
        set(SEALED_QT_PLUGIN_DIR "${qt_plugin_candidate}")
    endif()
endif()
find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)
find_package(Python3 COMPONENTS Interpreter)
include(CTest)

function(configure_sealed_test target)
    target_include_directories(${target} PRIVATE "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles")
    target_compile_definitions(${target} PRIVATE NOMINMAX)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /Zc:__cplusplus /permissive- /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:${target}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:${target}>
        )
        if(OPENSSL_RUNTIME_DLL)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${OPENSSL_RUNTIME_DLL}" $<TARGET_FILE_DIR:${target}>
            )
        endif()
    endif()
endfunction()

add_executable(sealed_crypto_test crypto_test.cpp)
configure_sealed_test(sealed_crypto_test)
target_link_libraries(sealed_crypto_test PRIVATE Qt6::Core OpenSSL::Crypto)

add_executable(sealed_duress_store_test
    duress_store_test.cpp
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_duress_store.cpp"
)
configure_sealed_test(sealed_duress_store_test)
target_link_libraries(sealed_duress_store_test PRIVATE Qt6::Core OpenSSL::Crypto)

add_executable(sealed_registry_test
    registry_test.cpp
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_registry.cpp"
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_duress_store.cpp"
)
configure_sealed_test(sealed_registry_test)
target_link_libraries(sealed_registry_test PRIVATE Qt6::Core OpenSSL::Crypto)

add_executable(sealed_signout_plan_test signout_plan_test.cpp)
configure_sealed_test(sealed_signout_plan_test)
target_link_libraries(sealed_signout_plan_test PRIVATE Qt6::Core)

add_executable(sealed_auto_mode_test auto_mode_test.cpp)
configure_sealed_test(sealed_auto_mode_test)
target_link_libraries(sealed_auto_mode_test PRIVATE Qt6::Core)

add_executable(sealed_send_plan_test send_plan_test.cpp)
configure_sealed_test(sealed_send_plan_test)
target_link_libraries(sealed_send_plan_test PRIVATE Qt6::Core)

add_executable(sealed_view_cache_test view_cache_test.cpp)
configure_sealed_test(sealed_view_cache_test)
target_link_libraries(sealed_view_cache_test PRIVATE Qt6::Core OpenSSL::Crypto)

add_executable(sealed_passcode_gate_test passcode_gate_test.cpp)
configure_sealed_test(sealed_passcode_gate_test)

add_executable(sealed_files_test
    files_test.cpp
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_files.cpp"
)
configure_sealed_test(sealed_files_test)
target_precompile_headers(sealed_files_test PRIVATE test_pch.h)
target_link_libraries(sealed_files_test PRIVATE Qt6::Core Qt6::Gui)

add_executable(sealed_store_facade_test
    store_facade/store_test.cpp
    store_facade/fake_storage.cpp
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_registry.cpp"
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_duress_store.cpp"
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_store.cpp"
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_crypto.cpp"
)
configure_sealed_test(sealed_store_facade_test)
target_include_directories(sealed_store_facade_test BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/store_facade/stubs"
    "${TDESKTOP_SOURCE_DIR}/Telegram/ThirdParty/GSL/include"
)
target_precompile_headers(sealed_store_facade_test PRIVATE store_facade/test_pch.h)
target_link_libraries(sealed_store_facade_test PRIVATE Qt6::Core OpenSSL::Crypto)

add_test(NAME sealed_crypto
    COMMAND sealed_crypto_test "${CMAKE_CURRENT_SOURCE_DIR}/hpke-auth-vectors.json"
)
add_test(NAME sealed_rng_failure
    COMMAND sealed_crypto_test --rng-failure "${CMAKE_CURRENT_SOURCE_DIR}/hpke-auth-vectors.json"
)
add_test(NAME sealed_files COMMAND sealed_files_test)
add_test(NAME sealed_duress_store COMMAND sealed_duress_store_test)
add_test(NAME sealed_registry COMMAND sealed_registry_test)
add_test(NAME sealed_signout_plan COMMAND sealed_signout_plan_test)
add_test(NAME sealed_auto_mode COMMAND sealed_auto_mode_test)
set_tests_properties(sealed_auto_mode PROPERTIES LABELS standalone)
add_test(NAME sealed_send_plan COMMAND sealed_send_plan_test)
add_test(NAME sealed_view_cache COMMAND sealed_view_cache_test)
set_tests_properties(sealed_send_plan sealed_view_cache PROPERTIES LABELS standalone)
add_test(NAME sealed_passcode_gate COMMAND sealed_passcode_gate_test)
set_tests_properties(sealed_passcode_gate PROPERTIES LABELS standalone)
add_test(NAME sealed_store_facade COMMAND sealed_store_facade_test)
set_tests_properties(sealed_crypto sealed_rng_failure sealed_files sealed_duress_store sealed_registry sealed_signout_plan PROPERTIES LABELS "standalone")
set_tests_properties(sealed_store_facade PROPERTIES LABELS "standalone;facade")
if(SEALED_QT_PLUGIN_DIR)
    set_tests_properties(sealed_files PROPERTIES
        ENVIRONMENT "QT_PLUGIN_PATH=${SEALED_QT_PLUGIN_DIR}"
    )
endif()

if(Python3_Interpreter_FOUND)
    execute_process(COMMAND "${Python3_EXECUTABLE}" -c "import cryptography"
        RESULT_VARIABLE has_cryptography OUTPUT_QUIET ERROR_QUIET
    )
    if(has_cryptography EQUAL 0)
        set(interop_fixture "${CMAKE_CURRENT_BINARY_DIR}/interoperability-fixture.json")
        add_test(NAME sealed_interop_fixture
            COMMAND sealed_crypto_test --fixtures
                "${CMAKE_CURRENT_SOURCE_DIR}/hpke-auth-vectors.json"
                "${interop_fixture}"
        )
        add_test(NAME sealed_python_interop
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/interop.py"
                --vectors "${CMAKE_CURRENT_SOURCE_DIR}/hpke-auth-vectors.json"
                --fixture "${interop_fixture}"
        )
        set_tests_properties(sealed_interop_fixture PROPERTIES
            FIXTURES_SETUP sealed_interop
        )
        set_tests_properties(sealed_python_interop PROPERTIES
            FIXTURES_REQUIRED sealed_interop
        )
    else()
        message(STATUS "Python cryptography is absent; independent interop tests are not configured")
    endif()
endif()

add_executable(sealed_codes_filter_test
    codes_filter_test.cpp
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_codes_filter.cpp"
)
configure_sealed_test(sealed_codes_filter_test)
target_link_libraries(sealed_codes_filter_test PRIVATE Qt6::Core)
add_test(NAME sealed_codes_filter COMMAND sealed_codes_filter_test)
set_tests_properties(sealed_codes_filter PROPERTIES LABELS standalone)

add_executable(sealed_code_mode_facade_test
    code_mode_facade/main.cpp
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_codes_filter.cpp"
)
configure_sealed_test(sealed_code_mode_facade_test)
target_include_directories(sealed_code_mode_facade_test BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/code_mode_facade/stubs"
    "${CMAKE_CURRENT_SOURCE_DIR}/code_mode_facade"
)
target_precompile_headers(sealed_code_mode_facade_test PRIVATE code_mode_facade/common.h)
target_link_libraries(sealed_code_mode_facade_test PRIVATE Qt6::Core OpenSSL::Crypto)
add_test(NAME sealed_code_mode_facade COMMAND sealed_code_mode_facade_test)
set_tests_properties(sealed_code_mode_facade PROPERTIES LABELS "standalone;facade")

add_executable(sealed_text_test
    text_test.cpp
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_text.cpp"
    "${TDESKTOP_SOURCE_DIR}/Telegram/SourceFiles/tele/tele_sealed_crypto.cpp"
)
configure_sealed_test(sealed_text_test)
target_link_libraries(sealed_text_test PRIVATE Qt6::Core OpenSSL::Crypto)
add_test(NAME sealed_text COMMAND sealed_text_test)
add_test(NAME sealed_text_rng_failure
    COMMAND sealed_text_test --rng-failure "${CMAKE_CURRENT_SOURCE_DIR}/hpke-auth-vectors.json"
)
set_tests_properties(sealed_text sealed_text_rng_failure PROPERTIES LABELS standalone)

add_executable(sealed_rich_send_plan_test rich_send_plan_test.cpp)
configure_sealed_test(sealed_rich_send_plan_test)
add_test(NAME sealed_rich_send_plan COMMAND sealed_rich_send_plan_test)
set_tests_properties(sealed_rich_send_plan PROPERTIES LABELS standalone)

add_executable(sealed_jobs_plan_test jobs_plan_test.cpp)
configure_sealed_test(sealed_jobs_plan_test)
add_test(NAME sealed_jobs_plan COMMAND sealed_jobs_plan_test)
set_tests_properties(sealed_jobs_plan PROPERTIES LABELS standalone)

add_executable(sealed_search_plan_test search_plan_test.cpp)
configure_sealed_test(sealed_search_plan_test)
target_link_libraries(sealed_search_plan_test PRIVATE Qt6::Core)
add_test(NAME sealed_search_plan COMMAND sealed_search_plan_test)
set_tests_properties(sealed_search_plan PROPERTIES LABELS standalone)
