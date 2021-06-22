cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)

project(softcut_nrt_test)


add_executable(softcut_nrt_test src/main.cpp)
add_executable(test-fade-curves src/test-fade-curves.cpp)

include_directories(./lib)
include_directories(../../softcut-lib/include)
include_directories(../../softcut-lib/lib)
include_directories(../../softcut-lib/lib/dsp-kit/lib)

target_link_libraries(softcut_nrt_test softcut sndfile cnpy)

target_compile_options(softcut_nrt_test PRIVATE -Wall -Wextra -pedantic)