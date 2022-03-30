cmake_minimum_required(VERSION 3.21)
project(ram)

set(CMAKE_CXX_STANDARD 20)

add_executable(demo main.cpp)

add_subdirectory(ram_lib)
target_link_libraries(demo PUBLIC ram_lib)

