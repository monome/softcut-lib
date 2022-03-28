//
// Created by ezra on 11/8/18.
//

#include <math.h>
#include "softcut/Svf.h"

const float Svf::MAX_NORM_FC = 0.4;

Svf::Svf() = default;

float Svf::getNextSample(float x) {
    update(x);
    return lp * lpMix + hp * hpMix + bp * bpMix + br * brMix;
}

void Svf::setSampleRate(float aSr) {
    sr = aSr;
    pi_sr = M_PI / sr;
    minFc = 10.f;
    maxFc = sr * MAX_NORM_FC;
    calcWarp();
    calcCoeffs();
}

void Svf::setFc(float aFc) {
    fc = (aFc > maxFc) ? maxFc : aFc;
    fc = (fc < minFc) ? minFc : fc;
    calcWarp();
    calcCoeffs();
}

void Svf::setRq(float aRq) {
    rq = aRq;
    calcCoeffs();
}

void Svf::setLpMix(float mix) {
    lpMix = mix;
}

void Svf::setHpMix(float mix) {
    hpMix = mix;
}

void Svf::setBpMix(float mix) {
    bpMix = mix;
}

void Svf::setBrMix(float mix) {
    brMix = mix;
}

void Svf::reset() { 
    clearState();
}

void Svf::calcWarp() { 
    // NB: wasn't actually able to beat `tan` for performance+accuracy sweet spot
    // on raspi with aggressive optimizations
    g = static_cast<float>(tan(fc * pi_sr));
}

void Svf::calcCoeffs() {
    g1 = g / (1.f + g * (g + rq));
    g2 = 2.f * (g + rq) * g1;
    g3 = g * g1;
    g4 = 2.f * g1;
}

void Svf::clearState() {
    v0z = 0;
    v1 = 0;
    v2 = 0;
}


void Svf::update(float in) {
    // update
    v0 = in;
    v1z = v1;
    v2z = v2;
    v3 = v0 + v0z - 2.f * v2z;
    v1 += g1 * v3 - g2 * v1z;
    v2 += g3 * v3 + g4 * v1z;
    v0z = v0;
    // output
    lp = v2;
    bp = v1;
    hp = v0 - rq * v1 - v2;
    br = v0 - rq * v1;
}

float Svf::getFc() {
    return fc;
}