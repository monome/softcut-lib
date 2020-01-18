//
// Created by emb on 11/10/18.
//

#ifndef Softcut_Softcut_H
#define Softcut_Softcut_H


#include <thread>

#include "../src/SoftcutVoice.h"

namespace softcut {

    template<int numVoices>
    class Softcut {

    private:
        SoftcutVoice scv[numVoices];

        void init();

    public:
        Softcut();

        void reset();

        // assumption: channel count is equal to voice count!
        void processBlock(int v, const float *in, float *out, int numFrames);

        void setSampleRate(unsigned int hz);

        void setRate(int voice, float rate);

        void setLoopStart(int voice, float sec);

        void setLoopEnd(int voice, float sec);

        void setLoopFlag(int voice, bool val);

        void setFadeTime(int voice, float sec);

        void setRecLevel(int voice, float amp);

        void setPreLevel(int voice, float amp);

        void setRecFlag(int voice, bool val);

        void setPlayFlag(int voice, bool val);

        void cutToPos(int voice, float sec);

        void setPreFilterFc(int voice, float x);

        void setPreFilterRq(int voice, float x);

        void setPreFilterLp(int voice, float x);

        void setPreFilterHp(int voice, float x);

        void setPreFilterBp(int voice, float x);

        void setPreFilterBr(int voice, float x);

        void setPreFilterDry(int voice, float x);

        void setPreFilterFcMod(int voice, float x);

        void setPostFilterFc(int voice, float x);

        void setPostFilterRq(int voice, float x);

        void setPostFilterLp(int voice, float x);

        void setPostFilterHp(int voice, float x);

        void setPostFilterBp(int voice, float x);

        void setPostFilterBr(int voice, float x);

        void setPostFilterDry(int voice, float x);

#if 0 // not allowing realtime manipulation of fade logic params
        void setPreFadeWindow(float x);

        void setRecFadeDelay(float x);

        void setPreFadeShape(float x);
        }

        void setRecFadeShape(float x);
#endif

        void setRecOffset(int i, float d);

        void setLevelSlewTime(int i, float d);

        void setRecPreSlewTime(int i, float d);

        void setRateSlewTime(int i, float d);

        phase_t getQuantPhase(int i);

        void setPhaseQuant(int i, phase_t q);

        void setPhaseOffset(int i, float sec);

        bool getRecFlag(int i);

        bool getPlayFlag(int i);

        void syncVoice(int follow, int lead, float offset);

        void setVoiceBuffer(int id, float *buf, size_t bufFrames);
    };
}


#endif //Softcut_Softcut_H2
