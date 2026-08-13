set(ASTEROIDS_PLATFORM_SOURCES "")
if(ANDROID)
    list(APPEND ASTEROIDS_PLATFORM_SOURCES platform/platform_android.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND ASTEROIDS_PLATFORM_SOURCES platform/platform_linux.c)
else()
    message(FATAL_ERROR "Asteroids currently only ships platform layers for Linux and Android. Add a platform/platform_<os>.c implementation for this target.")
endif()

set(ASTEROIDS_SOURCES
    main.c
    core/vec2.h
    core/rng.h
    core/shapes.h
    platform/platform.h
    ${ASTEROIDS_PLATFORM_SOURCES}
    config/config.h
    config/config.c
    save/save.h
    save/save.c
    game/entities.h
    game/game.h
    game/game.c
    render/renderer.h
    render/renderer.c
    render/vector_font.h
    render/vector_font.c
    audio/audio.h
    audio/audio.c
)

if(ANDROID)
    add_library(asteroids SHARED ${ASTEROIDS_SOURCES})
else()
    add_executable(asteroids ${ASTEROIDS_SOURCES})
endif()

target_include_directories(asteroids PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_library(ASTEROIDS_MATH_LIBRARY m)

target_link_libraries(asteroids PRIVATE SDL3::SDL3 asteroids_compiler_flags)
if(ASTEROIDS_MATH_LIBRARY)
    target_link_libraries(asteroids PRIVATE ${ASTEROIDS_MATH_LIBRARY})
endif()

target_compile_definitions(asteroids PRIVATE
    ASTEROIDS_ENABLE_DEBUG_OVERLAY=$<BOOL:${ASTEROIDS_ENABLE_DEBUG_OVERLAY}>
    ASTEROIDS_INSTALL_ASSET_DIR="${CMAKE_INSTALL_PREFIX}/share/asteroids/assets"
)

if(ASTEROIDS_ENABLE_ASSERTIONS)
    target_compile_definitions(asteroids PRIVATE ASTEROIDS_FORCE_ASSERTIONS=1)
else()
    target_compile_definitions(asteroids PRIVATE $<$<CONFIG:Release,MinSizeRel>:NDEBUG>)
endif()

if(ASTEROIDS_ALTERNATE_CONTROLS)
    target_compile_definitions(asteroids PRIVATE ASTEROIDS_ALTERNATE_CONTROLS=1)
endif()

if(ANDROID)
    install(TARGETS asteroids LIBRARY DESTINATION lib)
else()
    install(TARGETS asteroids RUNTIME DESTINATION bin)
endif()
