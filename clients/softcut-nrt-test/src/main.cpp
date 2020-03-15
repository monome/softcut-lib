//
// Created by emb on 3/13/20.
//

#include <array>
#include <iostream>

#include <sndfile.hh>
#include "cnpy/cnpy.h"

#include "softcut/Softcut.h"
#include "softcut/TestBuffers.h"

static constexpr double pi = 3.1415926535898;
static constexpr double twopi = 6.2831853071796;
static constexpr int sr = 48000;
static constexpr size_t nf = sr * 10;
static constexpr size_t bufsize = sr * 4;

static std::array<float, nf> input;
static std::array<float, nf> output;
static std::array<float, bufsize> buf;

void writeOutputSoundfile() {
    const std::string path = "output.wav";
    const int sr = 48000;
    const int channels = 1;
    const int format = SF_FORMAT_WAV | SF_FORMAT_PCM_24;

    SndfileHandle file(path, SFM_WRITE, format, channels, sr);
    if (not file) {
        std::cerr << "writeOutput(): cannot open sndfile" << path << " for writing" << std::endl;
        return;
    }

    file.command(SFC_SET_CLIPPING, NULL, SF_TRUE);
    file.writef(output.data(), output.size());
}

int main(int argc, const char **argv) {
    (void) argc;
    (void) argv;

    //--- fill buffer with sinewave
    double hz = 110.0;
    double inc = hz / (double) sr;
    double phase = 0.0;
    for (float &f : buf) {
        f = sinf((float) phase * twopi) * 0.5f;
        //std::cerr << "["<< phase << ","<< f << "], ";
        phase += inc;
        while (phase > 1.0) { phase -= 1.0; }
    }
    std::cerr << std::endl;

    softcut::Softcut<1> cut;
    softcut::TestBuffers testBuffers;
    cut.reset();

    cut.setVoiceBuffer(0, buf.data(), bufsize);
    cut.setSampleRate(sr);
    cut.setRate(0, 1.0);
    cut.setFadeTime(0, 0.05);
    cut.setLoopStart(0, 1);
    cut.setLoopEnd(0, 3);
    cut.setLoopFlag(0, true);
    cut.setPlayFlag(0, true);
    cut.setPosition(0, 0.5);

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
        std::cerr << "processed " << fr << " frames" << std::endl;
    }

//    std::cout << "output = { " << std::endl;
//    for (float i : output) {
//        std::cout << i << ", ";
//    }
//    std::cout << std::endl << "} ;" << std::endl;

    cnpy::npy_save("buffer.npy", buf.data(), {1, bufsize}, "w");
    cnpy::npy_save("output.npy", output.data(), {1, nf}, "w");
    cnpy::npy_save("rate.npy", testBuffers.getBuffer(softcut::TestBuffers::Rate), {1, nf}, "w");
    cnpy::npy_save("active.npy", testBuffers.getBuffer(softcut::TestBuffers::Active), {1, nf}, "w");
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
    cnpy::npy_save("dir0.npy", testBuffers.getBuffer(softcut::TestBuffers::Dir0), {1, nf}, "w");
    cnpy::npy_save("dir1.npy", testBuffers.getBuffer(softcut::TestBuffers::Dir1), {1, nf}, "w");

    writeOutputSoundfile();

}