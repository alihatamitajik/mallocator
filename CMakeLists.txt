cmake_minimum_required(VERSION 3.20)
project(malloc)

set(CMAKE_BUILD_TYPE Debug)

include_directories("./include")
set(SOURCES
"./src/buddy.c"
"./src/firstfit.c"
"./include/buddy.h"
"./include/firstfit.h"
"./include/myalloc.h"
)

# Get GTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test Executables
add_executable(
    MyAllocTest 
    "./tests/MyAllocTest.cc" 
    ${SOURCES}
)
target_link_libraries(
  MyAllocTest
  GTest::gtest_main
)


include(GoogleTest)
gtest_discover_tests(MyAllocTest)

# Main Execurtable
add_executable(testapp "./src/main.cpp" ${SOURCES})