//
// Created by emb on 3/21/20.
//

#ifndef SOFTCUT_TESTRATESIGNCHANGE_H
#define SOFTCUT_TESTRATESIGNCHANGE_H

template<size_t numFrames = (48000 * 4), size_t bufSize = (1048576 / 2), size_t blockSize = 128>
class TestRateSignChange : public Test<numFrames, bufSize, blockSize> {

    using super = Test<numFrames, bufSize, blockSize>;

    void init() override {
        super::init();
        super::loadInputSoundFile("octave-sines.wav");
        super::loadBufferSoundFile("octave-sines.wav");
        super::cut.setSampleRate(48000);
        super::cut.setFadeTime(0, 0.05);
        super::cut.setLoopStart(0, 0.1);
        super::cut.setLoopEnd(0, 0.2);
        super::cut.setLoopFlag(0, true);
        super::cut.setPlayFlag(0, true);

        super::cut.setPreFilterEnabled(0, false);

        super::cut.setRecFlag(0, true);
        super::cut.setRecLevel(0, 1.0);
        super::cut.setPreLevel(0, 0.75);
        super::cut.setPosition(0, 0.1);

        super::cut.setRateSlewTime(0, 0.0);
        super::cut.setRate(0,  1.0);
        super::cut.setRateSlewTime(0, 2.0);
        super::cut.setRate(0, -1.0);


    }
};

#endif //SOFTCUT_TESTRATESIGNCHANGE_H
