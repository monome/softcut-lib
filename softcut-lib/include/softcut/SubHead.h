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

        // updating position and state data is a per-frame operation.
        // structure contains all state variables required for each frame update.
        // (targets and sources)
        struct FramePositionData {
            phase_t phase;
            OpState opState;
            OpAction opAction;
            float fade;
        };

        // structure containing all parameters required for a single-frame position update,
        // (not unique to / stored by the subhead.)
        struct FramePositionParameters {
            float rate;
            float fadeInc;
            phase_t start;
            phase_t end;
            bool loop;
            size_t wrIdx;
        };
        // structure containing all data required for a single-frame level update,
        // (targets and sources)
        struct FrameLevelData {
            phase_t phase;
            OpState opState;
            float fade;
            float rec;
            float pre;
        };
        struct FrameLevelParameters {
            float rate;
            float pre; // base preserve level
            float rec; // base record level
            FadeCurves *fadeCurves;
        };
        // structure containing all source and target data involved in single-frame write

        struct FrameWriteData {
            size_t wrIdx;
        };

    protected:
        //-------------
        // --- helpers
        // update relevant buffers with single frame of position data.
        void storeFramePositionData(size_t idx, const FramePositionData &data) {
            phase[idx] = data.phase;
            opState[idx] = data.opState;
            opAction[idx] = data.opAction;
            fade[idx] = data.fade;
        }

        // initialize a frame position data structure from buffered values
        void loadFramePositionData(size_t idx, FramePositionData &data) {
            data.phase = phase[idx];
            data.opState = opState[idx];
            data.opAction = opAction[idx];
            data.fade = fade[idx];
        }

        // update relevant buffers with single frame of level data.
        void storeFrameLevelData(size_t idx, FrameLevelData &data) {
            phase[idx] = data.phase;
            opState[idx] = data.opState;
            fade[idx] = data.fade;
            rec[idx] = data.rec;
            pre[idx] = data.pre;
        }

        // initialize a frame level data structure from buffered values
        void loadFrameLevelData(size_t idx, FrameLevelData &data) {
            data.phase = phase[idx];
            data.opState = opState[idx];
            data.fade = fade[idx];
            data.rec = rec[idx];
            data.pre = pre[idx];
        }

        void loadFrameWriteData(size_t idx, FrameWriteData &data) {
            data.wrIdx = wrIdx[idx];
        }
        void storeFrameWriteData(size_t idx, const FrameWriteData &data) {
            wrIdx[idx] = data.wrIdx;
        }
        void updateWrIdx(size_t idx, float rate, int offset) {
            size_t w = static_cast<size_t>(phase[idx]);
            if (rate >= 0) {
                w = static_cast<size_t>(static_cast<long int>(w) + offset);
            } else {
                w = static_cast<size_t>(static_cast<long int>(w) - offset);
            }
            wrIdx[idx] = w;
        }

        // update frame position data from parameters
        static OpAction calcPositionUpdate(FramePositionData &x, const FramePositionParameters &a);
        // update frame level data from parameters
        static void calcLevelUpdate(FrameLevelData &x,  const FrameLevelParameters &a);

        // perform single frame write
        void performFrameWrite(FrameWriteData &x, size_t idx, float input);

        // read a single frame
        // this requires no state update, so no de-interleaved data structure
        float performFrameRead(size_t idx);

        void setBuffer(float *b, size_t fr) {
            this->buf = b;
            this->bufFrames = fr;
        }

        size_t wrapBufIndex(size_t x) {
            size_t y = x + bufFrames;
            while (y > bufFrames) { y -= bufFrames; }
            return y;
        }

    private:
        Resampler resamp;   // resampler
        float *buf{};         // current audio buffer
        size_t bufFrames{};   // total buffer size

        void updateWrIdx();
    };
}

#endif // Softcut_SUBHEAD_H

/* old (0d621dfa0c3835b5cb96336c9ad67ca2f21c254d)
namespace softcut {

    // state and action are tracked/taken each sample
    typedef enum {
        Playing = 0, Stopped = 1, FadeIn = 2, FadeOut = 3
    } State;
    typedef enum {
        None, Stop, LoopPos, LoopNeg
    } Action;


    class SubHead {
        friend class ReadWriteHead;

    protected:
        // internal block size for buffered state variables.
        // must be >= actual audio block size for any given callback.
        static constexpr int blockSize = 2048;
    public:
        void init(FadeCurves *fc);

        void setSampleRate(float sr);

    private:
        sample_t peek4();

        unsigned int wrapBufIndex(int x);


    protected:
        sample_t peek();

        //! poke
        //! @param in: input value
        //! @param pre: scaling level for previous buffer content
        //! @param rec: scaling level for new content
        void poke(sample_t in, float pre, float rec);

        Action updatePhase(phase_t start, phase_t end, bool loop);

        void updateFade(float inc);

        // getters
        phase_t phase() { return phase_; }

        float fade() { return fade_; }

        float trig() { return trig_; }

        State state() { return state_; }

        // setters
        void setState(State state);

        void setPhase(phase_t phase);

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // **NB** buffer size must be a power of two!!!!
        void setBuffer(sample_t *buf, unsigned int frames);

        void setRate(rate_t rate);

        FadeCurves *fadeCurves;

    private:
        Resampler resamp_;
        SoftClip clip_;

        sample_t *buf_; // output buffer

        unsigned int wrIdx_; // write index
        unsigned int bufFrames_;
        unsigned int bufMask_;

        State state_;
        rate_t rate_;
        int inc_dir_;
        phase_t phase_;
        float fade_;
        float trig_; // output trigger value
        bool active_;
        int recOffset_;buf

        float preFade_;
        float recFade_;

        void setRecOffsetSamples(int d);
    };

}


#endif //Softcut_SUBHEAD_H
*/