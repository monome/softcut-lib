//
// Created by ezra on 12/6/17.
//
#include <cmath>
#include "softcut/Resampler.h"
#include "softcut/ReadWriteHead.h"

using namespace softcut;
using namespace std;

ReadWriteHead::ReadWriteHead() {
    rate.fill(1.f);
    pre.fill(0.f);
    rec.fill(0.f);
    active.fill(-1);
    lastNumFrames = 0;
}
int ReadWriteHead::dequeuePositionChange(size_t fr) {
    if (enqueuedPosition < 0) {
        return -1;
    }
    for (int headIdx = 0; headIdx < 2; ++headIdx) {
        if (head[headIdx].opState[fr] == SubHead::Stopped) {
            std::cerr << "dequeing position change to head " << headIdx << " : " << enqueuedPosition << std::endl;
            head[headIdx].setPosition(fr, enqueuedPosition, this);
            enqueuedPosition = -1.0;
            return headIdx;
        }
    }
    return -1;
}

void ReadWriteHead::handleLoopAction(SubHead::OpAction action) {
    switch (action) {
        case SubHead::OpAction::LoopPositive:
            enqueuePositionChange(start);
            break;
        case SubHead::OpAction::LoopNegative:
            enqueuePositionChange(end);
            break;
        case SubHead::OpAction::DoneFadeIn:
        case SubHead::OpAction::DoneFadeOut:
        case SubHead::OpAction::None:
        default:;; // nothing to do
    }
}

void ReadWriteHead::updateSubheadPositions(size_t numFrames) {
    // TODO: apply `follow` here, using subhead positions from other ReadWriteHead

    size_t fr = 0;
    size_t fr_1 = lastNumFrames - 1;
    SubHead::OpAction action;
    while (fr < numFrames) {
        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        for (int i=0; i<2; ++i) {
           action =head[i].calcPositionUpdate(fr_1, fr, this);
           handleLoopAction(action);
        }
        // change positions if needed
        int headMoved = dequeuePositionChange(fr);
        if (headMoved >= 0) {
            active[fr] = headMoved;
        } else {
            active[fr] = active[fr_1];
        }
        fr_1 = fr;
        ++fr;
    }
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
    // TODO [efficiency]: move this loop to subhead method
    size_t fr = 0;
    size_t fr_1 = lastNumFrames - 1;
    while (fr < numFrames) {
        head[0].performFrameWrite(fr_1, fr, input[fr]);
        head[1].performFrameWrite(fr_1, fr, input[fr]);
        fr_1 = fr++;
    }
}

void ReadWriteHead::performSubheadReads(float *output, size_t numFrames) {
    // TODO [efficiency]: move this loop to subhead method
    float out0;
    float out1;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        out0 = this->head[0].performFrameRead(fr);
        out1 = this->head[1].performFrameRead(fr);
        output[fr] = mixFade(out0, out1, head[0].fade[fr], head[1].fade[fr]);
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

