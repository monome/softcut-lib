cmake_minimum_required(VERSION 3.7)

project(softcut)
add_subdirectory(softcut-lib)
add_subdirectory(clients/softcut-jack-osc)
add_subdirectory(clients/softcut-nrt-test)
add_subdirectory(softcut-lib/lib/dsp-kit/lib/cnpy)
