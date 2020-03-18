//
// Created by emb on 3/13/20.
//

#include <array>
#include <iostream>
#include <chrono>

#include <sndfile.hh>
#include "cnpy/cnpy.h"

#include "softcut/Softcut.h"
#include "softcut/TestBuffers.h"
#include "Test.h"
#include "TestLoopInPlace.h"


int main(int argc, const char **argv) {
    (void) argc;
    (void) argv;

    TestLoopInPlace test;
    test.run();

}

