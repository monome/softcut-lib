//
// Created by ezra on 11/16/18.
//

#ifndef Softcut_TESTBUFFERS_H
#define Softcut_TESTBUFFERS_H

#include <iostream>
#include <fstream>
#include <vector>

#include "Softcut.h"
#include "ReadWriteHead.h"
#include "SubHead.h"
#include "Voice.h"

namespace softcut {

    class TestBuffers {
        // stores the per-sample state of a complete ReadWriteHead,
        // for analysis and testing
        //
        // WARNING: TestBuffers keeps its entire output in heap memory,
        // and performs allocations when updating with new values.
        // therefore:
        // (1) using it from realtime audio threads could stall the thread longer than expected,
        // so it is most useful in NRT tests,
        // (2) its memory usage will grow without bounds when appending values,
        // so it should only be activated for brief intervals.

    public:
        typedef enum {
            //-- ReadWriteHead parameters
            Rate,        // movement rate
            Dir,        // movement direction (should just be sign of rate)
            Pre,         // preserve level
            Rec,         // record level
            Active,       // index of active subhead
            FrameInBlock, // current offset into audio block
            //--- each subhead:
            State0, State1,    // operational state for each subhead
            Action0, Action1,  // operation actions taken, if any
            Phase0, Phase1,    // phase in fractional samples
            Fade0, Fade1,      // fade position in [0, 1]
            Rec0, Rec1,        // record level (post-fade)
            Pre0, Pre1,        // preserve level (post-fade)
            WrIdx0, WrIdx1,    // write index in audio buffer
            NumBuffers
        } BufferId;

    private:
        std::array<std::vector<float>, NumBuffers> buffers;

        template<typename T>
        void appendToBuffer(BufferId id, const T *data, size_t numFrames) {
            for (size_t i = 0; i < numFrames; ++i) {
                buffers[id].push_back(static_cast<float>(data[i]));
            }
        }

    public:
        TestBuffers() {

        }

        template<int N>
        void update(Softcut <N> &cut, int voiceId, size_t numFrames) {
            update(cut.scv[voiceId].sch, numFrames);
        }

        void update(const ReadWriteHead &rwh, size_t numFrames) {
            appendToBuffer(Active, rwh.active.data(), numFrames);
            appendToBuffer(Rate, rwh.rate.data(), numFrames);
            appendToBuffer(Dir, rwh.dir.data(), numFrames);
            appendToBuffer(Rec, rwh.rec.data(), numFrames);
            appendToBuffer(Pre, rwh.pre.data(), numFrames);
            appendToBuffer(State0, rwh.head[0].opState.data(), numFrames);
            appendToBuffer(State1, rwh.head[1].opState.data(), numFrames);
            appendToBuffer(Action0, rwh.head[0].opAction.data(), numFrames);
            appendToBuffer(Action1, rwh.head[1].opAction.data(), numFrames);
            appendToBuffer(Phase0, rwh.head[0].phase.data(), numFrames);
            appendToBuffer(Phase1, rwh.head[1].phase.data(), numFrames);
            appendToBuffer(Fade0, rwh.head[0].fade.data(), numFrames);
            appendToBuffer(Fade1, rwh.head[1].fade.data(), numFrames);
            appendToBuffer(Rec0, rwh.head[0].rec.data(), numFrames);
            appendToBuffer(Rec1, rwh.head[1].rec.data(), numFrames);
            appendToBuffer(Pre0, rwh.head[0].pre.data(), numFrames);
            appendToBuffer(Pre1, rwh.head[1].pre.data(), numFrames);
            appendToBuffer(WrIdx0, rwh.head[0].wrIdx.data(), numFrames);
            appendToBuffer(WrIdx1, rwh.head[1].wrIdx.data(), numFrames);
            // frame offset is always just an index counting up from zero
            auto frbuf = new int[numFrames];
            for (unsigned int i = 0; i < numFrames; ++i) { frbuf[i] = i; }
            appendToBuffer(FrameInBlock, frbuf, numFrames);
            delete[]frbuf;
        }

        const float *getBuffer(BufferId id) { return buffers[id].data(); }


    };
}
#endif