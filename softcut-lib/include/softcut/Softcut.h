//
// Created by emb on 11/10/18.
//

#ifndef Softcut_Softcut_H
#define Softcut_Softcut_H

#include <array>
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
        std::array<Voice, numVoices> voices;
        // enabled flags
        std::array<bool, numVoices> voiceEnabled;
        // input and output busses are assigned and persisted
        std::array<float*, numVoices> input;
        std::array<float*, numVoices> output;

    public:

        Softcut() {
            this->reset();
        }

        void reset() {
            for (int i=0; i<numVoices; ++i) {

                voices[i].reset();
                /// test: set each voice to duck the next one, in a loop
                voices[i].setDuckTarget(&(voices[(i + 1) % numVoices]) );
            };

        }

        void processBlock(int numFrames) {
            for (int i=0; i<numVoices; ++i) {
                voices[i].processBlockMono(input[i], output[i], numFrames);
            }
        }

        void setSampleRate(unsigned int hz) {
            for (auto &v : voices) {
                v.setSampleRate(hz);
            }
        }

        Voice *voice(int i) {
            if (i >= 0 && i < numVoices) {
                return &(voices[i]);
            } else {
                return nullptr;
            }
        }


        void syncVoice(int follower, int leader, float offset) {
            // TODO
        }

        void setInputBus(float* src, int voiceIndex) {
            input[voiceIndex] = src;
        }
        void setOutputBus(float* dst, int voiceIndex) {
            output[voiceIndex] = dst;
        }
        void setVoiceEnabled(int voiceIndex, bool val) {
            voiceEnabled[voiceIndex] = val;
        }
    };
}


#endif //Softcut_Softcut_H2
