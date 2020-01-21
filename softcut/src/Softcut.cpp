#include "softcut/Softcut.h"
using namespace softcut;

void Softcut::init(int nv) {
    numVoices = nv;
    scv = std::make_unique<SoftcutVoice[]>(numVoices);
    // FIXME: fadecurves shouldn't be static    
    FadeCurves::setPreShape(FadeCurves::Shape::Linear);
    FadeCurves::setRecShape(FadeCurves::Shape::Raised);
    FadeCurves::setMinPreWindowFrames(0);
    FadeCurves::setMinRecDelayFrames(0);
    FadeCurves::setPreWindowRatio(1.f / 8);
    FadeCurves::setRecDelayRatio(1.f / (8 * 16));
}

Softcut::Softcut() {
    this->init();
    this->reset();
}

void Softcut::reset() {
    for (int v = 0; v < numVoices; ++v) {
	scv[v].reset();
    };
}

// assumption: channel count is equal to voice count!
void Softcut::processBlock(int v, const float *in, float *out, int numFrames) {
    scv[v].processBlockMono(in, out, numFrames);
}

void Softcut::setSampleRate(unsigned int hz) {
    for (auto &v : scv) {
	v.setSampleRate(hz);
    }
}

void Softcut::setRate(int voice, float rate) {
    scv[voice].setRate(rate);
}

void Softcut::setLoopStart(int voice, float sec) {
    scv[voice].setLoopStart(sec);
}

void Softcut::setLoopEnd(int voice, float sec) {
    scv[voice].setLoopEnd(sec);
}

void Softcut::setLoopFlag(int voice, bool val) {
    scv[voice].setLoopFlag(val);
}

void Softcut::setFadeTime(int voice, float sec) {
    scv[voice].setFadeTime(sec);
}

void Softcut::setRecLevel(int voice, float amp) {
    scv[voice].setRecLevel(amp);
}

void Softcut::setPreLevel(int voice, float amp) {
    scv[voice].setPreLevel(amp);
}

void Softcut::setRecFlag(int voice, bool val) {
    scv[voice].setRecFlag(val);
}

void Softcut::setPlayFlag(int voice, bool val) {
    scv[voice].setPlayFlag(val);
}

void Softcut::cutToPos(int voice, float sec) {
    scv[voice].cutToPos(sec);
}

void Softcut::setPreFilterFc(int voice, float x) {
    scv[voice].setPreFilterFc(x);
}

void Softcut::setPreFilterRq(int voice, float x) {
    scv[voice].setPreFilterRq(x);
}

void Softcut::setPreFilterLp(int voice, float x) {
    scv[voice].setPreFilterLp(x);
}

void Softcut::setPreFilterHp(int voice, float x) {
    scv[voice].setPreFilterHp(x);
}

void Softcut::setPreFilterBp(int voice, float x) {
    scv[voice].setPreFilterBp(x);
}

void Softcut::setPreFilterBr(int voice, float x) {
    scv[voice].setPreFilterBr(x);
}

void Softcut::setPreFilterDry(int voice, float x) {
    scv[voice].setPreFilterDry(x);
}

void Softcut::setPreFilterFcMod(int voice, float x) {
    scv[voice].setPreFilterFcMod(x);
}

void Softcut::setPostFilterFc(int voice, float x) {
    scv[voice].setPostFilterFc(x);
}

void Softcut::setPostFilterRq(int voice, float x) {
    scv[voice].setPostFilterRq(x);
}

void Softcut::setPostFilterLp(int voice, float x) {
    scv[voice].setPostFilterLp(x);
}

void Softcut::setPostFilterHp(int voice, float x) {
    scv[voice].setPostFilterHp(x);
}

void Softcut::setPostFilterBp(int voice, float x) {
    scv[voice].setPostFilterBp(x);
}

void Softcut::setPostFilterBr(int voice, float x) {
    scv[voice].setPostFilterBr(x);
}

void Softcut::setPostFilterDry(int voice, float x) {
    scv[voice].setPostFilterDry(x);
}

#if 0 // not allowing realtime manipulation of fade logic params
void Softcut::setPreFadeWindow(float x) {
    auto t = std::thread([x] {
	    FadeCurves::setPreWindowRatio(x);
	});
    t.detach();
}

void Softcut::setRecFadeDelay(float x) {
    auto t = std::thread([x] {
	    FadeCurves::setRecDelayRatio(x);
	});
    t.detach();
}

void Softcut::setPreFadeShape(float x) {
    auto t = std::thread([x] {
	    FadeCurves::setPreShape(static_cast<FadeCurves::Shape>(x));
	});
    t.detach();
}

void Softcut::setRecFadeShape(float x) {
    auto t = std::thread([x] {
	    FadeCurves::setRecShape(static_cast<FadeCurves::Shape>(x));
	});
    t.detach();
}
#endif

void Softcut::setRecOffset(int i, float d) {
    scv[i].setRecOffset(d);
}

void Softcut::setLevelSlewTime(int i, float d) {
    scv[i].setLevelSlewTime(d);
}

void Softcut::setRecPreSlewTime(int i, float d) {
    scv[i].setRecPreSlewTime(d);
}

void Softcut::setRateSlewTime(int i, float d) {
    scv[i].setRateSlewTime(d);
}

phase_t Softcut::getQuantPhase(int i) {
    return scv[i].getQuantPhase();
}

void Softcut::setPhaseQuant(int i, phase_t q) {
    scv[i].setPhaseQuant(q);
}

void Softcut::setPhaseOffset(int i, float sec) {
    scv[i].setPhaseOffset(sec);
}

bool Softcut::getRecFlag(int i) {
    return scv[i].getRecFlag();
}

bool Softcut::getPlayFlag(int i) {
    return scv[i].getPlayFlag();
}

void Softcut::syncVoice(int follow, int lead, float offset) {
    scv[follow].cutToPos(scv[lead].getPos() + offset);
}

void Softcut::setVoiceBuffer(int id, float *buf, size_t bufFrames) {
    scv[id].setBuffer(buf, bufFrames);
}
