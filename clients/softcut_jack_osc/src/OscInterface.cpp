

//
// Created by ezra on 11/4/18.
//

#include <utility>
#include <thread>
//#include <boost/format.hpp>

#include "softcut/FadeCurves.h"

#include "BufDiskWorker.h"
#include "Commands.h"
#include "OscInterface.h"

using namespace softcut_jack_osc;
using softcut::FadeCurves;

bool OscInterface::quitFlag;

std::string OscInterface::port;
lo_server_thread OscInterface::st;
lo_address OscInterface::clientAddress;

std::array<OscInterface::OscMethod, OscInterface::MaxNumMethods> OscInterface::methods;
unsigned int OscInterface::numMethods = 0;

std::unique_ptr<Poll> OscInterface::vuPoll;
std::unique_ptr<Poll> OscInterface::phasePoll;
SoftcutClient *OscInterface::softCutClient;

OscInterface::OscMethod::OscMethod(string p, string f, OscInterface::Handler h)
        : path(std::move(p)), format(std::move(f)), handler(h) {}


void OscInterface::init(SoftcutClient *sc) {
    quitFlag = false;
    // FIXME: should get port configs from program args or elsewhere
    port = "9999";
#if 1
    clientAddress = lo_address_new("127.0.0.1", "8888");
#else  // testing with SC
    clientAddress = lo_address_new("127.0.0.1", "57120");
#endif

    st = lo_server_thread_new(port.c_str(), handleLoError);
    addServerMethods();

    softCutClient = sc;

    //--- softcut phase poll
    phasePoll = std::make_unique<Poll>("softcut/phase");
    phasePoll->setCallback([](const char *path) {
        for (int i = 0; i < softCutClient->getNumVoices(); ++i) {
            if (softCutClient->checkVoiceQuantPhase(i)) {
                lo_send(clientAddress, path, "if", i, softCutClient->getQuantPhase(i));
            }
        }
    });
    phasePoll->setPeriod(1);


    //--- TODO: softcut trigger poll?

    lo_server_thread_start(st);
}


void OscInterface::addServerMethod(const char *path, const char *format, Handler handler) {
    OscMethod m(path, format, handler);
    methods[numMethods] = m;
    lo_server_thread_add_method(st, path, format,
                                [](const char *path,
                                   const char *types,
                                   lo_arg **argv,
                                   int argc,
                                   lo_message msg,
                                   void *data)
                                        -> int {
                                    (void) path;
                                    (void) types;
                                    (void) msg;
                                    auto pm = static_cast<OscMethod *>(data);
                                    //std::cerr << "osc rx: " << path << std::endl;
                                    pm->handler(argv, argc);
                                    return 0;
                                }, &(methods[numMethods]));
    numMethods++;
}


void OscInterface::addServerMethods() {
    addServerMethod("/hello", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;
        std::cout << "hello" << std::endl;
    });

    addServerMethod("/goodbye", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;
        std::cout << "goodbye" << std::endl;
        OscInterface::quitFlag = true;
    });

    addServerMethod("/quit", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;
        OscInterface::quitFlag = true;
    });


    //---------------------------
    //--- mixer polls

    addServerMethod("/poll/start/vu", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;
        vuPoll->start();
    });

    addServerMethod("/poll/stop/vu", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;
        vuPoll->stop();
    });



    //--------------------------------
    //-- softcut routing

    addServerMethod("/set/enabled/cut", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_ENABLED_CUT, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/level/cut", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_LEVEL_CUT, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/pan/cut", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_PAN_CUT, argv[0]->i, argv[1]->f);
    });



    //--- NB: these are handled by the softcut command queue,
    // because their corresponding mix points are processed by the softcut client.

    // input channel -> voice levels
    addServerMethod("/set/level/in_cut", "iif", [](lo_arg **argv, int argc) {
        if (argc < 3) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_LEVEL_IN_CUT, argv[0]->i, argv[1]->i, argv[2]->f);
    });


    // voice ->  voice levels
    addServerMethod("/set/level/cut_cut", "iif", [](lo_arg **argv, int argc) {
        if (argc < 3) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_LEVEL_CUT_CUT, argv[0]->i, argv[1]->i, argv[2]->f);
    });


    //--------------------------------
    //-- softcut params


    addServerMethod("/set/param/cut/rate", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_RATE, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/loop_start", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_LOOP_START, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/loop_end", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_LOOP_END, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/loop_flag", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_LOOP_FLAG, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/fade_time", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_FADE_TIME, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/rec_level", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_REC_LEVEL, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_level", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_LEVEL, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/rec_flag", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_REC_FLAG, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/play_flag", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PLAY_FLAG, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/rec_offset", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_REC_OFFSET, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/position", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POSITION, argv[0]->i, argv[1]->f);
    });

    // --- input filter
    addServerMethod("/set/param/cut/pre_filter_fc", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_FC, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_filter_fc_mod", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_FC_MOD, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_filter_rq", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_RQ, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_filter_lp", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_LP, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_filter_hp", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_HP, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_filter_bp", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_BP, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_filter_br", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_BR, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pre_filter_dry", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PRE_FILTER_DRY, argv[0]->i, argv[1]->f);
    });


    // --- output filter
    addServerMethod("/set/param/cut/post_filter_fc", "if", [
    ](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POST_FILTER_FC, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/post_filter_rq", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POST_FILTER_RQ, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/post_filter_lp", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POST_FILTER_LP, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/post_filter_hp", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POST_FILTER_HP, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/post_filter_bp", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POST_FILTER_BP, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/post_filter_br", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POST_FILTER_BR, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/post_filter_dry", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_POST_FILTER_DRY, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/voice_sync", "iif", [](lo_arg **argv, int argc) {
        if (argc < 3) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_VOICE_SYNC, argv[0]->i, argv[1]->i, argv[2]->f);
    });

    addServerMethod("/set/param/cut/level_slew_time", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_LEVEL_SLEW_TIME, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/pan_slew_time", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_PAN_SLEW_TIME, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/recpre_slew_time", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_RECPRE_SLEW_TIME, argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/rate_slew_time", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_RATE_SLEW_TIME, argv[0]->i, argv[1]->f);
    });


    addServerMethod("/set/param/cut/buffer", "ii", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        Commands::softcutCommands.post(Commands::Id::SET_CUT_BUFFER, argv[0]->i, argv[1]->i);
    });


    //-------------------------------
    //--- softcut buffer manipulation


    // FIXME: hrm, our system doesn't allow variable argument count. maybe need to make multiple methods
    addServerMethod("/softcut/buffer/read_mono", "sfffii", [](lo_arg **argv, int argc) {
        float startSrc = 0.f;
        float startDst = 0.f;
        float dur = -1.f;
        int chanSrc = 0;
        int chanDst = 0;
        if (argc < 1) {
            std::cerr << "/softcut/buffer/read_mono requires at least one argument (file path)" << std::endl;
            return;
        }
        if (argc > 1) {
            startSrc = argv[1]->f;
        }
        if (argc > 2) {
            startDst = argv[2]->f;
        }
        if (argc > 3) {
            dur = argv[3]->f;
        }
        if (argc > 4) {
            chanSrc = argv[4]->i;
        }
        if (argc > 5) {
            chanDst = argv[5]->i;
        }
        const char *str = &argv[0]->s;
        softCutClient->readBufferMono(str, startSrc, startDst, dur, chanSrc, chanDst);

    });

    // FIXME: hrm, our system doesn't allow variable argument count. maybe need to make multiple methods
    addServerMethod("/softcut/buffer/read_stereo", "sfff", [](lo_arg **argv, int argc) {
        float startSrc = 0.f;
        float startDst = 0.f;
        float dur = -1.f;
        if (argc < 1) {
            std::cerr << "/softcut/buffer/read_stereo requires at least one argument (file path)" << std::endl;
            return;
        }
        if (argc > 1) {
            startSrc = argv[1]->f;
        }
        if (argc > 2) {
            startDst = argv[2]->f;
        }
        if (argc > 3) {
            dur = argv[3]->f;
        }
        const char *str = &argv[0]->s;
        softCutClient->readBufferStereo(str, startSrc, startDst, dur);
    });


    // FIXME: hrm, our system doesn't allow variable argument count. maybe need to make multiple methods
    addServerMethod("/softcut/buffer/write_mono", "sffi", [](lo_arg **argv, int argc) {
        float start = 0.f;
        float dur = -1.f;
        int chan = 0;
        if (argc < 1) {
            std::cerr << "/softcut/buffer/write_mono requires at least one argument (file path)" << std::endl;
            return;
        }
        if (argc > 1) {
            start = argv[1]->f;
        }
        if (argc > 2) {
            dur = argv[2]->f;
        }
        if (argc > 3) {
            chan = argv[3]->i;
        }
        const char *str = &argv[0]->s;
        softCutClient->writeBufferMono(str, start, dur, chan);
    });

    // FIXME: hrm, our system doesn't allow variable argument count. maybe need to make multiple methods
    addServerMethod("/softcut/buffer/write_stereo", "sff", [](lo_arg **argv, int argc) {
        float start = 0.f;
        float dur = -1.f;
        if (argc < 1) {
            std::cerr << "/softcut/buffer/write_stereo requires at least one argument (file path)" << std::endl;
            return;
        }
        if (argc > 1) {
            start = argv[1]->f;
        }
        if (argc > 2) {
            dur = argv[2]->f;
        }
        const char *str = &argv[0]->s;
        softCutClient->writeBufferStereo(str, start, dur);
    });


    addServerMethod("/softcut/buffer/clear", "", [](lo_arg **argv, int argc) {
        (void) argc;
        (void) argv;
        softCutClient->clearBuffer(0);
        softCutClient->clearBuffer(1);
    });


    addServerMethod("/softcut/buffer/clear_channel", "i", [](lo_arg **argv, int argc) {
        if (argc < 1) {
            return;
        }
        softCutClient->clearBuffer(argv[0]->i);
    });

    addServerMethod("/softcut/buffer/clear_region", "ff", [](lo_arg **argv, int argc) {
        if (argc < 2) {
            return;
        }
        softCutClient->clearBuffer(0, argv[0]->f, argv[1]->f);
        softCutClient->clearBuffer(1, argv[0]->f, argv[1]->f);
    });

    addServerMethod("/softcut/buffer/clear_region_channel", "iff", [](lo_arg **argv, int argc) {
        if (argc < 3) {
            return;
        }
        softCutClient->clearBuffer(argv[0]->i, argv[1]->f, argv[2]->f);
    });

    addServerMethod("/softcut/reset", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;

        softCutClient->clearBuffer(0, 0, -1);
        softCutClient->clearBuffer(1, 0, -1);

        softCutClient->reset();
        for (int i = 0; i < SoftcutClient::NumVoices; ++i) {
            phasePoll->stop();
        }
    });

    //---------------------
    //--- softcut polls

    addServerMethod("/set/param/cut/phase_quant", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        softCutClient->setPhaseQuant(argv[0]->i, argv[1]->f);
    });

    addServerMethod("/set/param/cut/phase_offset", "if", [](lo_arg **argv, int argc) {
        if (argc < 2) { return; }
        softCutClient->setPhaseOffset(argv[0]->i, argv[1]->f);
    });

    addServerMethod("/poll/start/cut/phase", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;
        phasePoll->start();
    });

    addServerMethod("/poll/stop/cut/phase", "", [](lo_arg **argv, int argc) {
        (void) argv;
        (void) argc;
        phasePoll->stop();
    });
}

void OscInterface::printServerMethods() {
    using std::cout;
    using std::endl;
    using std::string;
    //using boost::format;
    cout << "osc methods: " << endl;
    for (unsigned int i = 0; i < numMethods; ++i) {
        //cout << format(" %1% [%2%]") % methods[i].path % methods[i].format << endl;
        cout << methods[i].path << "\t" << methods[i].format << endl;
    }
}

void OscInterface::deinit() {
    lo_address_free(clientAddress);
}

