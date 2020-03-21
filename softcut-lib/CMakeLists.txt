cmake_minimum_required(VERSION 3.7)
project(softcut)

set(CMAKE_CXX_STANDARD 14)

set(SRC
        src/DebugLog.cpp
        src/ReadWriteHead.cpp
        src/SubHead.cpp
        src/Voice.cpp
        lib/dsp-kit/Svf.cpp)

include_directories(include src lib)

add_library(softcut STATIC ${SRC})
