cmake_minimum_required(VERSION 3.5)

project(texture)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})

