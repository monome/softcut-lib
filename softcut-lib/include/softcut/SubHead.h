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
#include <iostream>
#include "Resampler.h"
#include "Types.h"

namespace softcut {
    class ReadWriteHead;

    class SubHead {
        friend class ReadWriteHead;
        friend class Voice;
        friend class TestBuffers;
    public:
        SubHead();

    public:
         enum class PlayState {
            Stopped, // stopped, fade down
            FadeIn,  // moving, fading in
            Playing, // moving, fade up
            FadeOut  // moving, fading out
        };

        enum class PhaseResult {
            WasStopped,
            WasPlaying,
            DoneFadeIn, // finished fading in
            DoneFadeOut, // finished fading out
            CrossLoopEnd, // crossed loop end, moving forward
            CrossLoopStart // crossed loop start, moving backward
        };

        using frame_t = long int;
        static constexpr frame_t maxBlockSize = 1024;
        template<typename T>
        using StateBuffer = std::array<T, maxBlockSize>;

    protected:
        ReadWriteHead *rwh{};
        //--- buffered state variables, owned:
        // rate direction multiplier
        SubHead::StateBuffer<int> rateDirMul{1};
        // final rate sign
        SubHead::StateBuffer<int> rateSign{1};
        // current state of operation
        StateBuffer<PlayState> playState{PlayState::Stopped};
//        // last action performed
//        StateBuffer<OpAction> opAction{None};
        // current "logical" buffer position, in fractional samples
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

        PhaseResult updatePhase(frame_t fr_1, frame_t i);

        // update frame level data
        void calcFrameLevels(frame_t i);

        // perform single frame write
        void performFrameWrite(frame_t i_1, frame_t i, float input);

        // read a single frame
        float performFrameRead(frame_t i);

        void setBuffer(float *b, frame_t fr);

        [[nodiscard]] frame_t wrapBufIndex(frame_t x) const;

        [[nodiscard]] phase_t wrapPhaseToBuffer(phase_t p) const;

    private:
        Resampler resamp;   // resampler
        float *buf{};         // current audio buffer
        frame_t bufFrames{};   // total buffer size

        void init(ReadWriteHead *rwh);

        void applyRateDeadzone(frame_t i);

        void syncWrIdx(frame_t i);

        void incrementPhase(frame_t fr);
    };
}

#endif // Softcut_SUBHEAD_H
