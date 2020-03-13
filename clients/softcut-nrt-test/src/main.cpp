//
// Created by emb on 3/13/20.
//

#include <array>
#include <iostream>

#include "cnpy/cnpy.h"

#include "softcut/Softcut.h"
#include "softcut/TestBuffers.h"

static constexpr double twopi =  3.1415926535898;
static constexpr int sr = 48000;
static constexpr size_t nf = sr * 2;
static constexpr size_t bufsize = sr * 10;

static std::array<float, nf> input;
static std::array<float, nf> output;
static std::array<float, bufsize> buf;

int main(int argc, const char **argv) {
    //--- fill buffer with sinewave
    double hz = 110.0;
    double inc =  hz / (double)sr;
    double phase = 0.0;
    for (float & f : buf) {
        f = sin((float)phase * twopi);
        phase += inc;
        while (phase > 1.0) { phase -= 1.0; }
    }

    softcut::Softcut<1> cut;
    softcut::TestBuffers testBuffers;
    cut.reset();

    cut.setVoiceBuffer(0, buf.data(), bufsize);
    cut.setSampleRate(sr);
    cut.setRate(0, 1.0);
    cut.setFadeTime(0, 0.1);
    cut.setLoopStart(0, 1.0);
    cut.setLoopEnd(0, 3.0);
    cut.setPlayFlag(0, true);

    size_t blocksize = 256;
    size_t maxframes = nf - blocksize;
    size_t fr = 0;
    float *src = input.data();
    float *dst = output.data();

    while (fr < maxframes) {
        cut.processBlock(0, src, dst, blocksize);
        testBuffers.update(cut, 0, blocksize);
        src += blocksize;
        dst += blocksize;
        fr += blocksize;
    }

    std::cout << "output = { " << std::endl;
    for (float i : output) {
        std::cout << i << ", ";
    }
    std::cout << std::endl << "} ;" << std::endl;

    cnpy::npy_save("rate.npy", testBuffers.getBuffer(softcut::TestBuffers::Rate), {1, nf}, "w");
    cnpy::npy_save("phase0.npy", testBuffers.getBuffer(softcut::TestBuffers::Phase0), {1, nf}, "w");
    cnpy::npy_save("phase1.npy", testBuffers.getBuffer(softcut::TestBuffers::Phase1), {1, nf}, "w");
    cnpy::npy_save("state0.npy", testBuffers.getBuffer(softcut::TestBuffers::State0), {1, nf}, "w");
    cnpy::npy_save("state1.npy", testBuffers.getBuffer(softcut::TestBuffers::State1), {1, nf}, "w");
    cnpy::npy_save("action0.npy", testBuffers.getBuffer(softcut::TestBuffers::Action0), {1, nf}, "w");
    cnpy::npy_save("action1.npy", testBuffers.getBuffer(softcut::TestBuffers::Action1), {1, nf}, "w");
    cnpy::npy_save("fade0.npy", testBuffers.getBuffer(softcut::TestBuffers::Fade0), {1, nf}, "w");
    cnpy::npy_save("fade1.npy", testBuffers.getBuffer(softcut::TestBuffers::Fade1), {1, nf}, "w");
    cnpy::npy_save("rec0.npy", testBuffers.getBuffer(softcut::TestBuffers::Rec0), {1, nf}, "w");
    cnpy::npy_save("rec1.npy", testBuffers.getBuffer(softcut::TestBuffers::Rec1), {1, nf}, "w");
    cnpy::npy_save("pre0.npy", testBuffers.getBuffer(softcut::TestBuffers::Pre0), {1, nf}, "w");
    cnpy::npy_save("pre1.npy", testBuffers.getBuffer(softcut::TestBuffers::Pre1), {1, nf}, "w");
    cnpy::npy_save("wrIdx0.npy", testBuffers.getBuffer(softcut::TestBuffers::WrIdx0), {1, nf}, "w");
    cnpy::npy_save("wrIdx1.npy", testBuffers.getBuffer(softcut::TestBuffers::WrIdx1), {1, nf}, "w");


}