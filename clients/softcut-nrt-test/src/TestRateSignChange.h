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

        // voice 0
        super::cut.voice(0)->setFadeTime(0.05);
        super::cut.voice(0)->setLoopStart(0.1);
        super::cut.voice(0)->setLoopEnd(0.2);
        super::cut.voice(0)->setLoopFlag(true);
        super::cut.voice(0)->setPlayFlag(true);

        super::cut.voice(0)->setPreFilterEnabled(false);

        super::cut.voice(0)->setRecFlag(true);
        super::cut.voice(0)->setRecLevel(1.0);
        super::cut.voice(0)->setPreLevel(0.75);
        super::cut.voice(0)->setPosition(0.1);

        super::cut.voice(0)->setRateSlewTime(0.0);
        super::cut.voice(0)->setRate(1.0);
        super::cut.voice(0)->setRateSlewTime(2.0);
        super::cut.voice(0)->setRate(-1.0);



    }
};

#endif //SOFTCUT_TESTRATESIGNCHANGE_H
