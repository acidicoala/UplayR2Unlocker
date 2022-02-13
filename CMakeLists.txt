cmake_minimum_required(VERSION 3.21)

include(KoalaBox/KoalaBox.cmake)

configure_globals(KoalaBox)

project(UplayR2Unlocker VERSION "3.0.1")

set_32_and_64(ORIG_DLL "upc_r2_loader" "upc_r2_loader64")
configure_build_config(extra_build_config)

configure_version_resource("https://github.com/acidicoala/UplayR2Unlocker")

set_32_and_64(DLL_INPUT "${RES_DIR}/uplay_r2.dll" "${RES_DIR}/upc_r2_loader64.dll")
configure_exports_generator()
configure_linker_exports(
    "${ORIG_DLL}_o"
    ${DLL_INPUT}
    "${SRC_DIR}/upc"
    "${SRC_DIR}/upc/upc.cpp"
)

configure_library(
    SHARED
    ${SRC_DIR}/config/config.cpp
    ${SRC_DIR}/hooker/hooker.cpp
    ${SRC_DIR}/unlocker/unlocker.cpp
    ${SRC_DIR}/upc/upc.cpp
    ${SRC_DIR}/main.cpp
)

configure_output_name(${ORIG_DLL})

configure_include_directories()

configure_dependencies(spdlog nlohmann_json cpr PolyHook_2)
