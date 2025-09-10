cmake_minimum_required(VERSION 3.16)
project(SFML_Test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/include")
set(SFML_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/libs/lib")

include_directories(${SFML_INCLUDE_DIR})
link_directories(${SFML_LIBRARY_DIR})

add_executable(sfml_test src/main.cpp)

target_link_libraries(sfml_test
    sfml-graphics
    sfml-window
    sfml-system
)
