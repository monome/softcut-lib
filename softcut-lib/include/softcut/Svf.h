//
// Created by ezra on 11/8/18.
//
// state variable filter
// after Hal Chamberlin, Andy Simper

#ifndef Softcut_SVF_H
#define Softcut_SVF_H

#include <memory>

class Svf {
public:
    Svf();
    float getNextSample(float x);
    void setSampleRate(float sr);
    void setFc(float fc);
    void setRq(float rq);
    void setLpMix(float mix);
    void setHpMix(float mix);
    void setBpMix(float mix);
    void setBrMix(float mix);
    void reset();
    void update(float x);
    void calcWarp();
    void calcCoeffs();
    void clearState();
    
    float getFc();

private:
    static const float MAX_NORM_FC;
    float lpMix;
    float hpMix;
    float bpMix;
    float brMix;
    float minFc;
    float maxFc;
    float pi_sr;

    // sample rate
    float sr;
    // corner frequency in hz
    float fc;
    // reciprocal of Q in [0,1]
    float rq;
    // intermediate coefficients
    float g;
    float g1;
    float g2;
    float g3;
    float g4;
    // state variables
    float v0;
    float v1;
    float v2;
    float v0z;
    float v1z;
    float v2z;
    float v3;
    // outputs
    float lp; // lowpass
    float hp; // highpass
    float bp; // bandpass
    float br; // bandreject

};


#endif //Softcut_SVF_H
