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
            None = 0,
            StartFadeIn = 1, DoneFadeIn = 2,
            LoopPositive = 3, LoopNegative = 4, FadeOutAndStop = 5,
            DoneFadeOut = 6
        } OpAction;

        static constexpr size_t maxBlockSize = 1024;
        template<typename T>
        using StateBuffer = std::array<T, maxBlockSize>;
        using frame_t = long int;

    protected:
        ReadWriteHead *rwh{};
        //--- buffered state variables, owned:
        // current state of operation
        StateBuffer<OpState> opState{Stopped};
        // last action performed
        StateBuffer<OpAction> opAction{None};
        // current "logical" buffer position, in units of time
        StateBuffer<phase_t> phase{0.0};
        // last write index in buffer
        StateBuffer<frame_t> wrIdx{0};
        // current fade position in [0,1]
        StateBuffer<float> fade{0.f};
        // final preserve level, post-fade
        StateBuffer<float> pre{0.f};
        // final record level, post-fade
        StateBuffer<float> rec{0.f};

    protected:
        void setPosition(frame_t i, phase_t position);

        // update phase, opState, and opAction
        OpAction calcFramePosition(frame_t i_1, frame_t i);

        // update frame level data
        void calcFrameLevels(frame_t i);

        // perform single frame write
        void performFrameWrite(frame_t i_1, frame_t i, float input);

        // read a single frame
        float performFrameRead(frame_t i);

        void setBuffer(float *b, frame_t fr) {
            this->buf = b;
            this->bufFrames = fr;
        }

        frame_t wrapBufIndex(frame_t x) {
            assert(bufFrames != 0 && "buffer frame count must not be zero when running");
            frame_t y = x;
            while (y >= bufFrames) { y -= bufFrames; }
            while (y < 0) { y += bufFrames; }
            return y;
        }
        void updateRate(frame_t idx, rate_t rate);

    private:
        Resampler resamp;   // resampler
        float *buf{};         // current audio buffer
        frame_t bufFrames{};   // total buffer size

        void init(ReadWriteHead *rwh);

        void setRwh(ReadWriteHead *rwh) { this->rwh = rwh; }
    };
}

#endif // Softcut_SUBHEAD_H
