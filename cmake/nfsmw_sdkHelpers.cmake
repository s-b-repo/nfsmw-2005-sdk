# nfsmw_sdkHelpers.cmake — shared between the in-tree build and installed
# consumers (via find_package(nfsmw_sdk CONFIG)). Single source of truth
# for the plugin build recipe.
#
# Requires, before inclusion:
#   NFSMW_SDK_INCLUDE_DIR  — path containing nfsmw_sdk/*.h
#   NFSMW_SDK_ENTRY_SOURCE — path to src/entry.c
#
# Provides:
#   nfsmw_add_plugin(<name> SOURCES <a.cpp> [b.cpp ...])
#     -> <name>.dll (BepInEx plugin) + <name>.asi (ASI loader copy)

function(nfsmw_add_plugin name)
    cmake_parse_arguments(P "" "" "SOURCES" ${ARGN})
    if(NOT P_SOURCES)
        message(FATAL_ERROR "nfsmw_add_plugin(${name}): SOURCES is required")
    endif()

    add_library(${name} SHARED ${P_SOURCES} "${NFSMW_SDK_ENTRY_SOURCE}")
    target_include_directories(${name} PRIVATE "${NFSMW_SDK_INCLUDE_DIR}")
    target_link_libraries(${name} PRIVATE kernel32)
    set_target_properties(${name} PROPERTIES
        PREFIX "" SUFFIX ".dll"
        C_STANDARD 11
        CXX_STANDARD 17)

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${name}> $<TARGET_FILE_DIR:${name}>/${name}.asi
        COMMENT "Copying ${name}.dll -> ${name}.asi for ASI Loader")
endfunction()
