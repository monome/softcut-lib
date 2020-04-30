cmake_minimum_required(VERSION 3.7)
project(softcut)

set(CMAKE_CXX_STANDARD 14)

set(SRC
        src/Voice.cpp
        src/ReadWriteHead.cpp
        src/SubHead.cpp
        src/FadeCurves.cpp
        src/Svf.cpp)

include_directories(include src)

if(APPLE)
    include_directories(/usr/local/include)
endif()

add_library(softcut STATIC ${SRC})
