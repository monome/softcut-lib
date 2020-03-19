//
// Created by ezra on 12/6/17.
//

#ifndef SOFTCUT_READWRITEHEAD_H
#define SOFTCUT_READWRITEHEAD_H

#include <cstdint>
#include "SubHead.h"
#include "Types.h"
#include "FadeCurves.h"

namespace softcut {

    class ReadWriteHead {

    protected:
        friend class SubHead;
        friend class TestBuffers;

        static constexpr size_t maxBlockSize = SubHead::maxBlockSize;
        template<typename T>
        using StateBuffer = SubHead::StateBuffer<T>;
        using frame_t = SubHead::frame_t;

    private:
        // FIXME: should maybe use a proper queue (e.g. from dsp-kit)
        /// for now, only a single value can be queued,
        /// and a negative value indicates that the queue is empty.
        phase_t enqueuedPosition = -1.0;
        size_t frameIdx; // last used index into processing block

        void enqueuePositionChange(phase_t pos);
        int dequeuePositionChange(size_t fr_1, size_t fr);
        void handleLoopAction(SubHead::OpAction action);

        static sample_t mixFade(sample_t x, sample_t y, float a, float b) {
            // we don't actually want equal power since we are summing!
            return x*a + y*b;
        }


    protected:
        SubHead head[2];         // sub-processors
        FadeCurves *fadeCurves{nullptr}; // fade-curve data (allocated elsewhere)

        //-------------------
        //--- state variables (unbuffered)
        sample_t *buf{nullptr}; // audio buffer (allocated elsewhere)
        size_t bufFrames{0};    // size of buffer in frames
        float sr{0};            // sample rate
        phase_t start{0};       // start/end points
        phase_t end{0};
        float fadeTime{0};      // fade time in seconds

        float fadeInc{0};       // linear fade increment per sample
        bool loopFlag{0};       // set to loop, unset for 1-shot
        int recOffsetSamples{-8}; // record offset from write head
        frame_t fadeOutFrameBeforeLoop;
        frame_t fadeOutFrameAfterLoop;

        //--- buffered state variables
        // rate, in per-sample position increment (1 == normal)
        SubHead::StateBuffer<rate_t> rate{1.f};
        // preserve and record levels, pre-fade
        SubHead::StateBuffer<float> pre{0.f};
        SubHead::StateBuffer<float> rec{0.f};
        // index of active subhead
        SubHead::StateBuffer<int> active{-1};

    public:

        ReadWriteHead();

        void init(FadeCurves *fc);

        // queue a position change
        void setPosition(float seconds);

        // update all position, state, and action buffers for both subheads
        void performSubheadWrites(const float *input, size_t numFrames);

        void performSubheadReads(float *output, size_t numFrames);

        void updateSubheadPositions(size_t numFrames);

        void updateSubheadWriteLevels(size_t numFrames);


//        // per-sample update functions
//        void processSample(sample_t in, sample_t *out);
//        void processSampleNoRead(sample_t in, sample_t *out);
//        void processSampleNoWrite(sample_t in, sample_t *out);

        void setSampleRate(float sr);

        void setBuffer(sample_t *buf, uint32_t size);

        void setLoopStartSeconds(float x);

        void setLoopEndSeconds(float x);

        void setFadeTime(float secs);

        void setLoopFlag(bool val);

//        //-- set buffered state for all frames
//        void setRate(rate_t x);
//        void setRec(float x);
//        void setPre(float x);

        //-- set buffered state for single frame
        void setRate(size_t idx, rate_t x);

        void setRec(size_t idx, float x);

        void setPre(size_t idx, float x);

        void setRecOffsetSamples(int d);

        phase_t getActivePhase();

        phase_t wrapPhaseToLoop(phase_t p);

        frame_t wrapFrameToLoopFade(frame_t w);
    };


}
#endif //SOFTCUT_READWRITEHEAD_H
