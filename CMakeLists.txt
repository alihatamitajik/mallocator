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

add_executable(testapp "./src/main.cpp" ${SOURCES})