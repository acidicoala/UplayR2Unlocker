cmake_minimum_required(VERSION 3.21)

include(KoalaBox/KoalaBox.cmake)

configure_globals(KoalaBox)

project(UplayR2Unlocker VERSION 3.0.1)

set_32_and_64(ORIGINAL_DLL upc_r2_loader upc_r2_loader64)
set_32_and_64(LEGACY_ORIGINAL_DLL uplay_r2_loader uplay_r2_loader64)

set_32_and_64(STORE_DLL uplay_r2 uplay_r264)

configure_version_resource("https://github.com/acidicoala/UplayR2Unlocker")

set_32_and_64(DLL_INPUT "${RES_DIR}/uplay_r2.dll" "${RES_DIR}/upc_r2_loader64.dll")
configure_exports_generator()
configure_linker_exports(
    false
    "${ORIGINAL_DLL}_o"
    ${DLL_INPUT}
    "${SRC_DIR}/upc"
    "${SRC_DIR}/upc/upc.cpp"
)

configure_build_config(extra_build_config)

configure_library(
    SHARED

    ${SRC_DIR}/unlocker/unlocker.cpp
    ${SRC_DIR}/upc/upc.cpp
    ${SRC_DIR}/main.cpp

    ${KOALABOX_SRC_DIR}/koalabox/dll_monitor/dll_monitor.cpp
    ${KOALABOX_SRC_DIR}/koalabox/hook/hook.cpp
    ${KOALABOX_SRC_DIR}/koalabox/loader/loader.cpp
    ${KOALABOX_SRC_DIR}/koalabox/file_logger/file_logger.cpp
    ${KOALABOX_SRC_DIR}/koalabox/util/util.cpp
    ${KOALABOX_SRC_DIR}/koalabox/win_util/win_util.cpp

    ${LINKER_EXPORTS}
)

configure_precompile_headers(${CMAKE_PROJECT_NAME} ${SRC_DIR}/pch.hpp)

configure_output_name(${ORIGINAL_DLL})

configure_include_directories()

configure_dependencies(${CMAKE_PROJECT_NAME} cpr PolyHook_2 nlohmann_json spdlog)
