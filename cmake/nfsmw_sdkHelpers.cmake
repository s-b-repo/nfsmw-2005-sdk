# nfsmw_sdkHelpers.cmake — shared between the in-tree build and installed
# consumers (via find_package(nfsmw_sdk CONFIG)). Single source of truth
# for the plugin build recipe.
#
# Requires, before inclusion:
#   NFSMW_SDK_INCLUDE_DIR  — path containing nfsmw_sdk/*.h
#   NFSMW_SDK_ENTRY_SOURCE — path to src/entry.c
#   NFSMW_SDK_MINHOOK_DIR  — path to the vendored extern/minhook tree
# Optional:
#   NFSMW_HOOKS_BACKEND    — "minhook" (default) or "simple"
#
# Provides:
#   nfsmw_add_plugin(<name> SOURCES <a.cpp> [b.cpp ...])
#     -> <name>.dll (BepInEx plugin) + <name>.asi (ASI loader copy)

if(NOT DEFINED NFSMW_HOOKS_BACKEND)
    set(NFSMW_HOOKS_BACKEND "minhook")
endif()

set(NFSMW__MINHOOK_SRCS
    "${NFSMW_SDK_MINHOOK_DIR}/src/buffer.c"
    "${NFSMW_SDK_MINHOOK_DIR}/src/hook.c"
    "${NFSMW_SDK_MINHOOK_DIR}/src/trampoline.c"
    "${NFSMW_SDK_MINHOOK_DIR}/src/hde/hde32.c"
    "${NFSMW_SDK_MINHOOK_DIR}/src/hde/hde64.c")

function(nfsmw_add_plugin name)
    cmake_parse_arguments(P "" "" "SOURCES" ${ARGN})
    if(NOT P_SOURCES)
        message(FATAL_ERROR "nfsmw_add_plugin(${name}): SOURCES is required")
    endif()

    set(_srcs ${P_SOURCES} "${NFSMW_SDK_ENTRY_SOURCE}")
    if(NFSMW_HOOKS_BACKEND STREQUAL "minhook")
        list(APPEND _srcs ${NFSMW__MINHOOK_SRCS})
    endif()

    add_library(${name} SHARED ${_srcs})
    target_include_directories(${name} PRIVATE "${NFSMW_SDK_INCLUDE_DIR}")
    target_link_libraries(${name} PRIVATE kernel32)
    set_target_properties(${name} PROPERTIES
        PREFIX "" SUFFIX ".dll"
        C_STANDARD 11
        CXX_STANDARD 17)

    if(NFSMW_HOOKS_BACKEND STREQUAL "minhook")
        target_include_directories(${name} PRIVATE
            "${NFSMW_SDK_MINHOOK_DIR}/include")
        target_compile_definitions(${name} PRIVATE NFSMW_HOOKS_BACKEND_MINHOOK)
    endif()

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${name}> $<TARGET_FILE_DIR:${name}>/${name}.asi
        COMMENT "Copying ${name}.dll -> ${name}.asi for ASI Loader")
endfunction()
