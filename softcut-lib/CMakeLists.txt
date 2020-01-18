cmake_minimum_required(VERSION 3.7)
project(softcut_lib)

set(CMAKE_CXX_STANDARD 14)

set(SRC src/FadeCurves.cpp
        src/Softcut.cpp
        src/SoftcutHead.cpp
        src/SoftcutVoice.cpp
  src/SubHead.cpp
  src/Svf.cpp)

include_directories(include src)

add_library(softcut_lib STATIC ${SRC})
