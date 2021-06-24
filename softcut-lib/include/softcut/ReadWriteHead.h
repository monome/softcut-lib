//
// Created by ezra on 12/6/17.
//

#ifndef SOFTCUT_READWRITEHEAD_H
#define SOFTCUT_READWRITEHEAD_H

#include <atomic>
#include <cstdint>

#include "dsp-kit/Smoother.hpp"

#include "SubHead.h"
#include "Types.h"

namespace softcut {

    class ReadWriteHead {

    protected:
        friend class Voice;

        friend class SubHead;

        friend class TestBuffers;

        template<typename T>
        using StateBuffer = SubHead::StateBuffer<T>;
        using frame_t = SubHead::frame_t;

    private:
        // set by the OSC server thread.
        // if non-negative, an explicit position change request uis pending
        std::atomic<phase_t> requestedPosition = -1.0;
        phase_t enqueuedPosition = -1.0;
        size_t frameIdx; // last used index into processing block
//
//        void enqueuePositionChange(phase_t pos);
//
//        int dequeuePositionChange(size_t fr);
//
//        void checkPositionChange(frame_t fr_1, frame_t fr);

        // attempt to perform any pending position change requests
        // return the index of the subhead that just became active,
        // or -1 if nothing happened
        int checkPositionRequest(size_t fr_1, size_t fr);

        void jumpToPosition(int newHeadIdx, size_t fr, phase_t pos);
        void loopToPosition(int oldHeadIdx, size_t fr, phase_t pos);


        static sample_t mixFade(sample_t x, sample_t y, float a, float b) {
            // we don't actually want equal power since we are summing!
            return x * a + y * b;
        }

        static float computeReadDuckLevel(const SubHead *a, const SubHead *b, size_t frame);

        static float computeWriteDuckLevel(const SubHead *a, const SubHead *b, size_t frame);


    protected:
        SubHead head[3];         // sub-processors
        //-------------------
        //--- state variables (unbuffered)
        sample_t *buf{nullptr}; // audio buffer (allocated elsewhere)
        float sr{0};            // sample rate
        phase_t start{0};       // start/end points
        phase_t end{0};

        float fadeInc{0};       // linear fade increment per sample
        bool loopFlag{false};       // set to loop, unset for 1-shot
        int recOffsetSamples{-8}; // record offset from write head

        //--- buffered state variables
        // rate, in per-sample position increment (1 == normal)
        SubHead::StateBuffer<rate_t> rate{1.f};
        SubHead::StateBuffer<int> dir{1};
        // preserve and record levels, pre-fade
        SubHead::StateBuffer<float> pre{0.f};
        SubHead::StateBuffer<float> rec{0.f};

        // bitfield of active subhead
        SubHead::StateBuffer<int> active{-1};

        // ducking levels. 0 == no ducking, 1 == full ducking
        SubHead::StateBuffer<float> readDuck{0};
        SubHead::StateBuffer<float> writeDuck{0};

        // smoothers for ducking
#if 0
        dspkit::AudioLevelSmoother readDuckRamp;
        dspkit::AudioLevelSmoother writeDuckRamp;
#else
        dspkit::OnePoleSmoother<float> readDuckRamp;
        dspkit::OnePoleSmoother<float> writeDuckRamp;
#endif

    public:

        ReadWriteHead();

        void init();

        // request a position change
        void requestPosition(float seconds);

        // update all position, state, and action buffers for both subheads
        void performSubheadWrites(const float *input, size_t numFrames);

        void performSubheadReads(float *output, size_t numFrames);

        void updateSubheadPositions(size_t numFrames);

        void copySubheadPositions(const ReadWriteHead &other, size_t numFrames);

        void updateSubheadWriteLevels(size_t numFrames);

        void computeReadDuckLevels(const ReadWriteHead *other, size_t numFrames);

        // read ducking affects output level
        void applyReadDuckLevels(float *output, size_t numFrames);

        void computeWriteDuckLevels(const ReadWriteHead *other, size_t numFrames);

        // write ducking affects internal rec/pre buffer
        void applyWriteDuckLevels(size_t numFrames);

        void setSampleRate(float sr);

        void setBuffer(sample_t *buf, uint32_t size);

        void setLoopStartSeconds(float x);

        void setLoopEndSeconds(float x);

        void setFadeTime(float secs);

        void setLoopFlag(bool val);

        //-- set buffered state for single frame
        void setRate(size_t idx, rate_t x);

        void setRec(size_t idx, float x);

        void setPre(size_t idx, float x);

        void setRecOffsetSamples(int d);

        phase_t getActivePhase() const;

        phase_t wrapPhaseToLoop(phase_t p);

        static constexpr size_t maxBlockSize = SubHead::maxBlockSize;

        float getRateBuffer(size_t i);
    };


}
#endif //SOFTCUT_READWRITEHEAD_H
