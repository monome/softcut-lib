//
// Created by ezra on 12/6/17.
//

#ifndef CUTFADEVOICE_CUTFADEVOICELOGIC_H
#define CUTFADEVOICE_CUTFADEVOICELOGIC_H

#include <cstdint>
#include "SubHead.h"
#include "Types.h"
#include "TestBuffers.h"
#include "FadeCurves.h"

namespace softcut {

    class ReadWriteHead {

    protected:
        friend class SubHead;
        friend class TestBuffers;
        static constexpr size_t maxBlockSize = SubHead::maxBlockSize;
        template<typename T>
        using StateBuffer = SubHead::StateBuffer<T>;

    private:
        // FIXME: should use a proper queue (e.g. from dsp-kit)
        /// for now, only a single value can be queued,
        /// and a negative value indicates that the queue is empty.
        phase_t enqueuedPosition = -1.0;
        size_t lastNumFrames;

        void enqueuePositionChange(phase_t pos) {
            enqueuedPosition = pos;
        }

        int dequeuePositionChange(size_t fr); //SubHead::FramePositionData &a, SubHead::FramePositionData &b);
        void handleLoopAction(SubHead::OpAction action);

        static sample_t mixFade(sample_t x, sample_t y, float a, float b) {
            // TODO [efficiency]: try low-order polynomial approximation
            return x * sinf(a * (float)M_PI_2) + y * sinf(b * (float) M_PI_2);
        }


    private:
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

        //--- buffered state variables
        // rate, in per-sample position increment (1 == normal)
        SubHead::StateBuffer<rate_t> rate{};
        // preserve and record levels, pre-fade
        SubHead::StateBuffer<float> pre{};
        SubHead::StateBuffer<float> rec{};
        // index of active subhead
        SubHead::StateBuffer<unsigned char> active{};

    public:

        void init(FadeCurves *fc) {
            fadeCurves = fc;
        }

        // queue a position change
        void cutToPos(float seconds);

        // update all position, state, and action buffers for both subheads
        void performSubheadWrites(const float* input, size_t numFrames);
        void performSubheadReads(float* output, size_t numFrames);
        void updateSubheadPositions(size_t numFrames);
        void updateSubheadWriteLevels(size_t numFrames);
        void setLastNumFrames(size_t lnf) { lastNumFrames = lnf; }


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
    };


}
#endif //CUTFADEVOICE_CUTFADEVOICELOGIC_H

/*
namespace softcut {

    class ReadWriteHead {
    public:

        void init(FadeCurves *fc);

        // per-sample update functions
        void processSample(sample_t in, sample_t *out);
        void processSampleNoRead(sample_t in, sample_t *out);
        void processSampleNoWrite(sample_t in, sample_t *out);

        void setSampleRate(float sr);
        void setBuffer(sample_t *buf, uint32_t size);
        void setRate(rate_t x);
        void setLoopStartSeconds(float x);
        void setLoopEndSeconds(float x);
        void setFadeTime(float secs);
        void setLoopFlag(bool val);
        void setRec(float x);
        void setPre(float x);
        void cutToPos(float seconds);

        void setRecOffsetSamples(int d);

        phase_t getActivePhase();
        rate_t getRate();

    protected:
        friend class SubHead;

    private:
        // fade in to new position (given in samples)
        // assumption: phase is in range!
        void cutToPhase(phase_t newPhase);
        void enqueueCrossfade(phase_t newPhase);
        void dequeueCrossfade();
        void takeAction(Action act);

        static sample_t mixFade(sample_t x, sample_t y, float a, float b); // mix two inputs with phases
        void calcFadeInc();

    private:
        SubHead head[2];

        //-------------------
        //--- state variables
        sample_t *buf{};      // audio buffer (allocated elsewhere)
        float sr{};           // sample rate
        phase_t start{};      // start/end points
        phase_t end{};
        phase_t queuedCrossfade{};
        bool queuedCrossfadeFlag{};
        float fadeTime{};     // fade time in seconds
        float fadeInc{};      // linear fade increment per sample

        unsigned char active{};         // current active play head index (0 or 1)
        bool loopFlag{};      // set to loop, unset for 1-shot
        float pre{};      // pre-record level
        float rec{};      // record level

        rate_t rate{};    // current rate
        TestBuffers testBuf;
    };
}
#endif //CUTFADEVOICE_CUTFADEVOICELOGIC_H
*/