//
// Created by ezra on 11/3/18.
//

#include <functional>
#include <softcut/Fades.h>

#include "softcut/Voice.h"
#include "softcut/Resampler.h"

using namespace softcut;

Voice::Voice() :
        rateRamp(48000, 0.1),
        preRamp(48000, 0.1),
        recRamp(48000, 0.1),
        preFilterFcBase(12000),
        preFilterFcMod(1.0),
        preFilterEnabled(true) {}

void Voice::reset() {
    preFilter.setCutoff(preFilterFcBase);
    preFilter.setQ(1.0);
    preFilterFcMod = 1.0;

    postFilter.setLpMix(0.0);
    postFilter.setHpMix(0.0);
    postFilter.setBpMix(0.0);
    postFilter.setBrMix(0.0);
    postFilter.setInverseQ(4.0);
    postFilter.setCutoff(12000);
    postFilterDryLevel = 1.0;

    setRecPreSlewTime(0.001);
    setRateSlewTime(0.001);

    recEnabled = false;
    playEnabled = false;

    rwh.init();
}

void Voice::processInputFilter(float *src, float *dst, size_t numFrames) {
    float fc, fcMod;
    for (size_t fr = 0; fr < numFrames; ++fr) {
        fcMod = std::fabs(rwh.getRateBuffer(fr));
        // FIXME: refactor
        fc = (preFilterFcMod*(preFilterFcBase*fcMod)) + ((1.f-preFilterFcMod)*preFilterFcBase);

        preFilter.setCutoff(std::fmax(0.f, std::fmin(16000.f, fc)));
        dst[fr] = preFilter.processSample(src[fr]);
    }
}

void Voice::setSampleRate(float hz) {
    sampleRate = hz;
    rateRamp.setSampleRate(hz);
    preRamp.setSampleRate(hz);
    recRamp.setSampleRate(hz);
    rwh.setSampleRate(hz);
    preFilter.init(hz);
    postFilter.setSampleRate(hz);
}

void Voice::setRate(float rate) {
    std::cout << "set rate target " << rate << std::endl;
    rateRamp.setTarget(rate);
    // FIXME: fix pre-filter smoothing
    //updatePreSvfFc();
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
    rwh.setPosition(sec);
}

void Voice::setPhase(phase_t phase) {
    rwh.enqueuePositionChange(phase);
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

// input filter
void Voice::setPreFilterFc(float x) {
    preFilterFcBase = x;
}

void Voice::setPreFilterFcMod(float x) {
    preFilterFcMod = x;
}

void Voice::setPreFilterEnabled(bool x) {
    preFilterEnabled = x;
}

// output filter
void Voice::setPostFilterFc(float x) {
    postFilter.setCutoff(x);
}

void Voice::setPostFilterRq(float x) {
    postFilter.setInverseQ(x);
}

void Voice::setPostFilterLp(float x) {
    postFilter.setLpMix(x);
}

void Voice::setPostFilterHp(float x) {
    postFilter.setHpMix(x);
}

void Voice::setPostFilterBp(float x) {
    postFilter.setBpMix(x);
}

void Voice::setPostFilterBr(float x) {
    postFilter.setBrMix(x);
}

void Voice::setPostFilterDry(float x) {
    // FIXME
    postFilterDryLevel = x;
}

void Voice::setBuffer(float *b, size_t nf) {
    buf = b;
    bufFrames = nf;
    rwh.setBuffer(buf, bufFrames);
}

void Voice::setRecOffset(float d) {
    rwh.setRecOffsetSamples(static_cast<int>(d * sampleRate));
}

void Voice::setRecPreSlewTime(float d) {
    recRamp.setTime(d);
    preRamp.setTime(d);
}

void Voice::setRateSlewTime(float d) {
    std::cout << "set rate time " << d << std::endl;
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
        quantPhase = rwh.getActivePhase() / sampleRate;
    } else {
        quantPhase = std::floor((rwh.getActivePhase() + phaseOffset) /
                                (sampleRate * phaseQuant)) * phaseQuant;
    }
}

bool Voice::getPlayFlag() const {
    return playEnabled;
}

bool Voice::getRecFlag() const {
    return recEnabled;
}

float Voice::getPos() const {
    return static_cast<float>(rwh.getActivePhase() / sampleRate);
}

void Voice::setPreFilterQ(float x) {
    preFilter.setQ(x);
}

void Voice::applyReadDuck(float *out, size_t numFrames) {
    const auto &phaseMine0 = rwh.head[0].phase.data();
    const auto &phaseMine1 = rwh.head[1].phase.data();
    const auto &fadeMine0 = rwh.head[0].fade.data();
    const auto &fadeMine1 = rwh.head[1].fade.data();
    const auto &phaseOther0 = readDuckTarget->rwh.head[0].phase.data();
    const auto &phaseOther1 = readDuckTarget->rwh.head[1].phase.data();
    const auto &recOther0 = readDuckTarget->rwh.head[0].rec.data();
    const auto &recOther1 = readDuckTarget->rwh.head[1].rec.data();
    const auto &preOther0 = readDuckTarget->rwh.head[0].pre.data();
    const auto &preOther1 = readDuckTarget->rwh.head[1].pre.data();
    for (size_t i=0; i<numFrames; ++i) {
        out[i] *= calcReadDuckFromPhasePair(phaseMine0[i], phaseOther0[i], recOther0[i], preOther0[i], fadeMine0[i]);
        out[i] *= calcReadDuckFromPhasePair(phaseMine0[i], phaseOther1[i], recOther1[i], preOther1[i], fadeMine0[i]);
        out[i] *= calcReadDuckFromPhasePair(phaseMine1[i], phaseOther0[i], recOther0[i], preOther0[i], fadeMine1[i]);
        out[i] *= calcReadDuckFromPhasePair(phaseMine1[i], phaseOther1[i], recOther1[i], preOther1[i], fadeMine1[i]);
    }
}

void Voice::applyWriteDuck(float *in, size_t numFrames) {
    const auto &phaseMine0 = rwh.head[0].phase.data();
    const auto &phaseMine1 = rwh.head[1].phase.data();
    const auto &phaseOther0 = readDuckTarget->rwh.head[0].phase.data();
    const auto &phaseOther1 = readDuckTarget->rwh.head[1].phase.data();
    const auto &recOther0 = readDuckTarget->rwh.head[0].rec.data();
    const auto &recOther1 = readDuckTarget->rwh.head[1].rec.data();
    const auto &preOther0 = readDuckTarget->rwh.head[0].pre.data();
    const auto &preOther1 = readDuckTarget->rwh.head[1].pre.data();
    for (size_t i=0; i<numFrames; ++i) {
        in[i] *= calcReadDuckFromPhasePair(phaseMine0[i], phaseOther0[i], recOther0[i], preOther0[i], fadeMine0[i]);
        in[i] *= calcReadDuckFromPhasePair(phaseMine0[i], phaseOther1[i], recOther1[i], preOther1[i], fadeMine0[i]);
        in[i] *= calcReadDuckFromPhasePair(phaseMine1[i], phaseOther0[i], recOther0[i], preOther0[i], fadeMine1[i]);
        in[i] *= calcReadDuckFromPhasePair(phaseMine1[i], phaseOther1[i], recOther1[i], preOther1[i], fadeMine1[i]);
    }
}



float Voice::calcReadDuckFromPhasePair(double a, double b, float r, float p, float f) {
    static constexpr float recMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float fadeMin = std::numeric_limits<float>::epsilon() * 2.f;
    static constexpr float preMax = 1.f - (std::numeric_limits<float>::epsilon() * 2.f);

    if (f <= fadeMin) { return 1.f; }
    if (r <= recMin && p >= preMax) { return 1.f; }

    phase_t d = fabs(a - b);

    // FIXME: these are in samples!
    // should be dynamic..?
    /// and probably scale with rate??
    static constexpr phase_t dmax = 1000;
    static constexpr phase_t dmin = 500;
    if (d > dmax) {
        return 1.f;
    }
    if (d < dmin) {
        return 0.f;
    }
    return Fades::raisedCosFadeIn(static_cast<float>(d-dmin)/static_cast<float>(dmax-dmin));
}


void Voice::updatePositions(size_t numFrames) {
    for (size_t fr = 0; fr < numFrames; ++fr) {
        rwh.setRate(fr, rateRamp.update());
    }

    if (followTarget == nullptr) {
        rwh.updateSubheadPositions(numFrames);
    } else {
        rwh.copySubheadPositions(followTarget->rwh, numFrames);
    }
}


void Voice::performReads(float *out, size_t numFrames) {
    if (playEnabled) {
        // TODO: use other voice for `duck`
        rwh.performSubheadReads(out, numFrames);
        // TODO: post-filter, phase poll
    }

    if (readDuckTarget != nullptr) {
        if (readDuckTarget->getRecFlag()) {
            applyReadDuck(out, numFrames);
        }
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
            rwh.setPre(fr, preRamp.update());
            rwh.setRec(fr, recRamp.update());
        }
        rwh.updateSubheadWriteLevels(numFrames);
        rwh.performSubheadWrites(src, numFrames);
    }
}

void Voice::syncPosition(const Voice &target, float offset) {
    phase_t newPhase = target.rwh.getActivePhase() + offset;
    // NB: relying on position change function to perform phase wrapping if needed
    rwh.enqueuePositionChange(newPhase);
}