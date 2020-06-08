//
// Created by ezra on 12/6/17.
//
#include <algorithm>
#include <dsp-kit/abs.hpp>

#include "dsp-kit/clamp.hpp"

#include "softcut/Fades.h"
#include "softcut/Resampler.h"
#include "softcut/ReadWriteHead.h"
#include "softcut/DebugLog.h"


using namespace softcut;
using namespace std;
using namespace dspkit;

ReadWriteHead::ReadWriteHead() {
    rate.fill(1.f);
    pre.fill(0.f);
    rec.fill(0.f);
    active.fill(-1);
    frameIdx = 0;
    readDuckRamp.setSampleRate(48000.f);
    readDuckRamp.setTime(0.01);
    writeDuckRamp.setSampleRate(48000.f);
    writeDuckRamp.setTime(0.01);
}

void ReadWriteHead::init() {
    start = 0.f;
    end = maxBlockSize * 2;
    head[0].init(this);
    head[1].init(this);
    rate.fill(1.f);
    pre.fill(0.f);
    rec.fill(0.f);
    active.fill(-1);
    frameIdx = 0;
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
    size_t fr_1 = frameIdx;
    size_t fr = 0;
    SubHead::OpAction action;
    checkPositionChange(fr_1, fr);
    while (fr < numFrames) {
        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        for (auto &h  : head) {
            action = h.calcFramePosition(fr_1, fr);
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
    for (size_t fr = 0; fr < numFrames; ++fr) {
        active0 = static_cast<unsigned int>(
                (head[0].opState[fr] != SubHead::Stopped)
                && (head[0].fade[fr] > 0.f));
        active1 = static_cast<unsigned int>(
                (head[1].opState[fr] != SubHead::Stopped)
                && (head[1].fade[fr] > 0.f));
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

phase_t ReadWriteHead::getActivePhase() const {
    // return the last written phase for the last active head
    return head[active[frameIdx]].phase[frameIdx];
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

float ReadWriteHead::getRateBuffer(size_t i) {
    return rate[i];
}

void ReadWriteHead::copySubheadPositions(const ReadWriteHead &src, size_t numFrames) {
    std::copy_n(src.rate.begin(), numFrames, rate.begin());
    std::copy_n(src.dir.begin(), numFrames, dir.begin());
    std::copy_n(src.active.begin(), numFrames, active.begin());
    for (int h = 0; h < 2; ++h) {
        std::copy_n(src.head[h].phase.begin(), numFrames, head[h].phase.begin());
        std::copy_n(src.head[h].wrIdx.begin(), numFrames, head[h].wrIdx.begin());
        std::copy_n(src.head[h].rec.begin(), numFrames, head[h].rec.begin());
        std::copy_n(src.head[h].pre.begin(), numFrames, head[h].pre.begin());
        std::copy_n(src.head[h].fade.begin(), numFrames, head[h].fade.begin());
        std::copy_n(src.head[h].opState.begin(), numFrames, head[h].opState.begin());
        std::copy_n(src.head[h].opAction.begin(), numFrames, head[h].opAction.begin());
    }
}

void ReadWriteHead::computeReadDuckLevels(const ReadWriteHead *other, size_t numFrames) {
    float x;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        x = computeReadDuckLevel(&(head[0]), &(other->head[0]), fr);
        x = clamp_lo<float>(x, computeReadDuckLevel(&(head[0]), &(other->head[1]), fr));
        x = clamp_lo<float>(x, computeReadDuckLevel(&(head[1]), &(other->head[0]), fr));
        x = clamp_lo<float>(x, computeReadDuckLevel(&(head[1]), &(other->head[1]), fr));
        readDuck[fr] = readDuckRamp.getNextValue(1.f- x);;
    }
}

void ReadWriteHead::applyReadDuckLevels(float* output, size_t numFrames) {
#if 1
    for (size_t fr = 0; fr < numFrames; ++fr) {
        output[fr] *= readDuck[fr];
    }
#endif
}

void ReadWriteHead::computeWriteDuckLevels(const ReadWriteHead *other, size_t numFrames) {
    float x;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        x = computeWriteDuckLevel(&(head[0]), &(other->head[0]), fr);
        x = clamp_lo<float>(x, computeWriteDuckLevel(&(head[0]), &(other->head[1]), fr));
        x = clamp_lo<float>(x, computeWriteDuckLevel(&(head[1]), &(other->head[0]), fr));
        x = clamp_lo<float>(x, computeWriteDuckLevel(&(head[1]), &(other->head[1]), fr));
        writeDuck[fr] = x;
    }
}

void ReadWriteHead::applyWriteDuckLevels(size_t numFrames) {
    float x, y;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        x = writeDuck[fr];
        y = writeDuckRamp.getNextValue(x);
        rec[fr] *= (1.f - y);
        pre[fr] += (1.f - pre[fr]) * y;
    }
}


float ReadWriteHead::computeReadDuckLevel(const SubHead* a, const SubHead* b, size_t frame) {
    /// FIXME: for this to really work, position needs to be calculated modulo loop points.
    //// as it is, we get artifacts when one or both voices cross the loop point,
    //// while being near each other on one side of it.
    //// running the duck level through a smoother is an attempt to mitigate these artifacts...
    static constexpr float recMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float fadeMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float preMax = 1.f - (std::numeric_limits<float>::epsilon() * 2.f);
    static constexpr phase_t dmax = 480*2;
    static constexpr phase_t dmin = 480;
    if (a->fade[frame] < fadeMin) { return 0.f; }
    if ((b->rec[frame] < recMin) && (b->pre[frame] > preMax)) { return 0.f; }
    const auto dabs = dspkit::abs<phase_t>(a->phase[frame] - b->phase[frame]);
    if (dabs >= dmax) { return 0.f; }
    if (dabs <= dmin) { return 1.f; }
    return Fades::raisedCosFadeOut(static_cast<float>((dabs-dmin) / (dmax-dmin)));
}


float ReadWriteHead::computeWriteDuckLevel(const SubHead* a, const SubHead* b, size_t frame) {
    static constexpr float recMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float preMax = 1.f - (std::numeric_limits<float>::epsilon() * 2.f);
    static constexpr phase_t dmax = 480*2;
    static constexpr phase_t dmin = 480;
    if (a->rec[frame] < recMin && a->pre[frame] > preMax) { return 0.f; }
    if (b->rec[frame] < recMin && b->pre[frame] > preMax) { return 0.f; }
    const auto dabs = dspkit::abs<phase_t>(a->phase[frame] - b->phase[frame]);
    if (dabs >= dmax) { return 0.f; }
    if (dabs <= dmin) { return 1.f; }
    return Fades::raisedCosFadeOut(static_cast<float>((dabs-dmin) / (dmax-dmin)));
}