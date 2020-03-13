cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)

project(softcut_nrt_test)

set(SRC src/main.cpp)

add_executable(softcut_nrt_test ${SRC})

include_directories(./lib)
include_directories(../../softcut-lib/include)

add_subdirectory(lib/cnpy)

target_link_libraries(softcut_nrt_test softcut sndfile cnpy)

target_compile_options(softcut_nrt_test PRIVATE -Wall -Wextra -pedantic)