//
// Created by ezra on 4/21/18.
//

#include "softcut/DebugLog.h"

#include "softcut/Fades.h"
#include "softcut/SubHead.h"
#include "softcut/ReadWriteHead.h"

using namespace softcut;

SubHead::SubHead() {}

void SubHead::init(ReadWriteHead *parent) {
    rwh = parent;
    resamp.setPhase(0);
    resamp.clearBuffers();
    frame_t w = rwh->recOffsetSamples;
    while (w < 0) { w += maxBlockSize; }
    while (w > maxBlockSize) { w -= maxBlockSize; }
    for (int i = 0; i < maxBlockSize; ++i) {
        phase[i] = 0.0;
        wrIdx[i] = w;
        opState[i] = Stopped;
        opAction[i] = None;
        fade[i] = 0.f;
        pre[i] = 0.f;
        rec[i] = 0.f;
    }
}

void SubHead::setPosition(frame_t i, phase_t position) {
    if (opState[i] != Stopped) {
        std::cerr << "error: setting position of moving subhead" << std::endl;
        assert(false);
    }
    phase_t p = rwh->wrapPhaseToLoop(position);
    phase[i] = p;
    syncWrIdx(i);
    opState[i] = SubHead::FadeIn;
    opAction[i] = SubHead::OpAction::StartFadeIn;
    // resetting the resampler here seems correct:
    // if rate !=1, then each process frame can produce a different number of write-frame advances.
    // so to ensure each pass over a loop has identical write-frame history,
    // resampler should always start each loop with internal phase=0
    resamp.setPhase(0);
    resamp.clearBuffers();
}

void SubHead::syncWrIdx(frame_t i){
    wrIdx[i] = wrapBufIndex(static_cast<frame_t>(phase[i])  + rwh->dir[i] * rwh->recOffsetSamples);
}

SubHead::OpAction SubHead::calcFramePosition(frame_t i_1, frame_t i) {
    switch (opState[i_1]) {
        case FadeIn:
            phase[i] = phase[i_1] + rwh->rate[i];
            fade[i] = fade[i_1] + rwh->fadeInc;
            if (fade[i] >= 1.f) {
                fade[i] = 1.f;
                opState[i] = Playing;
                opAction[i] = DoneFadeIn;
            } else {
                opState[i] = FadeIn;
                opAction[i] = None;
            }
            break;
        case FadeOut:
            phase[i] = phase[i_1] + rwh->rate[i];
            fade[i] = fade[i_1] - rwh->fadeInc;
            if (fade[i] <= 0.f) {
                fade[i] = 0.f;
                opState[i] = Stopped;
                opAction[i] = DoneFadeOut;
            } else { // still fading
                opState[i] = FadeOut;
                opAction[i] = None;
            }
            break;
        case Playing:
            phase[i] = phase[i_1] + rwh->rate[i];
            fade[i] = 1.f;
            if (rwh->rate[i] > 0.f) { // positive rate
                // if we're playing forwards, only loop at endpoint
                if (phase[i] > rwh->end) { // out of loop bounds
                    opState[i] = FadeOut;
                    if (rwh->loopFlag) {
                        opAction[i] = LoopPositive;
                    } else {
                        opAction[i] = FadeOutAndStop;
                    }
                } else { // in loop bounds
                    opAction[i] = None;
                    opState[i] = Playing;
                }
            } else { // negative rate
                if (phase[i] < rwh->start) {
                    opState[i] = FadeOut;
                    if (rwh->loopFlag) {
                        opAction[i] = LoopNegative;
                    } else {
                        opAction[i] = FadeOutAndStop;
                    }
                } else { // in loop bounds
                    opAction[i] = None;
                    opState[i] = Playing;
                }
            }
            break;
        case Stopped:
            phase[i] = phase[i_1];
            fade[i] = 0.f;
            opAction[i] = None;
            opState[i] = Stopped;
    }

    return opAction[i];
}


static float calcPreFadeCurve(float fade) {
    // time parameter is when to finish closing, when fading in
    // FIXME: make this dynamic
    // static constexpr float t = 0;
    static constexpr float t = 1.f/32;
   // static constexpr float t = 1.f/64;
    //static constexpr float t = 1.f/8;
    //static constexpr float t = 1.f;
    if (fade > t) {
        return 0.f;
    }
    else {

        return Fades::fastCosFadeOut(fade/t);
    }
}

static float calcRecFadeCurve(float fade) {
    // time parameter is delay before opening, when fading in
    // FIXME: make this dynamic
    //static constexpr float t = 0.f;
    //static constexpr float t = 0.0625f;
    //static constexpr float t = 0.125f;
    static constexpr float t = 0.25f;
    //static constexpr float t = 0.5f;
    static constexpr float nt = 1.f - t;
    if (fade <= t) {  return 0.f; }
    else { return Fades::raisedCosFadeIn((fade-t)/nt); }
}

void SubHead::calcFrameLevels(frame_t i) {
    switch (opState[i]) {
        case Stopped:
            pre[i] = 1.f;
            rec[i] = 0.f;
            return;
        case Playing:
            pre[i] = rwh->pre[i];
            rec[i] = rwh->rec[i];
            applyRateDeadzone(i);
            break;
        case FadeIn:
        case FadeOut:
            pre[i] = rwh->pre[i] + ((1.f - rwh->pre[i]) * calcPreFadeCurve(fade[i]));
            rec[i] = rwh->rec[i] * calcRecFadeCurve(fade[i]);
            applyRateDeadzone(i);
    }
}

void SubHead::performFrameWrite(frame_t i_1, frame_t i, const float input) {
    resamp.setRate(std::fabs(rwh->rate[i]));
    // push to resampler, even if stopped; avoids possible glitch when restarting
    int nsubframes = resamp.processFrame(input);
    if (opState[i] == Stopped) {
        // if we're stopped, don't touch the buffer at all.
        return;
    }
    if (opAction[i] == OpAction::StartFadeIn) {
        // if we start a fadein on this frame, previous write index is not meaningful;
        // assume current write index has already been updated (in setPosition())
        ;;
    } else {
        // otherwise, propagate last write position
        wrIdx[i] = wrIdx[i_1];
    }
    if (rwh->dir[i] != rwh->dir[i_1]) {
        // if rate sign was just flipped, we need to re-apply the record offset!
        syncWrIdx(i);
    }
    const sample_t *src = resamp.output();
    frame_t w = wrIdx[i];
    sample_t y; // output value
    for (int sfr = 0; sfr < nsubframes; ++sfr) {
        // for each frame produced by the resampler for this input frame,
        // mix, store, and advance the write index
        w = wrapBufIndex(w + rwh->dir[i]);
        y = (buf[w] * pre[i]) + (src[sfr] * rec[i]);
        buf[w] = y;
    }
    wrIdx[i] = w;
}

float SubHead::performFrameRead(frame_t i) {
    auto phase1 = static_cast<frame_t>(phase[i]);
    frame_t phase0 = phase1 - 1;
    frame_t phase2 = phase1 + 1;
    frame_t phase3 = phase1 + 2;

    float y0 = buf[wrapBufIndex(phase0)];
    float y1 = buf[wrapBufIndex(phase1)];
    float y3 = buf[wrapBufIndex(phase3)];
    float y2 = buf[wrapBufIndex(phase2)];

    auto x = static_cast<float>(phase[i] - (float) phase1);
    return Interpolate::hermite<float>(x, y0, y1, y2, y3);
}

SubHead::frame_t SubHead::wrapBufIndex(frame_t x) {
    assert(bufFrames != 0 && "buffer frame count must not be zero when running");
    frame_t y = x;
    while (y >= bufFrames) {
        y -= bufFrames;
    }
    while (y < 0) {
        y += bufFrames;
    }
    return y;
}


void SubHead::setBuffer(float *b, frame_t fr) {
    this->buf = b;
    this->bufFrames = fr;
}

void SubHead::applyRateDeadzone(SubHead::frame_t i) {
    static constexpr float deadzoneBound = 1.f/32.f;
    static constexpr float deadzoneBound_2 = 1.f/64.f;
    const float r = fabs(static_cast<float>(rwh->rate[i]));
    if (r > deadzoneBound) { return; }
    if (r < deadzoneBound_2) { rec[i] = 0.f; return; }
    const float f = (r - deadzoneBound_2) / deadzoneBound_2;
    const float a = Fades::raisedCosFadeIn(f);
    rec[i] *= a;
}
