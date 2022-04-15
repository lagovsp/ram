cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)

set(HUNTER_PACKAGES dlib)

include(FetchContent)
FetchContent_Declare(SetupHunter GIT_REPOSITORY https://github.com/cpp-pm/gate)
FetchContent_MakeAvailable(SetupHunter)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
FetchContent_MakeAvailable(googletest)

project(demo-ram LANGUAGES CXX)

find_package(dlib REQUIRED)

add_executable(${PROJECT_NAME} demo/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE dlib::dlib)

add_subdirectory(ram)
target_link_libraries(${PROJECT_NAME} PUBLIC ram)

enable_testing()

add_executable(tests tests/test.cpp)
target_link_libraries(tests ram)
target_link_libraries(tests gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
