cmake_minimum_required(VERSION 3.21)
project(demo-ram LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} demo/main.cpp)

add_subdirectory(ram)
target_link_libraries(${PROJECT_NAME} PUBLIC ram)
