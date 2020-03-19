//
// Created by emb on 3/18/20.
//

#ifndef SOFTCUT_TEST_H
#define SOFTCUT_TEST_H

#include <chrono>
#include "softcut/Softcut.h"


template<size_t numFrames = 480000, size_t bufSize = 1048576, size_t blockSize = 128>
class Test {
public:

    softcut::Softcut<1> cut;
    softcut::TestBuffers testBuffers;

    std::array<float, numFrames> input;
    std::array<float, numFrames> output;
    std::array<float, bufSize> buf;

    size_t frameCount{};
    size_t startMs{};

    Test() {
        cut.setVoiceBuffer(0, buf.data(), bufSize);
    }

    void writeOutputSoundfile(const std::string &path) {
        const int sr = 48000;
        const int channels = 1;
        const int format = SF_FORMAT_WAV | SF_FORMAT_PCM_24;

        SndfileHandle file(path, SFM_WRITE, format, channels, sr);
        if (not file) {
            std::cerr << "writeOutputSoundfile(): cannot open " << path << std::endl;
            return;
        }
        file.command(SFC_SET_CLIPPING, nullptr, SF_TRUE);
        file.writef(this->output.data(), output.size());
    }

    void loadInputSoundFile(const std::string &path) {
        SndfileHandle file(path, SFM_READ);
        if (not file) {
            std::cerr << "loadInputSoundFile(): cannot open " << path << std::endl;
            return;
        }
        file.readf(this->input.data(), file.frames());
    }

    void loadBufferSoundFile(const std::string &path) {
        SndfileHandle file(path, SFM_READ);
        if (not file) {
            std::cerr << "loadBufferSoundFile(): cannot open " << path << std::endl;
            return;
        }
        file.readf(this->buf.data(), file.frames());
    }

    virtual void init() {
        using namespace std::chrono;
        cut.reset();
        frameCount = 0;
    }

    void zeroInput() {
        for (size_t i = 0; i < numFrames; ++i) {
            input[i] = 0.f;
        }
    }

    void zeroBuffer() {
        for (size_t i = 0; i < bufSize; ++i) {
            buf[i] = 0.f;
        }
    }

    void performBlock(float* &src, float* &dst) {
        cut.processBlock(0, src, dst, blockSize);
        testBuffers.update(cut, 0, blockSize);
        src += blockSize;
        dst += blockSize;
        frameCount += blockSize;
    }

    void run() {
        init();

        using namespace std::chrono;
        startMs = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        float *src = input.data();
        float *dst = output.data();
        size_t maxFrames = numFrames - blockSize;
        while (frameCount < maxFrames) {
            performBlock(src, dst);
        }
        finish();
    }

    void finish() {
        using namespace std::chrono;
        auto endMs = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        std::cout << "processed " << numFrames << " frames in " << (endMs - startMs) << " ms" << std::endl;

        cnpy::npy_save("buffer.npy", buf.data(), {1, bufSize}, "w");
        cnpy::npy_save("output.npy", output.data(), {1, numFrames}, "w");

        // FIXME: would be nice to put this in TestBuffers, but that creates a `cnpy` dependency in the lib.
        /// TestBuffers kinda has to be in the lib itself so it can get access to protected fields.
        cnpy::npy_save("rate.npy", testBuffers.getBuffer(softcut::TestBuffers::Rate), {1, numFrames}, "w");
        cnpy::npy_save("dir.npy", testBuffers.getBuffer(softcut::TestBuffers::Rate), {1, numFrames}, "w");
        cnpy::npy_save("active.npy", testBuffers.getBuffer(softcut::TestBuffers::Active), {1, numFrames}, "w");
        cnpy::npy_save("frameInBlock.npy", testBuffers.getBuffer(softcut::TestBuffers::FrameInBlock), {1, numFrames}, "w");
        cnpy::npy_save("phase0.npy", testBuffers.getBuffer(softcut::TestBuffers::Phase0), {1, numFrames}, "w");
        cnpy::npy_save("phase1.npy", testBuffers.getBuffer(softcut::TestBuffers::Phase1), {1, numFrames}, "w");
        cnpy::npy_save("state0.npy", testBuffers.getBuffer(softcut::TestBuffers::State0), {1, numFrames}, "w");
        cnpy::npy_save("state1.npy", testBuffers.getBuffer(softcut::TestBuffers::State1), {1, numFrames}, "w");
        cnpy::npy_save("action0.npy", testBuffers.getBuffer(softcut::TestBuffers::Action0), {1, numFrames}, "w");
        cnpy::npy_save("action1.npy", testBuffers.getBuffer(softcut::TestBuffers::Action1), {1, numFrames}, "w");
        cnpy::npy_save("fade0.npy", testBuffers.getBuffer(softcut::TestBuffers::Fade0), {1, numFrames}, "w");
        cnpy::npy_save("fade1.npy", testBuffers.getBuffer(softcut::TestBuffers::Fade1), {1, numFrames}, "w");
        cnpy::npy_save("rec0.npy", testBuffers.getBuffer(softcut::TestBuffers::Rec0), {1, numFrames}, "w");
        cnpy::npy_save("rec1.npy", testBuffers.getBuffer(softcut::TestBuffers::Rec1), {1, numFrames}, "w");
        cnpy::npy_save("pre0.npy", testBuffers.getBuffer(softcut::TestBuffers::Pre0), {1, numFrames}, "w");
        cnpy::npy_save("pre1.npy", testBuffers.getBuffer(softcut::TestBuffers::Pre1), {1, numFrames}, "w");
        cnpy::npy_save("wrIdx0.npy", testBuffers.getBuffer(softcut::TestBuffers::WrIdx0), {1, numFrames}, "w");
        cnpy::npy_save("wrIdx1.npy", testBuffers.getBuffer(softcut::TestBuffers::WrIdx1), {1, numFrames}, "w");

        writeOutputSoundfile("output.wav");
    }
};

#endif //SOFTCUT_TEST_H
