//
// Created by ezra on 11/3/18.
//


#include <iostream>

#include "Commands.h"
#include "SoftcutClient.h"

using namespace softcut_jack_osc;

Commands Commands::softcutCommands;

Commands::Commands() = default;

void Commands::post(Commands::Id id, float f) {
    CommandPacket p(id, -1, f);
    q.enqueue(p);
}

void Commands::post(Commands::Id id, int i, float f) {
    CommandPacket p(id, i, f);
    q.enqueue(p);
}

void Commands::post(Commands::Id id, int i, int j) {
    CommandPacket p(id, i, j);
    q.enqueue(p);
}

void Commands::post(Commands::Id id, int i, int j, float f) {
    CommandPacket p(id, i, j, f);
    q.enqueue(p);
}


void Commands::handlePending(SoftcutClient *client) {
    CommandPacket p;
    while (q.try_dequeue(p)) {
        client->handleCommand(&p);
    }
}