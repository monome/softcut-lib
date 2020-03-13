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
    // FIXME: template would be nicer
    //template <size_t blockSizeExpected>
    class SubHead {

    protected:
        static constexpr size_t maxBlockSize = 1024;
        template <typename T>
        using StateBuffer = std::array<T, maxBlockSize>;

        friend class ReadWriteHead;
        friend class TestBuffers;
    public:
        // operational state and action descriptors
        typedef enum {
            Playing = 0, Stopped = 1, FadeIn = 2, FadeOut = 3
        } OpState;
        typedef enum {
            None, Stop, LoopPositive, LoopNegative, DoneFadeIn, DoneFadeOut,
        } OpAction;


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
        StateBuffer<size_t> wrIdx{0};
        // current read/write increment direction
        StateBuffer<int> dir{1};
        // current fade position in [0,1]
        StateBuffer<float> fade{0.f};
        // final preserve level, post-fade
        StateBuffer<float> pre{0.f};
        // final record level, post-fade
        StateBuffer<float> rec{0.f};

        // structure containing all parameters required for a single-frame position update,
        // (not unique to / stored by the subhead.)
        struct FramePositionParameters {
            float rate;
            float fadeInc;
            phase_t start;
            phase_t end;
            bool loop;
        };

        struct FrameLevelParameters {
            float rate;
            float pre; // base preserve level
            float rec; // base record level
            FadeCurves *fadeCurves;
        };

    protected:

        void updateWrIdx(size_t idx, float rate, int offset) {
            size_t w = static_cast<size_t>(phase[idx]);
            if (rate >= 0) {
                w = static_cast<size_t>(static_cast<long int>(w) + offset);
            } else {
                w = static_cast<size_t>(static_cast<long int>(w) - offset);
            }
            wrIdx[idx] = w;
        }

        // update phase, opState, and opAction
        OpAction calcPositionUpdate(size_t idx_1, size_t idx, const FramePositionParameters &a);
        // update frame level data
        void calcLevelUpdate(size_t idx,  const FrameLevelParameters &a);
        // perform single frame write
        void performFrameWrite(size_t idx_1, size_t idx, float input);
        // read a single frame
        float performFrameRead(size_t idx);

        void setBuffer(float *b, size_t fr) {
            this->buf = b;
            this->bufFrames = fr;
        }

        size_t wrapBufIndex(size_t x) {
            size_t y = x + bufFrames;
            // FIXME: should wrap to loop endpoints, maybe
            while (y > bufFrames) { y -= bufFrames; }
            return y;
        }

    private:
        Resampler resamp;   // resampler
        float *buf{};         // current audio buffer
        size_t bufFrames{};   // total buffer size
    };
}

#endif // Softcut_SUBHEAD_H
