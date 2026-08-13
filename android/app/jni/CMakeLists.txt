cmake_minimum_required(VERSION 3.20)

project(AsteroidsAndroid LANGUAGES C CXX)

add_subdirectory(SDL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../.. ${CMAKE_CURRENT_BINARY_DIR}/asteroids)
