//
// Created by ezra on 4/21/18.
//

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

void SubHead::setPosition(size_t idx, phase_t position, const softcut::ReadWriteHead *rwh) {
    if (opState[idx] != Stopped) {
        std::cerr << "error: setting position of moving subhead" << std::endl;
    }
    phase[idx] = position;
    opState[idx] = SubHead::FadeIn;
    opAction[idx] = SubHead::OpAction::StartFadeIn;
    updateWrIdx(idx, rwh->rate[idx], rwh->recOffsetSamples);
}

SubHead::OpAction SubHead::calcPositionUpdate(size_t i_1, size_t i,
//                                     const SubHead::FramePositionParameters &a) {
                                              const softcut::ReadWriteHead *rwh) {
    updateRate(i_1, rwh->rate[i]);

    switch (opState[i_1]) {
        case FadeIn:
            phase[i] = phase[i_1] + rwh->rate[i];
            fade[i] = fade[i_1] + rwh->fadeInc;
            if (fade[i] >= 1.f) {
                fade[i] = 1.f;
                std::cerr << "done fadein" << std::endl;
                opState[i] = Playing;
                opAction[i] = DoneFadeIn;
            } else {
                opState[i] = FadeIn;
                opAction[i] = None;
            }
            break;
        case FadeOut:
            phase[i] = phase[i_1] + rwh->rate[i];
            fade[i] = fade[i] - rwh->fadeInc;
            if (fade[i] <= 0.f) {
                std::cerr << "done fadeout" << std::endl;
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
                if (phase[i] > rwh->end || phase[i] < rwh->start) { // out of loop bounds
                    opState[i] = FadeOut;
                    if (rwh->loopFlag) {
                        std::cerr << "looping positive" << std::endl;
                        opAction[i] = LoopPositive;
                    } else {
                        std::cerr << "stopping after loop end" << std::endl;
                        opAction[i] = Stop;
                    }
                } else { // in loop bounds
                    opAction[i] = None;
                    opState[i] = Playing;
                }
            } else { // negative rate
                if (phase[i] > rwh->end || phase[i] < rwh->start) { // out of loop bounds
                    opState[i] = FadeOut;
                    if (rwh->loopFlag) {
                        std::cerr << "looping negative" << std::endl;
                        opAction[i] = LoopNegative;
                    } else {
                        std::cerr << "stopping after loop end" << std::endl;
                        opAction[i] = Stop;
                    }
                } else { // in loop bounds
                    opAction[i] = None;
                    opState[i] = Playing;
                    fade[i] = 0.f;
                }
            }
            break;
        case Stopped:
            fade[i] = 0.f;
            opAction[i] = None;
            opState[i] = Stopped;
    }

    return opAction[i];
}

void SubHead::calcLevelUpdate(size_t i,
        //const FrameLevelParameters &a) {
                              const softcut::ReadWriteHead *rwh) {
    pre[i] = rwh->pre[i] + (1.f - pre[i]) * rwh->fadeCurves->getPreFadeValue(fade[i]);
    rec[i] = rwh->rec[i] * rwh->fadeCurves->getRecFadeValue(fade[i]);
    // TODO: apply rate==0 deadzone
}


void SubHead::performFrameWrite(size_t i_1, size_t i, const float input) {
    // push to resampler, even if stopped
    // this should avoid a glitch when restarting
    int nframes = resamp.processFrame(input);

    // if we're stopped, don't touch our buffer or state vars
    if (opState[i_1] == Stopped) {
        return;
    }

    BOOST_ASSERT_MSG(fade[i_1] >= 0.f && fade[i_1] <= 1.f, "bad fade coefficient in write");

    sample_t y; // write value
    const sample_t *src = resamp.output();

    size_t w = wrIdx[i_1];
    for (int fr = 0; fr < nframes; ++fr) {
        y = src[fr];
        // TODO: possible further processing (e.g. softclip, filtering)
        buf[w] *= pre[i_1];
        buf[w] += y * rec[i_1];
        w = wrapBufIndex(w + dir[i_1]);
    }
    wrIdx[i] = w;
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
