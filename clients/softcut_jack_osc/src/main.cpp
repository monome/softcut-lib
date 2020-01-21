//
// Created by emb on 1/20/20.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

#include "SoftCutClient.h"
#include "OscInterface.h"
#include "BufDiskWorker.h"

static inline void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
    using namespace softcut_jack_osc;
    using std::cout;
    using std::endl;

    std::unique_ptr<SoftCutClient> sc;
    sc = std::make_unique<SoftCutClient>();

    sc->setup();
    sc->start();

    sc->connectAdcPorts();
    sc->connectDacPorts();

    OscInterface::init();

    cout << "entering main loop..." << endl;
    while(!OscInterface::shouldQuit())  {
        sleep(100);
    }
    sc->stop();
    sc->cleanup();
    return 0;
}