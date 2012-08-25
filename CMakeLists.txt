cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(LANA)

add_definitions(-DLANA_API_VERSION="1.0")
add_subdirectory(lana)
add_subdirectory(cli)
add_subdirectory(tests)

