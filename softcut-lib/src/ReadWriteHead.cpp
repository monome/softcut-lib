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
    frameIdx = 0;
    readDuckRamp.setSampleRate(48000.f);
    readDuckRamp.setTime(0.01);
    writeDuckRamp.setSampleRate(48000.f);
    writeDuckRamp.setTime(0.01);
    this->init();
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
    fadeInc = 1.f;
}

int ReadWriteHead::checkPositionRequest(size_t fr_1, size_t fr) {
    // check for pos change request
    phase_t pos = requestedPosition.load();
    if (pos >= 0.0) {
        int newHead = -1;
        for (int headIdx = 0; headIdx < 2; ++headIdx) {
            if (head[headIdx].opState[fr_1] == SubHead::Stopped) {
                newHead = headIdx;
                break;
            }
        }

        if (newHead != -1) {
            requestedPosition.store(-1.0);
            // std::cerr << "performing position request; head=" << newHead << "; fr=" << fr << "; pos=" << pos
                      //<< std::endl;
            jumpToPosition(newHead, fr_1, fr, pos);
            // hack...
            head[newHead].opState[fr_1] = SubHead::OpState::FadeIn;
            auto oldHead = 1 - newHead;
            if (head[oldHead].opState[fr_1] != SubHead::OpState::Stopped) {
                head[oldHead].opState[fr_1] = SubHead::OpState::FadeOut;
                head[oldHead].opAction[fr_1] = SubHead::OpAction::FadeOutAndStop;
            }
            return newHead;
        }
    }
    return -1;
}

void ReadWriteHead::jumpToPosition(int newHead, size_t fr_1, size_t fr, phase_t pos) {
    head[newHead].setPosition(static_cast<long>(fr_1), static_cast<long>(fr), pos);
    active[fr] = newHead;
}

void ReadWriteHead::loopToPosition(int oldHead, size_t fr_1, size_t fr, phase_t pos) {
    int newHead = oldHead > 0 ? 0 : 1;
    active[fr] = newHead;
    // std::cerr << "looping to position: " << pos << "(old: "<<oldHead<<"; new: "<<newHead<<")"<<std::endl;
    jumpToPosition(newHead, fr_1, fr, pos);
}

void ReadWriteHead::updateSubheadPositions(size_t numFrames) {
    size_t fr_1 = frameIdx;
    size_t fr = 0;
    SubHead::OpAction action;

    auto newActive = checkPositionRequest(fr_1, fr);
    if (newActive >= 0) {
        // std::cerr << "new active head, top of block: " << newActive << std::endl;
        active[fr_1] = newActive;
    }

    while (fr < numFrames) {
        int loopHead = -1;
        int loopDir = 0;
        for (int headIdx = 0; headIdx < 2; ++headIdx) {
            action = head[headIdx].calcFramePosition(fr_1, fr);
            if (action == SubHead::OpAction::LoopPositive) {
                loopHead = headIdx;
                loopDir = 1;
            } else if (action == SubHead::OpAction::LoopNegative) {
                loopHead = headIdx;
                loopDir = -1;
            }
        }

        newActive = checkPositionRequest(fr_1, fr);
        if (newActive >= 0) {
            // std::cerr << "new active head (frame " <<fr << " in block): "<< newActive << std::endl;
            active[fr] = active[fr_1] = newActive;
        } else {
            if (loopHead >= 0) {
                loopToPosition(loopHead, fr_1, fr, loopDir > 0 ? start : end);
            } else {
                active[fr] = active[fr_1];
            }
        }
        fr_1 = fr;
        ++fr;
    }
    // never happens:
//    newActive = checkPositionRequest(fr_1, 0);
//    if (newActive >= 0) {
//        std::cerr << "new active head, end of block: " << newActive << std::endl;
//        active[0] = newActive;
//    }
    frameIdx = fr_1;

}

void ReadWriteHead::updateSubheadWriteLevels(size_t numFrames) {
    for (size_t fr = 0; fr < numFrames; ++fr) {
        this->head[0].calcFrameLevels(fr);
        this->head[1].calcFrameLevels(fr);
    }
}

void ReadWriteHead::performSubheadWrites(const float *input, size_t numFrames) {
    auto fr_1 = static_cast<frame_t>(frameIdx);
    frame_t fr = 0;
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
        auto frIdx = static_cast<frame_t>(fr);
        switch (activeHeadBits) {
            case 0:
                output[fr] = 0.f;
                break;
            case 1:
                output[fr] = head[0].fade[fr] * head[0].performFrameRead(frIdx);
                break;
            case 2:
                output[fr] = head[1].fade[fr] * head[1].performFrameRead(frIdx);
                break;
            case 3:
                out0 = this->head[0].performFrameRead(frIdx);
                out1 = this->head[1].performFrameRead(frIdx);
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
    if (secs > 0.f) {
        this->fadeInc = 1.f / (secs * sr);
    } else {
        this->fadeInc = 1.f;
    }
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

void ReadWriteHead::requestPosition(float seconds) {
    requestedPosition.store(seconds * sr, std::memory_order_relaxed);
}

phase_t ReadWriteHead::getActivePhase() const {
    // return the last written phase for the last active head
    return head[active[frameIdx]].phase[frameIdx];
}

float ReadWriteHead::getRateBuffer(size_t i) {
    return rate[i];
}

void ReadWriteHead::copySubheadPosition(const ReadWriteHead &src, size_t numFrames) {
    frameIdx = src.frameIdx;
    fadeInc = src.fadeInc;
    loopFlag = src.loopFlag;
    recOffsetSamples = src.recOffsetSamples;
    start = src.start;
    end = src.end;
    sr  = src.sr;
    std::copy_n(src.rate.begin(), numFrames, rate.begin());
    std::copy_n(src.dir.begin(), numFrames, dir.begin());
    std::copy_n(src.active.begin(), numFrames, active.begin());
    for (int h = 0; h < 2; ++h) {
        std::copy_n(src.head[h].phase.begin(), numFrames, head[h].phase.begin());
        std::copy_n(src.head[h].wrIdx.begin(), numFrames, head[h].wrIdx.begin());
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
        readDuck[fr] = readDuckRamp.getNextValue(1.f - x);
    }
}

void ReadWriteHead::applyReadDuckLevels(float *output, size_t numFrames) {
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


float ReadWriteHead::computeReadDuckLevel(const SubHead *a, const SubHead *b, size_t frame) {
    /// FIXME: for this to really work, position needs to be calculated modulo loop points.
    //// as it is, we get artifacts when one or both voices cross the loop point,
    //// while being near each other on one side of it.
    //// running the duck level through a smoother is an attempt to mitigate these artifacts...
    static constexpr float recMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float fadeMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float preMax = 1.f - (std::numeric_limits<float>::epsilon() * 2.f);
    // FIXME: these magic numbers are unaffected by sample rate :/
    static constexpr phase_t dmax = 480 * 2;
    static constexpr phase_t dmin = 480;
    // if `a` fade level is ~0, no ducking is needed
    if (a->fade[frame] < fadeMin) { return 0.f; }
    // if `b` record level is ~0, and pre level is ~1, no ducking is needed
    if ((b->rec[frame] < recMin) && (b->pre[frame] > preMax)) { return 0.f; }
    // FIXME: this is where we probably need to compute distance modulo loop points...
    const auto dabs = dspkit::abs<phase_t>(a->phase[frame] - b->phase[frame]);
    if (dabs >= dmax) { return 0.f; }
    if (dabs <= dmin) { return 1.f; }
    return Fades::raisedCosFadeOut(static_cast<float>((dabs - dmin) / (dmax - dmin)));
}


float ReadWriteHead::computeWriteDuckLevel(const SubHead *a, const SubHead *b, size_t frame) {
    static constexpr float recMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float preMax = 1.f - (std::numeric_limits<float>::epsilon() * 2.f);
    static constexpr phase_t dmax = 480 * 2;
    static constexpr phase_t dmin = 480;
    if (a->rec[frame] < recMin && a->pre[frame] > preMax) { return 0.f; }
    if (b->rec[frame] < recMin && b->pre[frame] > preMax) { return 0.f; }
    const auto dabs = dspkit::abs<phase_t>(a->phase[frame] - b->phase[frame]);
    if (dabs >= dmax) { return 0.f; }
    if (dabs <= dmin) { return 1.f; }
    return Fades::raisedCosFadeOut(static_cast<float>((dabs - dmin) / (dmax - dmin)));
}