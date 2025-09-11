cmake_minimum_required(VERSION 3.16)
project(OPMon_Red CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFML setup
set(SFML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/include")
set(SFML_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/libs/lib")
include_directories(${SFML_INCLUDE_DIR})
include_directories("src/")
link_directories(${SFML_LIBRARY_DIR})

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

add_executable(OPMon_Red ${SOURCES})

target_link_libraries(OPMon_Red
    sfml-graphics
    sfml-window
    sfml-system
)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})