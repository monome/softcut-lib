//
// Created by ezra on 11/8/18.
//

#include <iostream>
#include <math.h>


#include "softcut/Svf.h"

Svf::Svf() = default;

void Svf::init() {
    clearState();
}

void Svf::clearState() {
    v0z = 0;
    v1 = 0;
    v2 = 0;
}

void Svf::setSampleRate(float aSr) {
    sr = aSr;
    pi_sr = M_PI/sr;
    normFcMin = 10*pi_sr;
    normFcMax = 0.4;
    calcCoeffs();
}

void Svf::setFc(float aFc) {
    fc = aFc > normFcMax ? normFcMax : aFc;
    calcCoeffs();
}

void Svf::setRq(float aRq) {
    rq = aRq;
    calcCoeffs();
}

void Svf::calcCoeffs() {
    g = static_cast<float>(tan(fc * pi_sr));
    g1 = g / (1.f + g * (g + rq));
    g2 = 2.f * (g + rq) * g1;
    g3 = g * g1;
    g4 = 2.f * g1;  
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

float Svf::getNextSample(float x) {
    update(x);
    return (lp * lpMix) + (hp * hpMix) + (bp * bpMix) + (br * brMix);
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

float Svf::getFc() {
    return fc;
}
