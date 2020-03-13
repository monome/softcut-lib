//
// Created by emb on 3/13/20.
//

#include <array>
#include <iostream>

#include "softcut/Softcut.h"
#include "softcut/Voice.h"

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
        src += blocksize;
        dst += blocksize;
        fr += blocksize;
    }

    std::cout << "output = { " << std::endl;
    for (float i : output) {
        std::cout << i << ", ";
    }
    std::cout << std::endl << "} ;" << std::endl;
}