//
// Created by ezra on 4/21/18.
//

/*
 *  a Sub-Head performs direct access on a monophonic buffer.
 *
 * a SubHead maintains all necessary state variables for `peek` (read) and `poke` (write),
 * including current crossfade status and position,
 * but does not make decisions re: when or if to update them.
 *
 * a ReadWriteHead is responsible for coordinating the operations of multiple SubHeads.
 * some buffered state variables of ReadWriteHead are used by SubHead;
 * these are accessed by pointer from the SubHead.
 */

#ifndef Softcut_SUBHEAD_H
#define Softcut_SUBHEAD_H

#include <array>

#include "Resampler.h"
#include "Types.h"
#include "FadeCurves.h"

namespace softcut {
    class ReadWriteHead;

    // FIXME: template would be nicer
    //template <size_t blockSizeExpected>
    class SubHead {
        friend class ReadWriteHead;

        friend class TestBuffers;

    public:
        SubHead();

    public:
        // operational state and action descriptors
        typedef enum {
            Stopped = 0, FadeIn = 1, Playing = 2, FadeOut = 3
        } OpState;
        typedef enum {
            None, Stop, StartFadeIn, LoopPositive, LoopNegative, DoneFadeIn, DoneFadeOut,
        } OpAction;

        static constexpr size_t maxBlockSize = 1024;
        template<typename T>
        using StateBuffer = std::array<T, maxBlockSize>;
        using frame_t = long int;

    protected:
        //--- buffered state variables, owned:
        // is operating
        StateBuffer<bool> active{false};
        // current state of operation
        StateBuffer<OpState> opState{Stopped};
        // last action performed
        StateBuffer<OpAction> opAction{None};
        // current "logical" buffer position, in units of time
        StateBuffer<phase_t> phase{0.0};
        // last write index in buffer
        StateBuffer<frame_t> wrIdx{0};

        // current read/write increment direction
        StateBuffer<int> dir{1};

        // current fade position in [0,1]
        StateBuffer<float> fade{0.f};
        // final preserve level, post-fade
        StateBuffer<float> pre{0.f};
        // final record level, post-fade
        StateBuffer<float> rec{0.f};

    protected:
        void setPosition(size_t idx_1, size_t idx, phase_t position, const softcut::ReadWriteHead *rwh);

        // update phase, opState, and opAction
        OpAction calcPositionUpdate(size_t i_1, size_t i, const softcut::ReadWriteHead *rwh);

        // update frame level data
        void calcLevelUpdate(size_t i, const softcut::ReadWriteHead *rwh);

        // perform single frame write
        void performFrameWrite(size_t i_1, size_t i, float input);

        // read a single frame
        float performFrameRead(size_t i);

        void setBuffer(float *b, size_t fr) {
            this->buf = b;
            this->bufFrames = fr;
        }

        frame_t wrapBufIndex(frame_t x) {
            frame_t y = x;
            // FIXME: should wrap to loop endpoints, maybe
            while (y > bufFrames) { y -= bufFrames; }
            while (y < 0) { y += bufFrames; }
            return y;
        }

        void updateRate(size_t idx, rate_t rate);

    private:
        Resampler resamp;   // resampler
        float *buf{};         // current audio buffer
        frame_t bufFrames{};   // total buffer size

        //// debug
        bool didSetPositionThisFrame  {false};

        void init(FadeCurves *pCurves);
    };
}

#endif // Softcut_SUBHEAD_H
