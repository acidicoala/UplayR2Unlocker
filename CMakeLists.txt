cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)

# Determine architecture

string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" CMAKE_GENERATOR_PLATFORM)

if (${CMAKE_GENERATOR_PLATFORM} STREQUAL "win32")
    set(IS_64_BIT FALSE)
elseif (${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
    set(IS_64_BIT TRUE)
else ()
    message(FATAL_ERROR "Unexpected argument for the platform generator")
endif ()

# Configure vcpkg

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
if (IS_64_BIT)
    set(VCPKG_TARGET_TRIPLET x64-windows-static CACHE STRING "VCPKG Target Triplet to use")
else ()
    set(VCPKG_TARGET_TRIPLET x86-windows-static CACHE STRING "VCPKG Target Triplet to use")
endif ()

# Configure project

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(RES_DIR ${CMAKE_SOURCE_DIR}/res)

set(KOALABOX_SRC_DIR ${CMAKE_SOURCE_DIR}/KoalaBox/src)
set(KOALABOX_RES_DIR ${CMAKE_SOURCE_DIR}/KoalaBox/res)

file(STRINGS "${RES_DIR}/version.txt" PROJECT_VERSION_TEXT)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(UplayR2Unlocker VERSION "${PROJECT_VERSION_TEXT}.0")

set(PROJECT_VERSION_SHORT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
set(VERSION_SUFFIX $ENV{VERSION_SUFFIX})

set(GEN_DIR ${PROJECT_BINARY_DIR}/generated)

if (IS_64_BIT)
    set(ORIG_DLL upc_r2_loader64)
else ()
    set(ORIG_DLL upc_r2_loader)
endif ()

# Build the exports generator executable

add_executable(exports_generator ${KOALABOX_SRC_DIR}/koalabox/exports_generator/exports_generator.cpp)

# Generate the linker exports
set(LINKER_EXPORTS ${GEN_DIR}/linker_exports.h)

if (IS_64_BIT)
    set(GENERATOR_DLL_INPUT "${RES_DIR}/upc_r2_loader64.dll")
else ()
    set(GENERATOR_DLL_INPUT "${RES_DIR}/uplay_r2.dll")
endif ()

set(FORWARDED_DLL "${ORIG_DLL}_o")

add_custom_command(
        OUTPUT ${LINKER_EXPORTS}
        COMMAND exports_generator # (Output from step 1)
        "${FORWARDED_DLL}" # Forwarded DLL Name
        "${GENERATOR_DLL_INPUT}" # Input DLL
        "${SRC_DIR}/upc" # Input headers
        "${LINKER_EXPORTS}" # Output header
        DEPENDS exports_generator "${GENERATOR_DLL_INPUT}"
        "${SRC_DIR}/upc/upc.cpp"
)

# Make the linker_exports header available before build
file(MAKE_DIRECTORY ${GEN_DIR}) # Touch will fail if directory does not exist
file(TOUCH ${LINKER_EXPORTS})

# Substitute variables in build and runtime config files

## Generate build config file
configure_file(${RES_DIR}/build_config.gen.h ${GEN_DIR}/build_config.h)

## Generate version resource file
set(DLL_VERSION_FILE_DESC "https://github.com/acidicoala/UplayR2Unlocker")
set(DLL_VERSION_PRODUCT_NAME "Uplay R2 Unlocker")
set(DLL_VERSION_INTERNAL_NAME "${DLL_VERSION_PRODUCT_NAME}")
configure_file(${KOALABOX_RES_DIR}/version.gen.rc ${GEN_DIR}/version.rc)

# Build the library

add_library(
        ${CMAKE_PROJECT_NAME} SHARED
        ## Sources
        ${SRC_DIR}/main.cpp
        ${SRC_DIR}/unlocker/unlocker.cpp
        ${SRC_DIR}/upc/upc.cpp
        ${SRC_DIR}/config/config.cpp
        ${SRC_DIR}/hooker/hooker.cpp

        ## Dependencies
        ${KOALABOX_SRC_DIR}/koalabox/koalabox.cpp
        ${KOALABOX_SRC_DIR}/koalabox/logger/logger.cpp
        ${KOALABOX_SRC_DIR}/koalabox/win_util/win_util.cpp
        ${KOALABOX_SRC_DIR}/koalabox/util/util.cpp

        ## Resources
        ${GEN_DIR}/version.rc
        ${LINKER_EXPORTS}
)

# Configure output dll name


if (IS_64_BIT)
    set(OUTPUT_NAME upc_r2_loader64)
else ()
    set(OUTPUT_NAME upc_r2_loader)
endif ()

set_target_properties(
        ${CMAKE_PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_NAME "${OUTPUT_NAME}"
)

target_include_directories(
        ${CMAKE_PROJECT_NAME} PRIVATE
        ${SRC_DIR}
        ${KOALABOX_SRC_DIR}
        ${GEN_DIR}
)

target_precompile_headers(
        ${CMAKE_PROJECT_NAME} PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${SRC_DIR}/pch.hpp>"
)

# Link dependencies

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(PolyHook_2 CONFIG REQUIRED)

target_link_libraries(
        ${CMAKE_PROJECT_NAME} PRIVATE
        spdlog::spdlog
        nlohmann_json
        cpr::cpr
        PolyHook_2::PolyHook_2
)
