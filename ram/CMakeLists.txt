project(ram)
set(SOURCE_FILES ram.h ram.cpp)

add_library(${PROJECT_NAME} STATIC ram.h ram.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})