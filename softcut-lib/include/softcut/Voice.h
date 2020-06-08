//
// Created by ezra on 11/3/18.
//

#ifndef Softcut_SoftcutVOICE_H
#define Softcut_SoftcutVOICE_H

#include <array>
#include <atomic>

#include "dsp-kit/DcBlocker.hpp"
#include "dsp-kit/LadderLpf.hpp"
#include "dsp-kit/Smoother.hpp"
#include "dsp-kit/Svf.hpp"
#include "dsp-kit/FastMover.hpp"
#include "dsp-kit/FastFader.hpp"

#include "ReadWriteHead.h"

namespace softcut {
    class Voice {
    protected:
        friend class TestBuffers;

    public:
        Voice();

        //----------------------------------
        // decomposed single-channel process functions
        void updatePositions(size_t numFrames);
        void performReads(float *out, size_t numFrames);
        void performWrites(float *in, size_t numFrames);
        //-----------------------------------------

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

        void setPreFilterEnabled(bool);

        void setPreFilterFcMod(float x);

        void setPreFilterQ(float x);

        void setPostFilterEnabled(bool);

        void setPostFilterFc(float);

        void setPostFilterRq(float);

        void setPostFilterLp(float);

        void setPostFilterHp(float);

        void setPostFilterBp(float);

        void setPostFilterBr(float);

        void setPostFilterDry(float);

        void setPosition(float sec);

        void setRecOffset(float d);

        //--- slew times / shapes

        // level slews have fixed shape (audio taper)
        void setLevelSlewTime(float value);
        void setRecPreSlewTime(float d);

        // rate has variable shape
        void setRateSlewTime(float t);
        void setRateSlewShape(int shape);

        // filter params have separate rise/fall shapes
        void setPostFilterFcSlewTime(float t);
        void setPostFilterRqSlewTime(float t);
        void setPostFilterRqRiseShape(int shape);
        void setPostFilterRqFallShape(int shape);
        void setPostFilterFcRiseShape(int shape);
        void setPostFilterFcFallShape(int shape);

        void setPhaseQuant(float x);
        void setPhaseOffset(float x);
       
        void updateQuantPhase();
        phase_t getQuantPhase();

        bool getPlayFlag() const;
        bool getRecFlag() const;

        void setReadDuckTarget(Voice* v);
        void setWriteDuckTarget(Voice* v);

        void setFollowTarget(Voice* v);
        void syncPosition(const Voice &v, float offset);

        void reset();

    private:
        void processInputFilter(float* src, float *dst, size_t numFrames);
        void processOutputFilter(float* buf, size_t numFrames);


    private:
        // audio buffer
        float *buf{};
        // size of buffer in frames
        size_t bufFrames{};
        // audio sample rate
        float sampleRate{};

        // crossfaded read/write head
        ReadWriteHead rwh;

        // pre-write filter
        dspkit::LadderLpf<float> preFilter;
        dspkit::DcBlocker<float> dcBlocker;
        std::array<float, ReadWriteHead::maxBlockSize> preFilterInputBuf{0};
        // post-read filter
        dspkit::Svf postFilter;

        // rate ramp
        dspkit::FastMover rateRamp;
        // pre-level ramp
        dspkit::FastFader preRamp;
        // record-level ramp
        dspkit::FastFader recRamp;

        // post-filter mix and parameter ramps
        enum { SVF_LP, SVF_HP, SVF_BP, SVF_BR, SVF_DRY, SVF_OUTPUTS };
        dspkit::FastFader postFilterLevelRamp[SVF_OUTPUTS];
        dspkit::FastMover postFilterFcRamp;
        dspkit::FastMover postFilterRqRamp;



        // base cutoff frequency as normalized pitch
        float preFilterFcBase = 1.0;
        // amount by which SVF frequency is modulated by rate
        float preFilterFcMod = 1.0;
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
        bool postFilterEnabled;

        std::atomic<const Voice *> readDuckTarget{nullptr};
        std::atomic<const Voice *> writeDuckTarget{nullptr};
        std::atomic<const Voice *> followTarget{nullptr};
    };
}


#endif //Softcut_SoftcutVOICE_H
