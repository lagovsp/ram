add_library(ram STATIC ram.h ram.cpp)
target_include_directories(ram PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
