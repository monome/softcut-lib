cmake_minimum_required(VERSION 3.7)
project(softcut)

set(CMAKE_CXX_STANDARD 14)

set(SRC
        src/DebugLog.cpp
        src/FadeCurves.cpp
        src/ReadWriteHead.cpp
        src/SubHead.cpp
        src/Svf.cpp
        src/Voice.cpp)

include_directories(include src)

add_library(softcut STATIC ${SRC})
