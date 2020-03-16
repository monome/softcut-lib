//
// Created by emb on 3/15/20.
//

#ifndef SOFTCUT_DEBUGLOG_H
#define SOFTCUT_DEBUGLOG_H

#include <cstddef>
#include <sstream>
#include <iostream>


#define SOFTCUT_DBG(offset, msg) \
    softcut::DebugLog::newLine(offset); \
    //std::cerr << (msg);

namespace softcut {
    class DebugLog {
        static size_t frameOffset;
        //std::stringstream ss;
        //static std::ostream *os;


    public:
        static void init() {
//            os = &(std::cerr);
            frameOffset = 0;
        }

        static void setFrameOffset(size_t fr) {
            frameOffset = fr;
        }

        static void advanceFrames(size_t fr) {
            frameOffset += fr;
        }

        // argument is an additional frame offset
        static void newLine(size_t offset = 0) {
            std::cout << std::endl;
            std::cout << "[" << (frameOffset + offset) << "] ";
        }

//        static std::ostream *stream() { return os; }

    };
}


#endif //SOFTCUT_DEBUGLOG_H
