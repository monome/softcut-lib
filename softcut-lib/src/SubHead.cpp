//
// Created by ezra on 4/21/18.
//

#include "softcut/SubHead.h"

using namespace softcut;

SubHead::OpAction SubHead::calcPositionUpdate(size_t idx_1, size_t idx,
                                     const SubHead::FramePositionParameters &a) {
    switch (opState[idx_1]) {
        case FadeIn:
            phase[idx] = phase[idx_1] + a.rate;
            fade[idx] = fade[idx_1] + a.fadeInc;
            if (fade[idx] > 1.f) {
                fade[idx] = 1.f;
                std::cerr << "done fadein" << std::endl;
                opState[idx] = Playing;
                opAction[idx] = DoneFadeIn;
            } else {
                opState[idx] = FadeIn;
                opAction[idx] = None;
            }
            break;
        case FadeOut:
            phase[idx] = phase[idx_1] + a.rate;
            fade[idx] = fade[idx] - a.fadeInc;
            if (fade[idx] < 0.f) {
                std::cerr << "done fadeout" << std::endl;
                fade[idx] = 0.f;
                opState[idx] = Stopped;
                opAction[idx] = DoneFadeOut;
            } else { // still fading
                opState[idx] = FadeOut;
                opAction[idx] = None;
            }
            break;
        case Playing:
            phase[idx] = phase[idx_1] + a.rate;
            if (a.rate > 0.f) { // positive rage
                if (phase[idx] > a.end) { // out of loop bounds
                    opState[idx] = FadeOut;
                    if (a.loop) {
                        std::cerr << "looping positive" << std::endl;
                        opAction[idx] = LoopPositive;
                    } else {
                        std::cerr << "stopping after loop end" << std::endl;
                        opAction[idx] = Stop;
                    }
                } else { // in loop bounds
                    opAction[idx] = None;
                    opState[idx] = Playing;
                }
            } else { // negative rate
                if (phase[idx] < a.start) { // out of loop bounds
                    opState[idx] = FadeOut;
                    if (a.loop) {
                        std::cerr << "looping negative" << std::endl;
                        opAction[idx] = LoopNegative;
                    } else {
                        std::cerr << "stopping after loop end" << std::endl;
                        opAction[idx] = Stop;
                    }
                } else { // in loop bounds
                    opAction[idx] = None;
                    opState[idx] = Playing;
                }
            }
            break;
        case Stopped:
            opAction[idx] = None;
            opState[idx] = Stopped;
    }

    return opAction[idx];
}
//SubHead::OpAction SubHead::calcPositionUpdate(
//        FramePositionData &x,
//        const FramePositionParameters &a) {
//    switch (x.opState) {
//        case FadeIn:
//            x.phase = x.phase + a.rate;
//            x.fade = x.fade + a.fadeInc;
//            if (x.fade > 1.f) {
//                x.fade = 1.f;
//                x.opState = Playing;
//                x.opAction = DoneFadeIn;
//            } else {
//                x.opState = FadeIn;
//                x.opAction = None;
//            }
//            break;
//        case FadeOut:
//            x.phase = x.phase + a.rate;
//            x.fade = x.fade - a.fadeInc;
//            if (x.fade < 0.f) {
//                x.fade = 0.f;
//                x.opState = Stopped;
//                x.opAction = DoneFadeOut;
//            } else { // still fading
//                x.opState = FadeOut;
//                x.opAction = None;
//            }
//            break;
//        case Playing:
//            x.phase = x.phase + a.rate;
//            if (a.rate > 0.f) { // positive rage
//                if (x.phase > a.end) { // out of loop bounds
//                    x.opState = FadeOut;
//                    if (a.loop) {
//                        x.opAction = LoopPositive;
//                    } else {
//                        x.opAction = Stop;
//                    }
//                } else { // in loop bounds
//                    x.opAction = None;
//                    x.opState = Playing;
//                }
//            } else { // negative rate
//                if (x.phase < a.start) { // out of loop bounds
//                    x.opState = FadeOut;
//                    if (a.loop) {
//                        x.opAction = LoopNegative;
//                    } else {
//                        x.opAction = Stop;
//                    }
//                } else { // in loop bounds
//                    x.opAction = None;
//                    x.opState = Playing;
//                }
//            }
//            break;
//        case Stopped:
//            x.opAction = None;
//            x.opState = Stopped;
//    }
//
//    return x.opAction;
//}

void SubHead::calcLevelUpdate(size_t idx, const FrameLevelParameters &a) {
    pre[idx] = a.pre + (1.f - a.pre) * a.fadeCurves->getPreFadeValue(fade[idx]);
    rec[idx] = a.rec * a.fadeCurves->getRecFadeValue(fade[idx]);
    // TODO: apply rate==0 deadzone
}


void SubHead::performFrameWrite(size_t idx_1, size_t idx, const float input) {
    // push to resampler, even if stopped
    // this should avoid a glitch when restarting
    int nframes = resamp.processFrame(input);

    // if we're stopped, don't touch our buffer or state vars
    if (opState[idx] == Stopped) {
        return;
    }

    BOOST_ASSERT_MSG(fade[idx] >= 0.f && fade[idx] <= 1.f, "bad fade coefficient in write");

    sample_t y; // write value
    const sample_t *src = resamp.output();

    size_t w = wrIdx[idx_1];
    for (int fr = 0; fr < nframes; ++fr) {
        y = src[fr];
        // TODO: possible further processing (e.g. softclip, filtering)
        buf[w] *= pre[idx];
        buf[w] += y * rec[idx];
        w = wrapBufIndex(w + 1);
    }
    wrIdx[idx] = w;
}

float SubHead::performFrameRead(size_t idx) {
    int phase1 = static_cast<int>(phase[idx]);
    int phase0 = phase1 - 1;
    int phase2 = phase1 + 1;
    int phase3 = phase1 + 2;

    float y0 = buf[wrapBufIndex(phase0)];
    float y1 = buf[wrapBufIndex(phase1)];
    float y3 = buf[wrapBufIndex(phase3)];
    float y2 = buf[wrapBufIndex(phase2)];

    auto x = static_cast<float>(phase[idx] - (float) phase1);
    return Interpolate::hermite<float>(x, y0, y1, y2, y3);
}
//
//void SubHead::setPhase(size_t idx, phase_t phase) {
//    phase_ = phase;
//    wrIdx_ = wrapBufIndex(static_cast<int>(phase_) + (inc_dir_ * recOffset_));
//
//    // NB: not resetting the resampler here:
//    // - it's ok to keep history of input when changing positions.
//    // - resamp output doesn't need clearing b/c we write/read from beginning on each sample anyway
//}

/*
#include <string.h>
#include <limits>

#include "softcut/Interpolate.h"
#include "softcut/FadeCurves.h"
#include "softcut/SubHead.h"

using namespace softcut;

void SubHead::init(FadeCurves *fc) {
    fadeCurves = fc;
    phase_ = 0;
    fade_ = 0;
    trig_ = 0;
    state_ = Stopped;
    resamp_.setPhase(0);
    inc_dir_ = 1;
    recOffset_ = -8;
}

Action SubHead::updatePhase(phase_t start, phase_t end, bool loop) {
    Action res = None;
    trig_ = 0.f;
    phase_t p;
    switch(state_) {
        case FadeIn:
        case FadeOut:
        case Playing:
            p = phase_ + rate_;
            if(active_) {
                if (rate_ > 0.f) {
                    if (p > end || p < start) {
                        if (loop) {
                            trig_ = 1.f;
                            res = LoopPos;
                        } else {
                            state_ = FadeOut;
                            res = Stop;
                        }
                    }
                } else { // negative rate
                    if (p > end || p < start) {
                        if(loop) {
                            trig_ = 1.f;
                            res = LoopNeg;
                        } else {
                            state_ = FadeOut;
                            res = Stop;
                        }
                    }
                } // rate sign check
            } // /active check
            phase_ = p;
            break;
        case Stopped:
        default:
            ;; // nothing to do
    }
    return res;
}

void SubHead::updateFade(float inc) {
    switch(state_) {
        case FadeIn:
            fade_ += inc;
            if (fade_ > 1.f) {
                fade_ = 1.f;
                state_ = Playing;
            }
            break;
        case FadeOut:
            fade_ -= inc;
            if (fade_ < 0.f) {
                fade_ = 0.f;
                state_ = Stopped;
            }
            break;
        case Playing:
        case Stopped:
        default:;; // nothing to do
    }
}

#if 0
/// test: no resampling
void Subhead::poke(float in, float pre, float rec, int numFades) {
    sample_t* p = &buf_[static_cast<unsigned int>(phase_)&bufMask_];
    *p *= pre;
    *p += (in * rec);
}
#else
void SubHead::poke(float in, float pre, float rec) {
    int nframes = resamp_.processFrame(in);

    if(state_ == Stopped) {
        return;
    }

    BOOST_ASSERT_MSG(fade_ >= 0.f && fade_ <= 1.f, "bad fade coefficient in poke()");

    preFade_ = pre + (1.f-pre) * fadeCurves->getPreFadeValue(fade_);
    recFade_ = rec * fadeCurves->getRecFadeValue(fade_);

    sample_t y; // write value
    const sample_t* src = resamp_.output();

    for(int i=0; i<nframes; ++i) {
        y = src[i];

#if 1 // soft clipper
        y = clip_.processSample(y);
#endif
#if 0 // lowpass filter
        lpf_.processSample(&y);
#endif
        buf_[wrIdx_] *= preFade_;
        buf_[wrIdx_] += y * recFade_;

        wrIdx_ = wrapBufIndex(wrIdx_ + inc_dir_);
    }
}
#endif

float SubHead::peek() {
    return peek4();
}

float SubHead::peek4() {
    int phase1 = static_cast<int>(phase_);
    int phase0 = phase1 - 1;
    int phase2 = phase1 + 1;
    int phase3 = phase1 + 2;

    float y0 = buf_[wrapBufIndex(phase0)];
    float y1 = buf_[wrapBufIndex(phase1)];
    float y3 = buf_[wrapBufIndex(phase3)];
    float y2 = buf_[wrapBufIndex(phase2)];

    auto x = static_cast<float>(phase_ - (float)phase1);
    return Interpolate::hermite<float>(x, y0, y1, y2, y3);
}

unsigned int SubHead::wrapBufIndex(int x) {
    x += bufFrames_;
    BOOST_ASSERT_MSG(x >= 0, "buffer index before masking is non-negative");
    return static_cast<unsigned int>(x) & bufMask_;
}

void SubHead::setSampleRate(float sr) {
    //... nothing to do
}

void SubHead::setPhase(phase_t phase) {
    phase_ = phase;
    wrIdx_ = wrapBufIndex(static_cast<int>(phase_) + (inc_dir_ * recOffset_));

    // NB: not resetting the resampler here:
    // - it's ok to keep history of input when changing positions.
    // - resamp output doesn't need clearing b/c we write/read from beginning on each sample anyway
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// **NB** buffer size must be a power of two!!!!
void SubHead::setBuffer(float *buf, unsigned int frames) {
    buf_  = buf;
    bufFrames_ = frames;
    bufMask_ = frames - 1;
    BOOST_ASSERT_MSG((bufFrames_ != 0) && !(bufFrames_ & bufMask_), "buffer size is not 2^N");
}

void SubHead::setRate(rate_t rate) {
    rate_ = rate;
    inc_dir_ = boost::math::sign(rate);
    // NB: resampler doesn't handle negative rates.
    // instead we copy the resampler output backwards into the buffer when rate < 0.
    resamp_.setRate(std::fabs(rate));
}


void SubHead::setState(State state) { state_ = state; }

void SubHead::setRecOffsetSamples(int d) {
    recOffset_  = d;
}
*/
