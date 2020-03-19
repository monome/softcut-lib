//
// Created by emb on 3/18/20.
//

#ifndef SOFTCUT_TESTLOOPINPLACE_H
#define SOFTCUT_TESTLOOPINPLACE_H

#include "Test.h"

template<size_t numFrames = (48000 * 10), size_t bufSize = (1048576 / 2), size_t blockSize = 512>
class TestLoopInPlace : public Test<numFrames, bufSize, blockSize> {

    using super = Test<numFrames, bufSize, blockSize>;

    void init() override {
        super::init();
        super::zeroInput();
        super::loadBufferSoundFile("octave-sines.wav");
        super::cut.setSampleRate(48000);
        super::cut.setRate(0, 2 * 1.1224620483094); // 2 semitones 12tet
        super::cut.setFadeTime(0, 0.05);
        super::cut.setLoopStart(0, 0.2);
        super::cut.setLoopEnd(0, 0.6);
        super::cut.setLoopFlag(0, true);
        super::cut.setPlayFlag(0, true);

        super::cut.setRecFlag(0, true);
        super::cut.setRecLevel(0, 1.0);
        super::cut.setPreLevel(0, 0.75);
        super::cut.setPosition(0.1, 0.0);
    }
};

#endif //SOFTCUT_TESTLOOPINPLACE_H
