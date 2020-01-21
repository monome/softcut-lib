cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 14)

project(softcut_jack_osc)

set(SRC src/main.cpp
        src/SoftcutClient.cpp
        src/Commands.cpp
        src/OscInterface.cpp
        src/BufDiskWorker.cpp
        src/Window.cpp)

add_executable(softcut_jack_osc ${SRC})

include_directories(../../softcut/include)

target_link_libraries(softcut_jack_osc softcut jack lo pthread sndfile)

target_compile_options(softcut_jack_osc PRIVATE -Wall -Wextra -pedantic)

