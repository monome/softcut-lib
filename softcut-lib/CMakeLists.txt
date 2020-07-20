cmake_minimum_required(VERSION 3.7)
project(softcut)

set(CMAKE_CXX_STANDARD 17)

set(SRC
        src/ReadWriteHead.cpp
        src/SubHead.cpp
        src/Voice.cpp
        lib/dsp-kit/Envelope.cpp
        lib/dsp-kit/Svf.cpp
        lib/dsp-kit/Taper.cpp
        lib/dsp-kit/FastFader.cpp
        lib/dsp-kit/FastMover.cpp)

include_directories(include src lib)

add_library(softcut STATIC ${SRC})
