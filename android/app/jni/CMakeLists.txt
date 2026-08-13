cmake_minimum_required(VERSION 3.20)

project(AsteroidsAndroid LANGUAGES C CXX)

add_subdirectory(SDL)

set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
add_subdirectory(SDL_ttf)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../.. ${CMAKE_CURRENT_BINARY_DIR}/asteroids)
