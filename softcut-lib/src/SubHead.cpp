//
// Created by ezra on 4/21/18.
//

#include <softcut/DebugLog.h>
#include "softcut/SubHead.h"
#include "softcut/ReadWriteHead.h"

using namespace softcut;

SubHead::SubHead() {
    phase.fill(0.0);
    wrIdx.fill(0);
    opState.fill(Stopped);
    opAction.fill(None);
    fade.fill(0.f);
    pre.fill(0.f);
    rec.fill(0.f);
}

void SubHead::updateRate(size_t idx, rate_t rate) {
    dir[idx] = rate > 0.f ? 1 : -1;
    resamp.setRate(std::fabs(rate));
}

void SubHead::setPosition(size_t idx_1, size_t idx, phase_t position, const softcut::ReadWriteHead *rwh) {
    if (opState[idx] != Stopped) {
        std::cerr << "error: setting position of moving subhead" << std::endl;
    }
    phase[idx] = position;
    phase[idx_1] = position;
    opState[idx] = SubHead::FadeIn;
    //opState[idx_1] = SubHead::Stopped;
    opAction[idx] = SubHead::OpAction::StartFadeIn;

    frame_t w = wrapBufIndex(static_cast<frame_t>(phase[idx]) + dir[idx] * rwh->recOffsetSamples);
    wrIdx[idx] = w;
    wrIdx[idx_1] = w;
//    DebugLog::newLine(idx);
//    std::cout << "updated write index buffer; last block idx = " << idx_1 << "; new block idx = " << idx
//    << "; new buf idx = " << w << std::endl;
//    didSetPositionThisFrame = true;
}

SubHead::OpAction SubHead::calcPositionUpdate(size_t i_1, size_t i,
                                              const softcut::ReadWriteHead *rwh) {
    updateRate(i, rwh->rate[i]);

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
                        opAction[i] = Stop;
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
                        opAction[i] = Stop;
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

void SubHead::calcLevelUpdate(size_t i, const softcut::ReadWriteHead *rwh) {
    switch (opState[i]) {
        case Stopped:
            return;
        case Playing:
            pre[i] = rwh->pre[i];
            rec[i] = rwh->rec[i];
        case FadeIn:
        case FadeOut:
#if 0 // use FadeCurves
            pre[i] = rwh->pre[i] + (1.f - pre[i]) * rwh->fadeCurves->getPreFadeValue(fade[i]);
    rec[i] = rwh->rec[i] * rwh->fadeCurves->getRecFadeValue(fade[i]);
#else // linear
            // TODO: apply rate==0 deadzone
            pre[i] = rwh->pre[i] + ((1.f - rwh->pre[i]) * (1.f - fade[i]));
            rec[i] = rwh->rec[i] * fade[i];
#endif
    }
}

void SubHead::performFrameWrite(size_t i_1, size_t i, const float input) {

    // push to resampler, even if stopped
    // this should avoid a glitch when restarting
    int nframes = resamp.processFrame(input);
    // std::cerr << nframes << std::endl;

    if (opState[i] == Stopped) {
        return;
    }

    sample_t y;
    frame_t w;
    wrIdx[i] = wrIdx[i_1]; // by default, propagate last write position

    const sample_t *src = resamp.output();

    for (int fr = 0; fr < nframes; ++fr) {
        w = wrapBufIndex(wrIdx[i] + dir[i]);
        wrIdx[i] = w;
        y = (buf[w] * pre[i]) + (src[fr] * rec[i]);
        // TODO: further processing (lowpass, clip)
        buf[w] = y;
    }
}

float SubHead::performFrameRead(size_t i) {
    int phase1 = static_cast<int>(phase[i]);
    int phase0 = phase1 - 1;
    int phase2 = phase1 + 1;
    int phase3 = phase1 + 2;

    float y0 = buf[wrapBufIndex(phase0)];
    float y1 = buf[wrapBufIndex(phase1)];
    float y3 = buf[wrapBufIndex(phase3)];
    float y2 = buf[wrapBufIndex(phase2)];

    auto x = static_cast<float>(phase[i] - (float) phase1);
    return Interpolate::hermite<float>(x, y0, y1, y2, y3);
}

void SubHead::init(FadeCurves *pCurves) {
    resamp.setPhase(0);
}
