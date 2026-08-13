cmake_minimum_required(VERSION 3.20)

project(Asteroids
    VERSION 1.0.0
    DESCRIPTION "A classic vector-style Asteroids arcade game"
    HOMEPAGE_URL "https://github.com/AnmiTaliDev/Asteroids"
    LANGUAGES C
)

set(ASTEROIDS_AUTHOR "AnmiTaliDev <anmitalidev@nuros.org>")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(ASTEROIDS_ENABLE_DEBUG_OVERLAY "Compile in the debug overlay (FPS, entity counts, hitboxes)" ON)
option(ASTEROIDS_ENABLE_ASSERTIONS "Enable runtime assertions regardless of build type" OFF)
option(ASTEROIDS_ALTERNATE_CONTROLS "Build with an alternate default control scheme (arrow keys instead of WASD)" OFF)
option(ASTEROIDS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ASTEROIDS_BUILD_TESTS "Build the unit test suite" OFF)

if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 REQUIRED CONFIG)
endif()

add_library(asteroids_compiler_flags INTERFACE)
target_compile_options(asteroids_compiler_flags INTERFACE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion>
    $<$<AND:$<C_COMPILER_ID:GNU,Clang>,$<BOOL:${ASTEROIDS_WARNINGS_AS_ERRORS}>>:-Werror>
)

add_subdirectory(src)

if(ASTEROIDS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(DIRECTORY assets/ DESTINATION share/asteroids/assets)
install(FILES config/game.cfg DESTINATION share/asteroids/config)

include(GNUInstallDirs)
message(STATUS "Asteroids ${PROJECT_VERSION} configuration summary")
message(STATUS "  Debug overlay:      ${ASTEROIDS_ENABLE_DEBUG_OVERLAY}")
message(STATUS "  Forced assertions:  ${ASTEROIDS_ENABLE_ASSERTIONS}")
message(STATUS "  Alternate controls: ${ASTEROIDS_ALTERNATE_CONTROLS}")
message(STATUS "  Warnings as errors: ${ASTEROIDS_WARNINGS_AS_ERRORS}")
message(STATUS "  Build tests:        ${ASTEROIDS_BUILD_TESTS}")
