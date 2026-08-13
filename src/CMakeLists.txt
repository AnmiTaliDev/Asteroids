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
    audio/audio.h
    audio/audio.c
)

if(ANDROID)
    set(ASTEROIDS_TARGET_NAME main)
    add_library(${ASTEROIDS_TARGET_NAME} SHARED ${ASTEROIDS_SOURCES})
else()
    set(ASTEROIDS_TARGET_NAME asteroids)
    add_executable(${ASTEROIDS_TARGET_NAME} ${ASTEROIDS_SOURCES})
endif()

target_include_directories(${ASTEROIDS_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_library(ASTEROIDS_MATH_LIBRARY m)

target_link_libraries(${ASTEROIDS_TARGET_NAME} PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf asteroids_compiler_flags)
if(ASTEROIDS_MATH_LIBRARY)
    target_link_libraries(${ASTEROIDS_TARGET_NAME} PRIVATE ${ASTEROIDS_MATH_LIBRARY})
endif()

target_compile_definitions(${ASTEROIDS_TARGET_NAME} PRIVATE
    ASTEROIDS_ENABLE_DEBUG_OVERLAY=$<BOOL:${ASTEROIDS_ENABLE_DEBUG_OVERLAY}>
    ASTEROIDS_INSTALL_ASSET_DIR="${CMAKE_INSTALL_PREFIX}/share/asteroids/assets"
)

if(ASTEROIDS_ENABLE_ASSERTIONS)
    target_compile_definitions(${ASTEROIDS_TARGET_NAME} PRIVATE ASTEROIDS_FORCE_ASSERTIONS=1)
else()
    target_compile_definitions(${ASTEROIDS_TARGET_NAME} PRIVATE $<$<CONFIG:Release,MinSizeRel>:NDEBUG>)
endif()

if(ASTEROIDS_ALTERNATE_CONTROLS)
    target_compile_definitions(${ASTEROIDS_TARGET_NAME} PRIVATE ASTEROIDS_ALTERNATE_CONTROLS=1)
endif()

if(ANDROID)
    install(TARGETS ${ASTEROIDS_TARGET_NAME} LIBRARY DESTINATION lib)
else()
    install(TARGETS ${ASTEROIDS_TARGET_NAME} RUNTIME DESTINATION bin)
endif()
