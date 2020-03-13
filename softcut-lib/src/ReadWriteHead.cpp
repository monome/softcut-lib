//
// Created by ezra on 12/6/17.
//
#include <cmath>
#include <limits>


#include "softcut/Interpolate.h"
#include "softcut/Resampler.h"

#include "softcut/ReadWriteHead.h"

using namespace softcut;
using namespace std;

int ReadWriteHead::dequeuePositionChange(size_t fr) {
//        SubHead::FramePositionData &a,
//        SubHead::FramePositionData &b) {
    if (enqueuedPosition < 0) {
        return -1;
    }
    for (int headIdx=0; headIdx<2; ++headIdx) {
        if (head[headIdx].opState[fr] == SubHead::Stopped) {
            head[headIdx].opState[fr] = SubHead::FadeIn;
            head[headIdx].phase[fr]  = enqueuedPosition;
            head[headIdx].updateWrIdx(fr, rate[fr], recOffsetSamples);
            enqueuedPosition = -1.0;
            return 0;
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
    /// TODO [optimize]: replace this data structure with `self`
    SubHead::FramePositionParameters params{};
    params.start = start;
    params.end = end;
    params.loop = loopFlag;
    params.fadeInc = fadeInc;

    size_t fr = 0;
    size_t fr_1 = lastNumFrames - 1;
    while (fr < numFrames) {
        params.rate = rate[fr];
        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        handleLoopAction(head[0].calcPositionUpdate(fr_1, fr, params));
        handleLoopAction(head[1].calcPositionUpdate(fr_1, fr, params));
        // change positions if needed
        int headMoved = dequeuePositionChange(fr);
        if (headMoved == 0) { head[0].updateWrIdx(fr, rate[fr], recOffsetSamples); }
        if (headMoved == 1) { head[1].updateWrIdx(fr, rate[fr], recOffsetSamples); }
        fr_1 = fr;
        ++fr;
    }
}

void ReadWriteHead::updateSubheadWriteLevels(size_t numFrames) {
    /// TODO [optimize]: replace this data structure with `self`
    SubHead::FrameLevelParameters params{};
    params.fadeCurves = fadeCurves;

    for (size_t fr=0; fr < numFrames; ++fr) {
        params.rate = this->rate[fr];
        params.pre = this->pre[fr];
        params.rec = this->rec[fr];
        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        head[0].calcLevelUpdate(fr, params);
        head[1].calcLevelUpdate(fr, params);
    }
}


void ReadWriteHead::performSubheadWrites(const float *input, size_t numFrames) {
    size_t fr = 0;
    size_t fr_1 = lastNumFrames - 1;
    while (fr < numFrames) {
        head[0].performFrameWrite(fr_1, fr, input[fr]);
        head[1].performFrameWrite(fr_1, fr, input[fr]);
        fr_1 = fr++;
    }
}

void ReadWriteHead::performSubheadReads(float *output, size_t numFrames) {
    float out0;
    float out1;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        out0 = head[0].performFrameRead(fr);
        out1 = head[1].performFrameRead(fr);
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

void ReadWriteHead::cutToPos(float seconds) {
    enqueuePositionChange(seconds * sr);
}

phase_t ReadWriteHead::getActivePhase() {
    // FIXME
    return 0;
}

