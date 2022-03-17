//
// Created by ezra on 11/8/18.
//

#include <math.h>
#include "softcut/Svf.h"

Svf::Svf() = default;

float Svf::getNextSample(float x) {
    update(x);
    return lp * lpMix + hp * hpMix + bp * bpMix + br * brMix;
}

void Svf::setSampleRate(float sr) {
    setSr(sr);
}

void Svf::setFc(float fc) {
    setFc(fc);
}

void Svf::setRq(float rq) {
    setRq(rq);
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

/////////////////
// C implementation

void Svf::calcCoeffs() {
    //g = static_cast<float>(tan(M_PI * fc / sr));
    g = warpApprox(fc/sr);
    g1 = g / (1.f + g * (g + rq));
    g2 = 2.f * (g + rq) * g1;
    g3 = g * g1;
    g4 = 2.f * g1;  
}

void Svf::init() {
    clearState();
}

void Svf::clearState() {
    v0z = 0;
    v1 = 0;
    v2 = 0;
}

void Svf::setSr(float sr) {
    sr = sr;
    calcCoeffs();
}

void Svf::setFc(float fc) {
    fc = (fc > sr / 2) ? sr / 2 : fc;
    calcCoeffs();
}

void Svf::setRq(float rq) {
    rq = rq;
    calcCoeffs();
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

// 5th-degree polynomial approximation of tan(pi*theta),
// for normalized freq in ~[1/4800, 1/3]
float Svf::warpApprox(float x) {
    static const float a0 = -0.0001414077269146219;
    static const float a1 = 3.1951048374176025;
    static const float a2 = -2.1814463138580322;
    static const float a3 = 38.69891357421875;
    static const float a4 = -146.58091735839844;
    static const float a5 = 311.98516845703125;
    return a0 + x*(a1 + x*(a2 + x*(a3 + x*(a4 + x*(a5)))));
}
    