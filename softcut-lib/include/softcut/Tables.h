//
// Created by emb on 6/7/20.
//
// structure containing table data needed for fast computation of some dspkit components (filters.)
//
// this stuff is samplerate-dependent, so cannot be made static,
// but one copy can be shared by all softcut voices.
// hence, it is implemented as a singleton.

#ifndef CRONE_TABLES_H
#define CRONE_TABLES_H

#include <array>

#include "dsp-kit/Conversion.hpp"
#include "dsp-kit/LadderLpf.hpp"
#include "dsp-kit/Svf.hpp"

namespace softcut {
    class Tables {

        //--- singleton lifecycle
    public:
        static Tables& shared() {
            static Tables instance;
            return instance;
        }
        Tables() = default;
        Tables(Tables const&) = delete;
        void operator=(Tables const&) = delete;

        //--- data fields
    public:
        static constexpr int svfGTableSize = 1024;
        static constexpr int ladderLpfGTableSize = 1024;
    private:
        std::array<float, svfGTableSize> svfGTable{};
        std::array<float, ladderLpfGTableSize> ladderLpfGTable{};
    public:
        //-- accessors
        const float* getSvfGTable() { return svfGTable.data(); }
        const float* getLadderLpfGTable() { return ladderLpfGTable.data(); }
        void setSampleRate(float sr) {
            dspkit::Svf::fillGTable(svfGTable.data(), svfGTableSize, sr);
            dspkit::LadderLpf<float>::fillGTable(ladderLpfGTable.data(), ladderLpfGTableSize,
                    sr, 8, 11);
        }
    };
}


#endif //CRONE_TABLES_H
