//
// Created by emb on 11/10/18.
//

#ifndef Softcut_Softcut_H
#define Softcut_Softcut_H

#include <array>
#include <memory>
#include <thread>

#include "Tables.h"
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
                voiceEnabled[i] = false;
                voices[i].reset();
            };

        }

        void processBlock(int numFrames) {
            for (int i=0; i<numVoices; ++i) {
                if (voiceEnabled[i]) {
                    voices[i].updatePositions(numFrames);
                    voices[i].updateQuantPhase();
                }
            }
            for (int i=0; i<numVoices; ++i) {
                if (voiceEnabled[i]) {
                    voices[i].performReads(output[i], numFrames);
                }
            }
            for (int i=0; i<numVoices; ++i) {
                if (voiceEnabled[i]) {
                    voices[i].performWrites(input[i], numFrames);
                }
            }
        }

        void setSampleRate(unsigned int hz) {
            for (auto &v : voices) {
                v.setSampleRate(hz);
            }
            Tables::shared().setSampleRate(hz);
        }

        Voice *voice(int i) {
            if (i >= 0 && i < numVoices) {
                return &(voices[i]);
            } else {
                return nullptr;
            }
        }

        void syncVoice(int follower, int leader, float offset) {
            voices[follower].syncPosition(voices[leader], offset);
        }

        void setInputBus( int vIdx, float* src) {
            input[vIdx] = src;
        }
        void setOutputBus(int vIdx, float* dst) {
            output[vIdx] = dst;
        }
        void setVoiceEnabled(int vIdx, bool val) {
            voiceEnabled[vIdx] = val;
        }
        bool getVoiceEnabled(int vIdx) {
            return voiceEnabled[vIdx];
        }

    };
}


#endif //Softcut_Softcut_H