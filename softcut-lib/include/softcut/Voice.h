//
// Created by ezra on 11/3/18.
//

#ifndef Softcut_SoftcutVOICE_H
#define Softcut_SoftcutVOICE_H

#include <array>
#include <atomic>

#include "ReadWriteHead.h"
#include "Svf.h"
#include "Utilities.h"
#include "FadeCurves.h"

namespace softcut {
    class Voice {
    public:
        Voice();

        void init(FadeCurves *fc);

        void setBuffer(float *buf, unsigned int numFrames);

        void setSampleRate(float hz);

        void setRate(float rate);

        void setLoopStart(float sec);

        void setLoopEnd(float sec);

        void setLoopFlag(bool val);

        void setFadeTime(float sec);

        void setRecLevel(float amp);

        void setPreLevel(float amp);

        void setRecFlag(bool val);

        void setRecOnceFlag(bool val);

        void setPlayFlag(bool val);

        void setPreFilterFc(float);

        void setPreFilterRq(float);

        void setPreFilterLp(float);

        void setPreFilterHp(float);

        void setPreFilterBp(float);

        void setPreFilterBr(float);

        void setPreFilterDry(float);

        void setPreFilterFcMod(float x);

        void setPostFilterFc(float);

        void setPostFilterRq(float);

        void setPostFilterLp(float);

        void setPostFilterHp(float);

        void setPostFilterBp(float);

        void setPostFilterBr(float);

        void setPostFilterDry(float);

        void cutToPos(float sec);

        // process a single channel
        void processBlockMono(const float *in, float *out, int numFrames);

        void setRecOffset(float d);

        void setRecPreSlewTime(float d);

        void setRateSlewTime(float d);

        void setPhaseQuant(float x);

        void setPhaseOffset(float x);

        phase_t getQuantPhase();

        bool getPlayFlag();

        bool getRecFlag();

	float getActivePosition();

	// use this from non-audio threads
        float getSavedPosition();

        void reset();

	// immediately put both subheads in a stopped state
	void stop();

    private:
        void updatePreSvfFc();

        void updateQuantPhase();

    private:
        float *buf;
        int bufFrames;
        float sampleRate;

        // fade curve data
        FadeCurves fadeCurves;
        // xfaded read/write head
        ReadWriteHead sch;
        // input filter
        Svf svfPre;
        // output filter
        Svf svfPost;
        // rate ramp
        LogRamp rateRamp;
        // pre-level ramp
        LogRamp preRamp;
        // record-level ramp
        LogRamp recRamp;


        // default frequency for SVF
        // reduced automatically when setting rate
        float svfPreFcBase;
        // the amount by which SVF frequency is modulated by rate
        float svfPreFcMod = 1.0;
        float svfPreDryLevel = 1.0;
        float svfPostDryLevel = 1.0;
        // phase quantization unit, should be in [0,1]
        phase_t phaseQuant;
        // phase offset in sec
        float phaseOffset = 0;
	
	//-- these stored phases are for access from non-audio threads,
	// and are updated once per block:
	std::atomic<phase_t> rawPhase;
        std::atomic<phase_t> quantPhase;

    private:

        bool playFlag;
        bool recFlag;

    };
}


#endif //Softcut_SoftcutVOICE_H
