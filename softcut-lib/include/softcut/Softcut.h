//
// Created by emb on 11/10/18.
//

#ifndef Softcut_Softcut_H
#define Softcut_Softcut_H

#include <memory>
#include <thread>
#include "Types.h"
#include "Voice.h"

namespace softcut {
    template<int numVoices>
    class Softcut {
    protected:

        friend class TestBuffers;

    private:
        Voice scv[numVoices];
    public:

        Softcut() {
            this->reset();
        }

        void reset() {
            for (int v = 0; v < numVoices; ++v) {
                scv[v].reset();
                /// test: set each voice to duck the next one, in a loop
                scv[v].setDuckTarget( &(scv[(v+1)%numVoices]) );
                // test: set voices to duck each other in pairs
                //// (FIXME: probably won't work right because each voice processes whole block in turn...)
//                scv[0].setDuckTarget(&(scv[1]));
//                scv[1].setDuckTarget(&(scv[0]));
            };

        }

        // assumption: v is in range
        void processBlock(int v, float *in, float *out, int numFrames) {
            scv[v].processBlockMono(in, out, numFrames);
        }

        void setSampleRate(unsigned int hz) {
            for (auto &v : scv) {
                v.setSampleRate(hz);
            }
        }

        Voice *voice(int i) {
            if (i >= 0 && i < numVoices) {
                return &(scv[i]);
            } else {
                return nullptr;
            }
        }


        void syncVoice(int follower, int leader, float offset) {
            // TODO
        }
    };
}


#endif //Softcut_Softcut_H2
