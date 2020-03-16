//
// Created by ezra on 11/3/18.
//

#include <functional>

#include "softcut/Voice.h"
#include "softcut/Resampler.h"

using namespace softcut;

Voice::Voice() :
rateRamp(48000, 0.1),
preRamp(48000, 0.1),
recRamp(48000, 0.1)
{
    svfPreFcBase = 16000;
}

void Voice::reset() {
    fadeCurves.init();
    svfPre.setLpMix(1.0);
    svfPre.setHpMix(0.0);
    svfPre.setBpMix(0.0);
    svfPre.setBrMix(0.0);
    svfPre.setRq(4.0);
    svfPre.setFc(svfPreFcBase);
    svfPreFcMod = 1.0;
    svfPreDryLevel = 0.0;

    svfPost.setLpMix(0.0);
    svfPost.setHpMix(0.0);
    svfPost.setBpMix(0.0);
    svfPost.setBrMix(0.0);
    svfPost.setRq(4.0);
    svfPost.setFc(12000);
    svfPostDryLevel = 1.0;

    setRecPreSlewTime(0.001);
    setRateSlewTime(0.001);

    recFlag = false;
    playFlag = false;

    sch.init(&fadeCurves);
}

void Voice:: processBlockMono(float *in, float *out, int numFrames) {

    for(size_t fr=0; fr<numFrames; ++fr) {
        sch.setRate(fr, rateRamp.update());
    }

    // TODO: use other voice for `follow`
    sch.updateSubheadPositions(numFrames);

    if (playFlag) {
        // TODO: use other voice for `duck`
        sch.performSubheadReads(out, numFrames);
        // TODO: post-filter, phase poll
    }

    if (recFlag) {
        sch.updateSubheadWriteLevels(numFrames);
        for(size_t fr=0; fr<numFrames; ++fr) {
            sch.setPre(fr, preRamp.update());
            sch.setRec(fr, recRamp.update());
            // TODO: pre-filter?
        }
        sch.performSubheadWrites(in, numFrames);
    }


}

void Voice::setSampleRate(float hz) {
    sampleRate = hz;
    rateRamp.setSampleRate(hz);
    preRamp.setSampleRate(hz);
    recRamp.setSampleRate(hz);
    sch.setSampleRate(hz);
    svfPre.setSampleRate(hz);
    svfPost.setSampleRate(hz);
}

void Voice::setRate(float rate) {    
    rateRamp.setTarget(rate);
    // FIXME: fix pre-filter smoothing
    //updatePreSvfFc();
}

void Voice::setLoopStart(float sec) {
    sch.setLoopStartSeconds(sec);
}

void Voice::setLoopEnd(float sec) {
    sch.setLoopEndSeconds(sec);
}

void Voice::setFadeTime(float sec) {
    sch.setFadeTime(sec);
}

void Voice::setPosition(float sec) {
    sch.setPosition(sec);
}

void Voice::setRecLevel(float amp) {
    recRamp.setTarget(amp);
}

void Voice::setPreLevel(float amp) {
    preRamp.setTarget(amp);
}

void Voice::setRecFlag(bool val) {
    recFlag = val;
}


void Voice::setPlayFlag(bool val) {
    playFlag = val;
}

void Voice::setLoopFlag(bool val) {
    sch.setLoopFlag(val);
}

// input filter
void Voice::setPreFilterFc(float x) {
    svfPreFcBase = x;
    // FIXME
    // updatePreSvfFc();
}

void Voice::setPreFilterRq(float x) {
    svfPre.setRq(x);
}

void Voice::setPreFilterLp(float x) {
    svfPre.setLpMix(x);
}

void Voice::setPreFilterHp(float x) {
    svfPre.setHpMix(x);
}

void Voice::setPreFilterBp(float x) {
    svfPre.setBpMix(x);
}

void Voice::setPreFilterBr(float x) {
    svfPre.setBrMix(x);
}

void Voice::setPreFilterDry(float x) {
    svfPreDryLevel = x;
}

void Voice::setPreFilterFcMod(float x) {
    svfPreFcMod = x;
}

/// FIXME: needs to be per-sample
//void Voice::updatePreSvfFc() {
//    float fcMod = std::min(svfPreFcBase, svfPreFcBase * std::fabs(static_cast<float>(sch.getRate())));
//    fcMod = svfPreFcBase + svfPreFcMod * (fcMod - svfPreFcBase);
//    svfPre.setFc(fcMod);
//}

// output filter
void Voice::setPostFilterFc(float x) {
    svfPost.setFc(x);
}

void Voice::setPostFilterRq(float x) {
    svfPost.setRq(x);
}

void Voice::setPostFilterLp(float x) {
    svfPost.setLpMix(x);
}

void Voice::setPostFilterHp(float x) {
    svfPost.setHpMix(x);
}

void Voice::setPostFilterBp(float x) {
    svfPost.setBpMix(x);
}

void Voice::setPostFilterBr(float x) {
    svfPost.setBrMix(x);
}

void Voice::setPostFilterDry(float x) {
    // FIXME
    svfPostDryLevel = x;
}

void Voice::setBuffer(float *b, unsigned int nf) {
    buf = b;
    bufFrames = nf;
    sch.setBuffer(buf, bufFrames);
}

void Voice::setRecOffset(float d) {
    sch.setRecOffsetSamples(static_cast<int>(d * sampleRate));
}

void Voice::setRecPreSlewTime(float d) {
    recRamp.setTime(d);
    preRamp.setTime(d);
}

void Voice::setRateSlewTime(float d) {
    rateRamp.setTime(d);
}

void Voice::setPhaseQuant(float x) {
    phaseQuant = x;
}

void Voice::setPhaseOffset(float x) {
    phaseOffset = x * sampleRate;
}

phase_t Voice::getQuantPhase() {
    return quantPhase;
}

void Voice::updateQuantPhase() {
    if (phaseQuant == 0) {
        quantPhase = sch.getActivePhase() / sampleRate;
    } else {
        quantPhase = std::floor( (sch.getActivePhase() + phaseOffset) /
            (sampleRate *phaseQuant)) * phaseQuant;
    }
}

bool Voice::getPlayFlag() {
    return playFlag;
}

bool Voice::getRecFlag() {
    return recFlag;
}

float Voice::getPos() {
    return static_cast<float>(sch.getActivePhase() / sampleRate);
}
