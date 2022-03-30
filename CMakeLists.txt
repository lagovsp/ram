cmake_minimum_required(VERSION 3.21)
project(random-access-machine-emulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(demo demo/main.cpp)

add_subdirectory(ram)
target_link_libraries(demo PUBLIC ram)

