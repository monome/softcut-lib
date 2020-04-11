//
// Created by ezra on 11/3/18.
//

#ifndef Softcut_SoftcutVOICE_H
#define Softcut_SoftcutVOICE_H

#include <array>
#include <atomic>

#include "dsp-kit/LadderLpf.hpp"
#include "dsp-kit/Svf.hpp"

#include "ReadWriteHead.h"
#include "Utilities.h"

namespace softcut {
    class Voice {
    protected:
        friend class TestBuffers;

    public:
        Voice();

        void setBuffer(float *buf, size_t numFrames);

        void setSampleRate(float hz);

        void setRate(float rate);

        void setLoopStart(float sec);

        void setLoopEnd(float sec);

        void setLoopFlag(bool val);

        void setFadeTime(float sec);

        void setRecLevel(float amp);

        void setPreLevel(float amp);

        void setRecFlag(bool val);

        void setPlayFlag(bool val);

        void setPreFilterFc(float);

//        void setPreFilterRq(float);
//
//        void setPreFilterLp(float);
//
//        void setPreFilterHp(float);
//
//        void setPreFilterBp(float);
//
//        void setPreFilterBr(float);
//
//        void setPreFilterDry(float);

        void setPreFilterEnabled(bool);
        void setPreFilterFcMod(float x);
        void setPreFilterQ(float x);

        void setPostFilterFc(float);

        void setPostFilterRq(float);

        void setPostFilterLp(float);

        void setPostFilterHp(float);

        void setPostFilterBp(float);

        void setPostFilterBr(float);

        void setPostFilterDry(float);

        void setPosition(float sec);

        // process a single channel
        //void processBlockMono(float *in, float *out, size_t numFrames);
        void updatePositions(size_t numFrames);
        void performReads(float *out, size_t numFrames);
        void performWrites(float *in, size_t numFrames);

        void setRecOffset(float d);

        void setRecPreSlewTime(float d);

        void setRateSlewTime(float d);

        void setPhaseQuant(float x);

        void setPhaseOffset(float x);

        phase_t getQuantPhase();

        bool getPlayFlag() const;

        bool getRecFlag() const;

        float getPos() const;

        void setDuckTarget(Voice* v)  {
            duckTarget = v;
        }

        void reset();

    private:
        void processInputFilter(float* src, float *dst, size_t numFrames);
        void updateQuantPhase();

    private:
        float *buf{};
        size_t bufFrames{};
        float sampleRate{};

        // crossfaded read/write head
        ReadWriteHead rwh;

        // pre-write filter
        dspkit::LadderLpf<float> preFilter{};
        std::array<float, ReadWriteHead::maxBlockSize>  preFilterInputBuf{};
        // post-read filter
        dspkit::Svf postFilter;

        // rate ramp
        LogRamp rateRamp;
        // pre-level ramp
        LogRamp preRamp;
        // record-level ramp
        LogRamp recRamp;

        // default frequency for SVF
        // reduced automatically when setting rate
        float preFilterFcBase;
        // the amount by which SVF frequency is modulated by rate
        float preFilterFcMod = 1.0;
        // dry level at post-playback filtering stage
        float postFilterDryLevel = 1.0;
        // phase quantization unit, in fractional frames
        phase_t phaseQuant{};
        // phase offset in sec
        float phaseOffset = 0;
        // quantized phase
        std::atomic<phase_t> quantPhase{};


    private:
        bool playEnabled{};
        bool recEnabled{};
        bool preFilterEnabled;

        const Voice *duckTarget{nullptr};
        const Voice *followTarget{nullptr};

        void applyDucking(float *out, size_t numFrames);

        static float calcPhaseDuck(double a, double b, float rec, float pre);
    };
}


#endif //Softcut_SoftcutVOICE_H
