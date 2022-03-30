add_library(ram_lib STATIC ram.h ram.cpp)
target_include_directories(ram_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
