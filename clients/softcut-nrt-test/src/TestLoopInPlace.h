//
// Created by emb on 3/18/20.
//

#ifndef SOFTCUT_TESTLOOPINPLACE_H
#define SOFTCUT_TESTLOOPINPLACE_H

#include "Test.h"

template<size_t numFrames = (48000 * 2), size_t bufSize = 1048576 / 2, size_t blockSize = 64>
class TestLoopInPlace : public Test<numFrames, bufSize, blockSize> {

    using super = Test<numFrames, bufSize, blockSize>;

    void init() override {
        super::init();
        super::cut.setSampleRate(48000);
        super::cut.setRate(0, 2.77);
        //super::cut.setRate(0, 1.0);
        super::cut.setFadeTime(0, 0.05);
        super::cut.setLoopStart(0, 1.2);
        super::cut.setLoopEnd(0, 1.6);
        super::cut.setLoopFlag(0, true);
        super::cut.setPlayFlag(0, true);

        super::cut.setRecFlag(0, true);
        super::cut.setRecLevel(0, 1.0);
        super::cut.setPreLevel(0, 0.75);

        super::cut.setPosition(0, 1.0);
    }
};

#endif //SOFTCUT_TESTLOOPINPLACE_H
