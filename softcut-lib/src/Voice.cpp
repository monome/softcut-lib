//
// Created by ezra on 11/3/18.
//

#include <functional>
#include <dsp-kit/abs.hpp>

#include "dsp-kit/clamp.hpp"

#include "softcut/Fades.h"
#include "softcut/Resampler.h"
#include "softcut/Tables.h"
#include "softcut/Voice.h"

using namespace softcut;

Voice::Voice() {
    auto &tables = Tables::shared();
    preFilter.setGTable(tables.getLadderLpfGTable(), Tables::ladderLpfGTableSize);
    postFilter.setGTable(tables.getSvfGTable(), Tables::svfGTableSize);
    setSampleRate(48000);
    reset();
}

void Voice::reset() {
    preFilterFcMod = 1.0;
    preFilterEnabled = true;
    preFilter.setQ(1.0);

    filterParamEnvFrameCount = 0;
    filterLevelEnvFrameCount = 0;

    postFilterFcRamp.setTime(0.1f);
    postFilterFcRamp.setTarget(1.0);
    postFilterRqRamp.setTime(0.1f);
    postFilterRqRamp.setTarget(1.0);

    postFilterLevelRamp[SVF_LP].setTime(0.1);
    postFilterLevelRamp[SVF_HP].setTime(0.1);
    postFilterLevelRamp[SVF_BP].setTime(0.1);
    postFilterLevelRamp[SVF_BR].setTime(0.1);
    postFilterLevelRamp[SVF_DRY].setTime(0.1);

    postFilterLevelRamp[SVF_LP].setTarget(0);
    postFilterLevelRamp[SVF_HP].setTarget(0);
    postFilterLevelRamp[SVF_BP].setTarget(0);
    postFilterLevelRamp[SVF_BR].setTarget(0);
    postFilterLevelRamp[SVF_DRY].setTarget(1.f);

    postFilterEnabled = true;
    rateRamp.setTime(0.1);
    rateRamp.setValue(1.0);
    preRamp.setTime(0.1);
    preRamp.setPos(0.0);
    recRamp.setTime(0.1);
    recRamp.setPos(0.0);

    setRecPreSlewTime(0.01);
    setRateSlewTime(0.01);

    recEnabled = false;
    playEnabled = false;

    rwh.init();
}

void Voice::processInputFilter(float *src, float *dst, size_t numFrames) {
    float fc, fcMod;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        fcMod = dspkit::abs<phase_t>(rwh.getRateBuffer(fr));
        // fc = preFilterFcMod * fcMod * preFilterFcBase + (1 - preFilterFcMod) * preFilterFcBase;
        // refactored:
        fc = preFilterFcBase * (preFilterFcMod * (fcMod - 1) + 1);
        preFilter.setCutoffPitch(fc);
        dst[fr] = preFilter.processSample(src[fr]);
    }
}

void Voice::updatePositions(size_t numFrames) {
    for (size_t fr = 0; fr < numFrames; ++fr) {
        rwh.setRate(fr, rateRamp.getNextValue());
    }
    const Voice *target = followTarget;
    if (target == nullptr) {
        rwh.updateSubheadState(numFrames);
    } else {
        rwh.copySubheadPositions(target->rwh, numFrames);
    }
}


void Voice::performReads(float *out, size_t numFrames) {
    if (playEnabled) {
        rwh.performSubheadReads(out, numFrames);
    }

    const Voice *target = readDuckTarget;
    if (target != nullptr) {
        if (target->getRecFlag()) {
            rwh.computeReadDuckLevels(&(target->rwh), numFrames);
            rwh.applyReadDuckLevels(out, numFrames);
        }
    }
    if (postFilterEnabled) {
        processOutputFilter(out, numFrames);
    }
}

void Voice::performWrites(float *in, size_t numFrames) {
    if (recEnabled) {
        // NB: could move filter outside of recEnabled,
        // consuming CPU but reducing clicks on rec toggle
        float *src;
        if (preFilterEnabled) {
            src = preFilterInputBuf.data();
            processInputFilter(in, src, numFrames);
        } else {
            src = in;
        }

        for (size_t fr = 0; fr < numFrames; ++fr) {
            src[fr] = dcBlocker.processSample(src[fr]);
        }
        for (size_t fr = 0; fr < numFrames; ++fr) {
            rwh.setPre(fr, preRamp.getNextValue());
            rwh.setRec(fr, recRamp.getNextValue());
        }

        rwh.updateSubheadWriteLevels(numFrames);

        const Voice *target = writeDuckTarget;
        if (target != nullptr) {
            if (target->getRecFlag()) {
                rwh.computeWriteDuckLevels(&(target->rwh), numFrames);
                rwh.applyWriteDuckLevels(numFrames);
            }
        }

        rwh.performSubheadWrites(src, numFrames);
    }
}

void Voice::syncPosition(const Voice &target, float offset) {
    // FIXME: this should set a flag to sync phases at top of process block
}


void Voice::updateQuantPhase() {
    if (phaseQuant == 0) {
        quantPhase = rwh.getActivePhase() / sampleRate;
    } else {
        quantPhase = std::floor((rwh.getActivePhase() + phaseOffset) /
                                (sampleRate * phaseQuant)) * phaseQuant;
    }
}

//=====================================
//==== setters


void Voice::setSampleRate(float hz) {
    sampleRate = hz;
    rwh.setSampleRate(hz);
    preFilter.init(hz);
    postFilter.setSampleRate(hz);
    dcBlocker.init(hz, 20);
    for (auto &ramp : postFilterLevelRamp) {
        ramp.setSampleRate(hz / FILTER_LEVEL_ENV_SR_DIVISOR);
    }
    postFilterFcRamp.setSampleRate(hz / FILTER_PARAM_ENV_SR_DIVISOR);
    postFilterRqRamp.setSampleRate(hz / FILTER_PARAM_ENV_SR_DIVISOR);
    rateRamp.setSampleRate(hz);
    preRamp.setSampleRate(hz);
    recRamp.setSampleRate(hz);
}

void Voice::setRate(float rate) {
    rateRamp.setTarget(rate);
}

void Voice::setLoopStart(float sec) {
    rwh.setLoopStartSeconds(sec);
}

void Voice::setLoopEnd(float sec) {
    rwh.setLoopEndSeconds(sec);
}

void Voice::setFadeTime(float sec) {
    rwh.setFadeTime(sec);
}

void Voice::setPosition(float sec) {
    rwh.requestPosition(sec);
}

void Voice::setRecLevel(float amp) {
    recRamp.setTarget(amp);
}

void Voice::setPreLevel(float amp) {
    preRamp.setTarget(amp);
}

void Voice::setRecFlag(bool val) {
    recEnabled = val;
}

void Voice::setPlayFlag(bool val) {
    playEnabled = val;
}

void Voice::setLoopFlag(bool val) {
    rwh.setLoopFlag(val);
}

void Voice::setLoopMode(LoopMode loopMode) {
    rwh.setLoopMode(loopMode);
}

//-------------------------------------------------------------
//--- input filter
void Voice::setPreFilterFc(float x) {
    /// FIXME: we are pretending that the pre-filter cutoff is MIDI-scaled, but it's not really
    /// (it's kinda close)
    x /= 127.f;
    preFilterFcBase = x < 0 ? 0 : (x > 1.f ? 1.f : x);
}

void Voice::setPreFilterFcMod(float x) {
    preFilterFcMod = x;
}

void Voice::setPreFilterEnabled(bool x) {
    preFilterEnabled = x;
}

//-------------------------------------------------------------
//--- output filter

void Voice::setPostFilterEnabled(bool x) {
    postFilterEnabled = x;
}

void Voice::setPostFilterFc(float x) {
    /// NB: magic here because we know that the FC gain table is midi-scaled
    postFilterFcRamp.setTarget(x / 127.f);
}

void Voice::setPostFilterRq(float x) {
    postFilterRqRamp.setTarget(x);
}

void Voice::setPostFilterLp(float x) {
    postFilterLevelRamp[SVF_LP].setTarget(x);
}

void Voice::setPostFilterHp(float x) {
    postFilterLevelRamp[SVF_HP].setTarget(x);
}

void Voice::setPostFilterBp(float x) {
    postFilterLevelRamp[SVF_BP].setTarget(x);
}

void Voice::setPostFilterBr(float x) {
    postFilterLevelRamp[SVF_BR].setTarget(x);
}

void Voice::setPostFilterDry(float x) {
    postFilterLevelRamp[SVF_DRY].setTarget(x);
}

//-------------------------------------------------------------
//--- buffer
void Voice::setBuffer(float *b, size_t nf) {
    buf = b;
    bufFrames = nf;
    rwh.setBuffer(buf, bufFrames);
}

//-------------------------------------------------------------
//--- record parameters
void Voice::setRecOffset(float d) {
    rwh.setRecOffsetSamples(static_cast<int>(d * sampleRate));
}

void Voice::setRecPreSlewTime(float d) {
    recRamp.setTime(d);
    preRamp.setTime(d);
}

void Voice::setRateSlewTime(float d) {
    rateRamp.setTime(d);
}
void Voice::setRateSlewShape(int shape) {
    rateRamp.setRiseShape(shape);
    rateRamp.setFallShape(shape);
}


//-------------------------------------------------------------
//--- phase update parameters
void Voice::setPhaseQuant(float x) {
    phaseQuant = x;
}

void Voice::setPhaseOffset(float x) {
    phaseOffset = x * sampleRate;
}

phase_t Voice::getQuantPhase() {
    return quantPhase;
}

bool Voice::getPlayFlag() const {
    return playEnabled;
}

bool Voice::getRecFlag() const {
    return recEnabled;
}

void Voice::setPreFilterQ(float x) {
    preFilter.setQ(x);
}

void Voice::setReadDuckTarget(Voice *v) {
    readDuckTarget = v;
}

void Voice::setWriteDuckTarget(Voice *v) {
    writeDuckTarget = v;
}

void Voice::setFollowTarget(Voice *v) {
    followTarget = v;
}

void Voice::processOutputFilter(float *buf, size_t numFrames) {
    for (size_t fr = 0; fr < numFrames; ++fr) {
        if (++filterLevelEnvFrameCount == FILTER_LEVEL_ENV_SR_DIVISOR) {
            filterLevelEnvFrameCount = 0;
            postFilterDryLevel = postFilterLevelRamp[SVF_DRY].getNextValue();
            postFilter.setLpMix(postFilterLevelRamp[SVF_LP].getNextValue());
            postFilter.setHpMix(postFilterLevelRamp[SVF_HP].getNextValue());
            postFilter.setBpMix(postFilterLevelRamp[SVF_BP].getNextValue());
            postFilter.setBrMix(postFilterLevelRamp[SVF_BR].getNextValue());
        }
        if (++filterParamEnvFrameCount == FILTER_PARAM_ENV_SR_DIVISOR) {
            filterParamEnvFrameCount = 0;
#if 0 // testing..
            //float hz = 110.f * powf(2.f, 4.f * postFilterFcRamp.getNextValue());
            float pitch = postFilterFcRamp.getNextValue();
            float hz = dspkit::Conversion<float>::midihz(pitch * 127.f);
            hz = std::fmax(0, std::min(12000.f, hz));
            postFilter.setCutoff(hz);
            postFilter.setInverseQ(postFilterRqRamp.getNextValue());
#else
            postFilter.setCutoffPitchNoCalc(postFilterFcRamp.getNextValue());
            postFilter.setInverseQNoCalc(postFilterRqRamp.getNextValue());
            postFilter.calcSecondaryCoeffs();
#endif
        }
        buf[fr] = (buf[fr] * postFilterDryLevel) + postFilter.processSample(buf[fr]);
    }
}

void Voice::setLevelSlewTime(float t) {
    postFilterLevelRamp[SVF_LP].setTime(t);
    postFilterLevelRamp[SVF_HP].setTime(t);
    postFilterLevelRamp[SVF_BP].setTime(t);
    postFilterLevelRamp[SVF_BR].setTime(t);
    postFilterLevelRamp[SVF_DRY].setTime(t);
}

void Voice::setPostFilterFcSlewTime(float t) {
    postFilterFcRamp.setTime(t);
}

void Voice::setPostFilterRqSlewTime(float t) {
    postFilterRqRamp.setTime(t);
}

void Voice::setPostFilterFcRiseShape(int shape) {
    postFilterFcRamp.setRiseShape(shape);
}

void Voice::setPostFilterFcFallShape(int shape) {
    postFilterFcRamp.setFallShape(shape);
}

void Voice::setPostFilterRqRiseShape(int shape) {
    postFilterRqRamp.setRiseShape(shape);
}

void Voice::setPostFilterRqFallShape(int shape) {
    postFilterRqRamp.setFallShape(shape);
}
