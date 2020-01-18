//
// Created by ezra on 11/3/18.
//

#include "SoftcutVoice.h"
#include <functional>

using namespace softcut;

SoftcutVoice::SoftcutVoice() :
rateRamp(48000, 0.1),
preRamp(48000, 0.1),
recRamp(48000, 0.1)
{
    svfPreFcBase = 16000;
    reset();
}

void SoftcutVoice::reset() {
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

    sch.init();

}

void SoftcutVoice:: processBlockMono(const float *in, float *out, int numFrames) {
    std::function<void(sample_t, sample_t*)> sampleFunc;
    if(playFlag) {
        if(recFlag) {
            sampleFunc = [this](float in, float* out) {
                this->sch.processSample(in, out);
            };
        } else {
            sampleFunc = [this](float in, float* out) {
                this->sch.processSampleNoWrite(in, out);
            };
        }
    } else {
        if(recFlag) {
            sampleFunc = [this](float in, float* out) {
                this->sch.processSampleNoRead(in, out);
            };
        } else {
            // FIXME? do nothing, i guess?
            sampleFunc = [](float in, float* out) {
                (void)in;
                (void)out;
            };
        }
    }

    float x, y;
    for(int i=0; i<numFrames; ++i) {
        x = svfPre.getNextSample(in[i]) + in[i]*svfPreDryLevel;
        sch.setRate(rateRamp.update());
        sch.setPre(preRamp.update());
        sch.setRec(recRamp.update());
        sampleFunc(x, &y);
	    out[i] = svfPost.getNextSample(y) + y*svfPostDryLevel;
        updateQuantPhase();
    }
}

void SoftcutVoice::setSampleRate(float hz) {
    sampleRate = hz;
    rateRamp.setSampleRate(hz);
    preRamp.setSampleRate(hz);
    recRamp.setSampleRate(hz);
    sch.setSampleRate(hz);
    svfPre.setSampleRate(hz);
    svfPost.setSampleRate(hz);
}

void SoftcutVoice::setRate(float rate) {
    rateRamp.setTarget(rate);
    updatePreSvfFc();
}

void SoftcutVoice::setLoopStart(float sec) {
    sch.setLoopStartSeconds(sec);
}

void SoftcutVoice::setLoopEnd(float sec) {
    sch.setLoopEndSeconds(sec);
}

void SoftcutVoice::setFadeTime(float sec) {
    sch.setFadeTime(sec);
}

void SoftcutVoice::cutToPos(float sec) {
    sch.cutToPos(sec);
}

void SoftcutVoice::setRecLevel(float amp) {
    recRamp.setTarget(amp);
}

void SoftcutVoice::setPreLevel(float amp) {
    preRamp.setTarget(amp);
}

void SoftcutVoice::setRecFlag(bool val) {
    recFlag = val;
}


void SoftcutVoice::setPlayFlag(bool val) {
    playFlag = val;
}

void SoftcutVoice::setLoopFlag(bool val) {
    sch.setLoopFlag(val);
}

// input filter
void SoftcutVoice::setPreFilterFc(float x) {
    svfPreFcBase = x;
    updatePreSvfFc();
}

void SoftcutVoice::setPreFilterRq(float x) {
    svfPre.setRq(x);
}

void SoftcutVoice::setPreFilterLp(float x) {
    svfPre.setLpMix(x);
}

void SoftcutVoice::setPreFilterHp(float x) {
    svfPre.setHpMix(x);
}

void SoftcutVoice::setPreFilterBp(float x) {
    svfPre.setBpMix(x);
}

void SoftcutVoice::setPreFilterBr(float x) {
    svfPre.setBrMix(x);
}

void SoftcutVoice::setPreFilterDry(float x) {
    svfPreDryLevel = x;
}

void SoftcutVoice::setPreFilterFcMod(float x) {
    svfPreFcMod = x;
}

void SoftcutVoice::updatePreSvfFc() {
    float fcMod = std::min(svfPreFcBase, svfPreFcBase * std::fabs(static_cast<float>(sch.getRate())));
    fcMod = svfPreFcBase + svfPreFcMod * (fcMod - svfPreFcBase);
    svfPre.setFc(fcMod);
}

// output filter
void SoftcutVoice::setPostFilterFc(float x) {
    svfPost.setFc(x);
}

void SoftcutVoice::setPostFilterRq(float x) {
    svfPost.setRq(x);
}

void SoftcutVoice::setPostFilterLp(float x) {
    svfPost.setLpMix(x);
}

void SoftcutVoice::setPostFilterHp(float x) {
    svfPost.setHpMix(x);
}

void SoftcutVoice::setPostFilterBp(float x) {
    svfPost.setBpMix(x);
}

void SoftcutVoice::setPostFilterBr(float x) {
    svfPost.setBrMix(x);
}

void SoftcutVoice::setPostFilterDry(float x) {
    // FIXME
    svfPostDryLevel = x;
}

void SoftcutVoice::setBuffer(float *b, unsigned int nf) {
    buf = b;
    bufFrames = nf;
    sch.setBuffer(buf, bufFrames);
}

void SoftcutVoice::setRecOffset(float d) {
    sch.setRecOffsetSamples(static_cast<int>(d * sampleRate));
}

void SoftcutVoice::setRecPreSlewTime(float d) {
    recRamp.setTime(d);
    preRamp.setTime(d);
}

void SoftcutVoice::setRateSlewTime(float d) {
    rateRamp.setTime(d);
}

void SoftcutVoice::setPhaseQuant(float x) {
    phaseQuant = x;
}

void SoftcutVoice::setPhaseOffset(float x) {
    phaseOffset = x * sampleRate;
}


phase_t SoftcutVoice::getQuantPhase() {
    return quantPhase;
}

void SoftcutVoice::updateQuantPhase() {
    if (phaseQuant == 0) {
        quantPhase = sch.getActivePhase() / sampleRate;
    } else {
        quantPhase = std::floor( (sch.getActivePhase() + phaseOffset) /
            (sampleRate *phaseQuant)) * phaseQuant;
    }
}

bool SoftcutVoice::getPlayFlag() {
    return playFlag;
}

bool SoftcutVoice::getRecFlag() {
    return recFlag;
}

float SoftcutVoice::getPos() {
    return static_cast<float>(sch.getActivePhase() / sampleRate);
}
