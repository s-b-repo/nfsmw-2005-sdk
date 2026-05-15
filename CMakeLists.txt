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

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# === The SDK is header-only — interface library ===
add_library(nfsmw_sdk INTERFACE)
target_include_directories(nfsmw_sdk INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(nfsmw_sdk INTERFACE kernel32)

# === Unified entry shim (one DLL serves ASI + BepInEx) ===
add_library(nfsmw_entry OBJECT src/entry.c)
target_link_libraries(nfsmw_entry PUBLIC nfsmw_sdk)

# === Plugin helper macro ===
# Usage:  nfsmw_add_plugin(my_mod SOURCES my_mod.cpp)
# Produces my_mod.dll (BepInEx plugin) + my_mod.asi (ASI loader copy).
function(nfsmw_add_plugin name)
    cmake_parse_arguments(P "" "" "SOURCES" ${ARGN})

    add_library(${name} SHARED ${P_SOURCES} $<TARGET_OBJECTS:nfsmw_entry>)
    target_link_libraries(${name} PRIVATE nfsmw_sdk)
    set_target_properties(${name} PROPERTIES PREFIX "" SUFFIX ".dll")

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${name}> $<TARGET_FILE_DIR:${name}>/${name}.asi
        COMMENT "Copying ${name}.dll -> ${name}.asi for ASI Loader")
endfunction()

# === Examples ===
if(NFSMW_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# === Install ===
install(DIRECTORY include/nfsmw_sdk DESTINATION include)
install(TARGETS nfsmw_sdk EXPORT nfsmw_sdkTargets)
install(EXPORT nfsmw_sdkTargets DESTINATION lib/cmake/nfsmw_sdk)
