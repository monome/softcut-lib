//
// Created by emb on 3/17/20.
//
#include <iostream>
#include <math.h>

static float raisedCosWin(float unitphase) {
    return 0.5f * (cosf(M_PI * (1.f + unitphase)) + 1.f);
};

static float calcPreFadeCurve(float fade) {
    static constexpr float del = 1.f / 128.f;
    static constexpr float ndel = 1.f - del;
    if (fade < del) {  return 0.f; }
    else { return raisedCosWin((fade-del)/ndel); }
}

static float calcRecFadeCurve(float fade) {
    static constexpr float del = 1.f / 8.f;
    static constexpr float ndel = 1.f - del;
    if (fade>del) { return 0.f; }
    else { return 1.f - raisedCosWin(fade/del); }
}

static void testfunc(float (*fn)(float)) {
    using namespace std;
    float f = 0;
    float inc = 0.001;
    cout << " [ ";
    while (f < 1.f) {
        cout << (*fn)(f) << ", ";
        f += inc;
    }
    cout << " ] " << endl;
}

int main() {
    testfunc(&calcPreFadeCurve);
    testfunc(&calcRecFadeCurve);
}