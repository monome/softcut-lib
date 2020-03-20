//
// Created by emb on 3/18/20.
//

#ifndef SOFTCUT_FADES_H
#define SOFTCUT_FADES_H

#include <math.h>

namespace softcut {

    class Fades {
        public:
        static float raisedCosFadeIn(float unitphase) {
            return 0.5f * (cosf(M_PI * (1.f + unitphase)) + 1.f);
        };

        static float raisedCosFadeOut(float unitphase) {
            return 0.5f * (cosf(M_PI * unitphase) + 1.f);
        };

        static float fastCosFadeOut(float unitphase) {
            return  cosf(M_PI_2 * (unitphase + 1)) + 1.f;
        }
    };
}

#endif //SOFTCUT_FADES_H
