//
// Created by ezra on 12/6/17.
//
#include <cmath>
#include "softcut/Resampler.h"
#include "softcut/ReadWriteHead.h"
#include "softcut/DebugLog.h"

using namespace softcut;
using namespace std;

ReadWriteHead::ReadWriteHead() {
    rate.fill(1.f);
    pre.fill(0.f);
    rec.fill(0.f);
    active.fill(-1);
    frameIdx = 0;
}


void ReadWriteHead::init(FadeCurves *fc) {
    fadeCurves = fc;
    start = 0.f;
    end = maxBlockSize * 2;
    head[0].init(this);
    head[1].init(this);
    // FIXME: for now, we have to goose the phase update before we start running with write enabled.
    /// could probably deal with this directly in subhead init...
    // this->updateSubheadPositions(maxBlockSize);
}


void ReadWriteHead::enqueuePositionChange(phase_t pos) {
    enqueuedPosition = pos;
}

int ReadWriteHead::dequeuePositionChange(size_t fr_1, size_t fr) {
    if (enqueuedPosition < 0) {
        return -1;
    }
    for (int headIdx = 0; headIdx < 2; ++headIdx) {
        if (head[headIdx].opState[fr] == SubHead::Stopped) {
            std::cout << "dequeing position on head " << headIdx << std::endl;
            head[headIdx].setPosition(fr_1, fr, enqueuedPosition, this);
            enqueuedPosition = -1.0;
            return headIdx;
        }
    }
    return -1;
}

void ReadWriteHead::updateSubheadPositions(size_t numFrames) {
    // TODO: apply `follow` here, using subhead positions from other ReadWriteHead

    size_t fr_1 = frameIdx;
    size_t fr = 0;
    SubHead::OpAction action;
    while (fr < numFrames) {
//        bool didloop = false; // testing...
        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        for (int i=0; i<2; ++i) {
           action =head[i].calcPositionUpdate(fr_1, fr, this);
            if (action == SubHead::OpAction::LoopPositive) {
//                assert(!didloop);
//                didloop = true;
                enqueuePositionChange(start);
            } else if (action == SubHead::OpAction::LoopNegative) {
//                assert(!didloop);
//                didloop = true;
                enqueuePositionChange(end);
            }
        }
        // change positions if needed
        int headMoved = dequeuePositionChange(fr_1, fr);
        if (headMoved >= 0) {
            active[fr] = headMoved;
        } else {
            active[fr] = active[fr_1];
        }
        fr_1 = fr;
        ++fr;
    }
    frameIdx = fr_1;
}

void ReadWriteHead::updateSubheadWriteLevels(size_t numFrames) {
    for (size_t fr = 0; fr < numFrames; ++fr) {
        // TODO: apply `duck` here, using subhead positions/levels from other ReadWriteHead
        // update rec/pre level for each subhead
        this->head[0].calcLevelUpdate(fr, this);
        this->head[1].calcLevelUpdate(fr, this);
    }
}

void ReadWriteHead::performSubheadWrites(const float *input, size_t numFrames) {
    size_t fr_1 = frameIdx;
    size_t fr = 0;
    while (fr < numFrames) {
        head[0].performFrameWrite(fr_1, fr, input[fr]);
        head[1].performFrameWrite(fr_1, fr, input[fr]);
        fr_1 = fr;
        ++fr;
    }
    frameIdx = fr_1;
}

void ReadWriteHead::performSubheadReads(float *output, size_t numFrames) {
    float out0;
    float out1;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        out0 = this->head[0].performFrameRead(fr);
        out1 = this->head[1].performFrameRead(fr);
        output[fr] = mixFade(out0, out1, head[0].fade[fr], head[1].fade[fr]);
        /// testing....
        //output[fr] = mixFade(out0, 0, head[0].fade[fr], 0);
    }
}

void ReadWriteHead::setSampleRate(float sr) {
    this->sr = sr;
    // TODO: refresh dependent variables..
    // though in present applications, we are unlikely to change SR at runtime.
}

void ReadWriteHead::setBuffer(sample_t *b, uint32_t sz) {
    this->buf = b;
    this->bufFrames = sz;
    head[0].setBuffer(b, sz);
    head[1].setBuffer(b, sz);
}

void ReadWriteHead::setLoopStartSeconds(float x) {
    this->start = x * sr;
}

void ReadWriteHead::setRecOffsetSamples(int d) {
    this->recOffsetSamples = d;
}

void ReadWriteHead::setLoopEndSeconds(float x) {
    this->end = x * sr;
}

void ReadWriteHead::setFadeTime(float secs) {
    this->fadeTime = secs;
    this->fadeInc = 1.f / (secs * sr);
}

void ReadWriteHead::setLoopFlag(bool val) {
    this->loopFlag = val;
}

// TODO:
//phase_t ReadWriteHead::getActivePhase() {
//    return 0;
//}
//
//rate_t ReadWriteHead::getRate() {
//    return 0;
//}

void ReadWriteHead::setRate(size_t idx, rate_t x) {
    rate[idx] = x;
}

void ReadWriteHead::setRec(size_t idx, float x) {
    rec[idx] = x;
}

void ReadWriteHead::setPre(size_t idx, float x) {
    pre[idx] = x;
}

void ReadWriteHead::setPosition(float seconds) {
    enqueuePositionChange(seconds * sr);
}

phase_t ReadWriteHead::getActivePhase() {
    // FIXME
    return 0;
}

