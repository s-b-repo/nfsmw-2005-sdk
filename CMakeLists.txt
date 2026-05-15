cmake_minimum_required(VERSION 3.16)
project(nfsmw_sdk LANGUAGES C CXX VERSION 1.0.0)

# === Sanity checks ===
if(NOT WIN32)
    message(FATAL_ERROR
        "nfsmw_sdk targets Win32 PE only. To build from Linux or macOS, use:\n"
        "  cmake -DCMAKE_TOOLCHAIN_FILE=cmake/nfsmw-toolchain-mingw-i686.cmake ..\n"
        "Requires: i686-w64-mingw32-gcc (install via your package manager).")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR
        "nfsmw_sdk requires a 32-bit build. NFSMW is i386 only.\n"
        "Add `-m32` to CMAKE_CXX_FLAGS or use the i686 toolchain file.")
endif()

# === Options ===
option(NFSMW_BUILD_EXAMPLES "Build example mods" ON)
set(NFSMW_HOOKS_BACKEND "minhook" CACHE STRING
    "Inline-hook backend: minhook (robust, vendored) or simple (5-byte E9)")
set_property(CACHE NFSMW_HOOKS_BACKEND PROPERTY STRINGS minhook simple)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# === The SDK is header-only — interface library ===
add_library(nfsmw_sdk INTERFACE)
add_library(nfsmw::sdk ALIAS nfsmw_sdk)
target_include_directories(nfsmw_sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(nfsmw_sdk INTERFACE kernel32)

# === Plugin helper (shared with installed consumers) ===
set(NFSMW_SDK_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(NFSMW_SDK_ENTRY_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/entry.c")
set(NFSMW_SDK_MINHOOK_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/extern/minhook")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/nfsmw_sdkHelpers.cmake")

# === Examples ===
if(NFSMW_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# === Install + package config ===
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(INCLUDE_INSTALL_DIR  "${CMAKE_INSTALL_INCLUDEDIR}")
set(ENTRY_INSTALL_PATH   "${CMAKE_INSTALL_DATADIR}/nfsmw_sdk/src/entry.c")
set(MINHOOK_INSTALL_DIR  "${CMAKE_INSTALL_DATADIR}/nfsmw_sdk/extern/minhook")
set(CONFIG_INSTALL_DIR   "${CMAKE_INSTALL_LIBDIR}/cmake/nfsmw_sdk")

install(DIRECTORY include/nfsmw_sdk
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES src/entry.c
        DESTINATION "${CMAKE_INSTALL_DATADIR}/nfsmw_sdk/src")
install(DIRECTORY extern/minhook
        DESTINATION "${CMAKE_INSTALL_DATADIR}/nfsmw_sdk/extern")
install(FILES cmake/nfsmw_sdkHelpers.cmake
        DESTINATION "${CONFIG_INSTALL_DIR}")
install(TARGETS nfsmw_sdk EXPORT nfsmw_sdkTargets)
install(EXPORT nfsmw_sdkTargets
        NAMESPACE nfsmw::
        DESTINATION "${CONFIG_INSTALL_DIR}")

configure_package_config_file(
    cmake/nfsmw_sdkConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/nfsmw_sdkConfig.cmake"
    INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
    PATH_VARS INCLUDE_INSTALL_DIR ENTRY_INSTALL_PATH MINHOOK_INSTALL_DIR)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/nfsmw_sdkConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nfsmw_sdkConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/nfsmw_sdkConfigVersion.cmake"
    DESTINATION "${CONFIG_INSTALL_DIR}")
