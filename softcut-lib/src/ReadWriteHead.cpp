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


void ReadWriteHead::init() {
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

int ReadWriteHead::dequeuePositionChange(size_t fr) {
    if (enqueuedPosition < 0) {
        return -1;
    }
    for (int headIdx = 0; headIdx < 2; ++headIdx) {
        if (head[headIdx].opState[fr] == SubHead::Stopped) {
            head[headIdx].setPosition(fr, enqueuedPosition);
            enqueuedPosition = -1.0;
            return headIdx;
        }
    }
    return -1;
}

void ReadWriteHead::checkPositionChange(frame_t fr_1, frame_t fr) {
    // change positions if needed
    int headMoved = dequeuePositionChange(fr);
    if (headMoved >= 0) {
        active[fr] = headMoved;
    } else {
        active[fr] = active[fr_1];
    }
}

void ReadWriteHead::updateSubheadPositions(size_t numFrames) {
    // TODO: apply `follow` here, using subhead positions from other ReadWriteHead
    size_t fr_1 = frameIdx;
    size_t fr = 0;
    SubHead::OpAction action;
    checkPositionChange(fr_1, fr);
    while (fr < numFrames) {
        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        for (int i = 0; i < 2; ++i) {
            action = head[i].calcFramePosition(fr_1, fr);
            if (action == SubHead::OpAction::LoopPositive) {
                enqueuePositionChange(start);
            } else if (action == SubHead::OpAction::LoopNegative) {
                enqueuePositionChange(end);
            }
        }
        checkPositionChange(fr_1, fr);
        fr_1 = fr;
        ++fr;
    }
    frameIdx = fr_1;
}

void ReadWriteHead::updateSubheadWriteLevels(size_t numFrames) {
    for (size_t fr = 0; fr < numFrames; ++fr) {
        // TODO: apply `duck` here, using subhead positions/levels from other ReadWriteHead
        // update rec/pre level for each subhead
        this->head[0].calcFrameLevels(fr);
        this->head[1].calcFrameLevels(fr);
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
    unsigned int activeHeadBits;
    unsigned int active0;
    unsigned int active1;
    // FIXME: these checks are probably not a net win for performance
    for (size_t fr = 0; fr < numFrames; ++fr) {
        active0 = static_cast<unsigned int>(
                (head[0].opState[fr] != SubHead::Stopped)
                && (head[0].fade[fr] > 0.f) );
        active1 = static_cast<unsigned int>(
                (head[1].opState[fr] != SubHead::Stopped)
                && (head[1].fade[fr] > 0.f) );
        activeHeadBits = active0 | (active1 << 1u);
        switch (activeHeadBits) {
            case 0:
                output[fr] = 0.f;
                break;
            case 1:
                output[fr] = head[0].fade[fr] * head[0].performFrameRead(fr);
                break;
            case 2:
                output[fr] = head[1].fade[fr] * head[1].performFrameRead(fr);
                break;
            case 3:
                out0 = this->head[0].performFrameRead(fr);
                out1 = this->head[1].performFrameRead(fr);
                output[fr] = mixFade(out0, out1, head[0].fade[fr], head[1].fade[fr]);
                break;
            default:
                output[fr] = 0.f;
        }
    }
}

void ReadWriteHead::setSampleRate(float sr) {
    this->sr = sr;
    // TODO: refresh dependent variables..
    /// though in present applications, we are unlikely to change SR at runtime.
}

void ReadWriteHead::setBuffer(sample_t *b, uint32_t sz) {
    this->buf = b;
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
    this->fadeInc = 1.f / (secs * sr);
    // TODO: something like,
//    fadeOutFrameBeforeLoop = start - (fadeTime * r)
}

void ReadWriteHead::setLoopFlag(bool val) {
    this->loopFlag = val;
}

void ReadWriteHead::setRate(size_t i, rate_t x) {
    rate[i] = x;
    dir[i] = x > 0.f ? 1 : -1;
}

void ReadWriteHead::setRec(size_t i, float x) {
    rec[i] = x;
}

void ReadWriteHead::setPre(size_t i, float x) {
    pre[i] = x;
}

void ReadWriteHead::setPosition(float seconds) {
    enqueuePositionChange(seconds * sr);
}

phase_t ReadWriteHead::getActivePhase() {
    // TODO
    return 0;
}


phase_t ReadWriteHead::wrapPhaseToLoop(phase_t p) {
    if (p < start) {
        return p + end - start;
    } else if (p > end) {
        return p - end + start;
    } else {
        return p;
    }
}

ReadWriteHead::frame_t ReadWriteHead::wrapFrameToLoopFade(frame_t w) {
    //frame_t max -
    return 0;
}

