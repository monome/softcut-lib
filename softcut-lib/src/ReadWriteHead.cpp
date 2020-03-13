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

int ReadWriteHead::dequeuePositionChange(
        SubHead::FramePositionData& a,
        SubHead::FramePositionData& b)
{
    if (enqueuedPosition < 0) {
        return -1;
    }
    if (a.opState == SubHead::Stopped) {
        a.opState = SubHead::FadeIn;
        a.phase = enqueuedPosition;
        enqueuedPosition = -1.0;
        return 0;
    }
    if (b.opState == SubHead::Stopped) {
        b.opState = SubHead::FadeIn;
        b.phase = enqueuedPosition;
        enqueuedPosition = -1.0;
        return 1;
    }
    return -1;
}

void ReadWriteHead::handleLoopAction(SubHead::OpAction action) {
    switch(action) {
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
    /// TODO [optimize]: persist this data structure
    SubHead::FramePositionParameters params{};
    params.start = start;
    params.end = end;
    params.loop = loopFlag;
    params.fadeInc = fadeInc;

    SubHead::FramePositionData data0{};
    SubHead::FramePositionData data1{};

    // start with the values in the buffer from end of last block
    head[0].loadFramePositionData(SubHead::maxBlockSize-1, data0);
    head[1].loadFramePositionData(SubHead::maxBlockSize-1, data1);

    for (size_t fr=0; fr<numFrames; ++fr) {
        params.rate = rate[fr];
        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        handleLoopAction (softcut::SubHead::calcPositionUpdate(data0, params));
        handleLoopAction (softcut::SubHead::calcPositionUpdate(data1, params));

        // change positions if needed
        int headMoved = dequeuePositionChange(data0, data1);
        // store new values in subhead buffers
        head[0].storeFramePositionData(fr, data0);
        head[1].storeFramePositionData(fr, data1);
        if (headMoved == 0) { head[0].updateWrIdx(fr, rate[fr], recOffsetSamples); }
        if (headMoved == 1) { head[1].updateWrIdx(fr, rate[fr], recOffsetSamples); }
    }
}

void ReadWriteHead::updateSubheadWriteLevels(size_t numFrames) {
    /// TODO [optimize]: persist this data structure
    SubHead::FrameLevelParameters params{};
    params.fadeCurves = fadeCurves;

    SubHead::FrameLevelData data0{};
    SubHead::FrameLevelData data1{};

    head[0].loadFrameLevelData(SubHead::maxBlockSize-1, data0);
    head[1].loadFrameLevelData(SubHead::maxBlockSize-1, data1);

    for (size_t fr=0; fr<numFrames; ++fr) {
        params.rate = this->rate[fr];
        params.pre = this->pre[fr];
        params.rec = this->rec[fr];

        // update phase/action/state for each subhead
        // this may result in a position change being enqueued
        softcut::SubHead::calcLevelUpdate(data0, params);
        softcut::SubHead::calcLevelUpdate(data1, params);

        // store new values in subhead buffers
        head[0].storeFrameLevelData(fr, data0);
        head[1].storeFrameLevelData(fr, data1);
    }
}


void ReadWriteHead::performSubheadWrites(const float *input, size_t numFrames) {
    SubHead::FrameWriteData data0{};
    SubHead::FrameWriteData data1{};
    for (size_t fr =0; fr<numFrames; ++fr) {
        head[0].performFrameWrite(data0, fr, input[fr]);
        head[1].performFrameWrite(data1, fr, input[fr]);
        head[0].storeFrameWriteData(fr, data0);
        head[1].storeFrameWriteData(fr, data1);
    }
}

void ReadWriteHead::performSubheadReads(float *output, size_t numFrames) {
    SubHead::FrameWriteData data0{};
    SubHead::FrameWriteData data1{};
    float out0;
    float out1;
    for (size_t fr =0; fr<numFrames; ++fr) {
        out0 = head[0].performFrameRead(fr);
        out1 = head[0].performFrameRead(fr);
        // TODO: apply `duck` here, using subhead record levels from other ReadWriteHead
        head[0].storeFrameWriteData(fr, data0);
        head[1].storeFrameWriteData(fr, data1);
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


//void ReadWriteHead::setRate(rate_t x) {
//
//}
//
//void ReadWriteHead::setRec(float x) {
//
//}
//
//void ReadWriteHead::setPre(float x) {
//
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


/*
void ReadWriteHead::init(FadeCurves *fc) {
    start = 0.f;
    end = 0.f;
    active = 0;
    rate = 1.f;
    setFadeTime(0.1f);
    testBuf.init();
    queuedCrossfade = 0;
    queuedCrossfadeFlag = false;
    head[0].init(fc);
    head[1].init(fc);
}

void ReadWriteHead::processSample(sample_t in, sample_t *out) {

    *out = mixFade(head[0].peek(), head[1].peek(), head[0].fade(), head[1].fade());

    BOOST_ASSERT_MSG(!(head[0].state_ == Playing && head[1].state_ == Playing), "multiple active heads");

    head[0].poke(in, pre, rec);
    head[1].poke(in, pre, rec);



    takeAction(head[0].updatePhase(start, end, loopFlag));
    takeAction(head[1].updatePhase(start, end, loopFlag));

    head[0].updateFade(fadeInc);
    head[1].updateFade(fadeInc);
    dequeueCrossfade();
}


void ReadWriteHead::processSampleNoRead(sample_t in, sample_t *out) {
    (void)out;

    BOOST_ASSERT_MSG(!(head[0].state_ == Playing && head[1].state_ == Playing), "multiple active heads");

    head[0].poke(in, pre, rec);
    head[1].poke(in, pre, rec);

    takeAction(head[0].updatePhase(start, end, loopFlag));
    takeAction(head[1].updatePhase(start, end, loopFlag));

    head[0].updateFade(fadeInc);
    head[1].updateFade(fadeInc);
    dequeueCrossfade();
}

void ReadWriteHead::processSampleNoWrite(sample_t in, sample_t *out) {
    (void)in;
    *out = mixFade(head[0].peek(), head[1].peek(), head[0].fade(), head[1].fade());

    BOOST_ASSERT_MSG(!(head[0].state_ == Playing && head[1].state_ == Playing), "multiple active heads");

    takeAction(head[0].updatePhase(start, end, loopFlag));
    takeAction(head[1].updatePhase(start, end, loopFlag));

    head[0].updateFade(fadeInc);
    head[1].updateFade(fadeInc);
    dequeueCrossfade();
}

void ReadWriteHead::setRate(rate_t x)
{
    rate = x;
    calcFadeInc();
    head[0].setRate(x);
    head[1].setRate(x);
}

void ReadWriteHead::setLoopStartSeconds(float x)
{
    start = x * sr;
}

void ReadWriteHead::setLoopEndSeconds(float x)
{
    end = x * sr;
}

void ReadWriteHead::takeAction(Action act)
{
    switch (act) {
        case Action::LoopPos:
            enqueueCrossfade(start);
            break;
        case Action::LoopNeg:
            enqueueCrossfade(end);
            break;
        case Action::Stop:
            break;
        case Action::None:
        default: ;;
    }
}

void ReadWriteHead::enqueueCrossfade(phase_t pos) {
    // std::cout <<"enqueuing crossfade\n";
    queuedCrossfade = pos;
    queuedCrossfadeFlag = true;
}

void ReadWriteHead::dequeueCrossfade() {
    State s = head[active].state();
    if(! (s == State::FadeIn || s == State::FadeOut)) {
	if(queuedCrossfadeFlag ) {
	    // std::cout <<"dequeuing crossfade\n";
	    cutToPhase(queuedCrossfade);
	}
	queuedCrossfadeFlag = false;
    }
}


void ReadWriteHead::cutToPhase(phase_t pos) {
    State s = head[active].state();

    if(s == State::FadeIn || s == State::FadeOut) {
	// should never enter this condition
	// cout << "bleeeeaaaaaaaaaaaaargh!!!!\n";
	return;
    }

    // activate the inactive head
    int newActive = active ^ 1;
    if(s != State::Stopped) {
        head[active].setState(State::FadeOut);
    }

    head[newActive].setState(State::FadeIn);
    head[newActive].setPhase(pos);

    head[active].active_ = false;
    head[newActive].active_ = true;
    active = newActive;
}

void ReadWriteHead::setFadeTime(float secs) {
    fadeTime = secs;
    calcFadeInc();
}
void ReadWriteHead::calcFadeInc() {
    fadeInc = (float) fabs(rate) / std::max(1.f, (fadeTime * sr));
    fadeInc = std::max(0.f, std::min(fadeInc, 1.f));
}

void ReadWriteHead::setBuffer(float *b, uint32_t bf) {
    buf = b;
    head[0].setBuffer(b, bf);
    head[1].setBuffer(b, bf);
}

void ReadWriteHead::setLoopFlag(bool val) {
    loopFlag = val;
}

void ReadWriteHead::setSampleRate(float sr_) {
    sr = sr_;
    head[0].setSampleRate(sr);
    head[1].setSampleRate(sr);
}

sample_t ReadWriteHead::mixFade(sample_t x, sample_t y, float a, float b) {
    // TODO [efficiency]: try low-order polynomial approximation
    return x * sinf(a * (float)M_PI_2) + y * sinf(b * (float) M_PI_2);
}

void ReadWriteHead::setRec(float x) {
    rec = x;
}

void ReadWriteHead::setPre(float x) {
    pre = x;
}

phase_t ReadWriteHead::getActivePhase() {
  return head[active].phase();
}

void ReadWriteHead::cutToPos(float seconds) {
    enqueueCrossfade(seconds * sr);
}

rate_t ReadWriteHead::getRate() {
    return rate;
}

void ReadWriteHead::setRecOffsetSamples(int d) {
    head[0].setRecOffsetSamples(d);
    head[1].setRecOffsetSamples(d);
}

 */
//void ReadWriteHead::processSample(sample_t in, sample_t *out) {
//    SubHead::FramePositionData subFrameResult[2];
//}
