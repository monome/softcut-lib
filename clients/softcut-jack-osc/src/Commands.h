//
// Created by ezra on 11/3/18.
//

#ifndef CRONE_COMMANDS_H
#define CRONE_COMMANDS_H

#include <boost/lockfree/spsc_queue.hpp>


namespace softcut_jack_osc {

    class SoftcutClient;

    class Commands {
    public:
        typedef enum {
            //-- softcut commands

            // mix
            SET_ENABLED_CUT,
            SET_CUT_VOICE_LEVEL,
            SET_CUT_VOICE_PAN,
            // level of individual input channel -> cut voice
            SET_LEVEL_IN_CUT,
            SET_LEVEL_CUT_CUT,

            // voice parameters
            SET_CUT_VOICE_BUFFER,
            SET_CUT_VOICE_REC_ENABLED,
            SET_CUT_VOICE_PLAY_ENABLED,

            SET_CUT_VOICE_RATE,
            SET_CUT_VOICE_LOOP_START,
            SET_CUT_VOICE_LOOP_END,
            SET_CUT_VOICE_LOOP_ENABLED,
            SET_CUT_VOICE_POSITION,

            SET_CUT_VOICE_FADE_TIME,
            SET_CUT_VOICE_REC_LEVEL,
            SET_CUT_VOICE_PRE_LEVEL,
            SET_CUT_VOICE_REC_OFFSET,

            SET_CUT_VOICE_PRE_FILTER_FC,
            SET_CUT_VOICE_PRE_FILTER_FC_MOD,
            SET_CUT_VOICE_PRE_FILTER_Q,
            SET_CUT_VOICE_PRE_FILTER_ENABLED,

	        SET_CUT_VOICE_POST_FILTER_FC,
	        SET_CUT_VOICE_POST_FILTER_RQ,
            SET_CUT_VOICE_POST_FILTER_LP,
            SET_CUT_VOICE_POST_FILTER_HP,
            SET_CUT_VOICE_POST_FILTER_BP,
            SET_CUT_VOICE_POST_FILTER_BR,
            SET_CUT_VOICE_POST_FILTER_DRY,

            SET_CUT_VOICE_LEVEL_SLEW_TIME,
            SET_CUT_VOICE_PAN_SLEW_TIME,
            SET_CUT_VOICE_RECPRE_SLEW_TIME,
            SET_CUT_VOICE_RATE_SLEW_TIME,

            SET_CUT_VOICE_SYNC,
            SET_CUT_VOICE_DUCK_TARGET,
            SET_CUT_VOICE_FOLLOW_TARGET,

            NUM_COMMANDS,
        } Id;

    public:
        Commands();
        void post(Commands::Id id, float f);
        void post(Commands::Id id, int i, float f);
        void post(Commands::Id id, int i, int j);
        void post(Commands::Id id, int i, int j, float f);

        void handlePending(SoftcutClient *client);

        struct CommandPacket {
            CommandPacket() = default;
            CommandPacket(Commands::Id i, int i0,  float f) : id(i), idx_0(i0), idx_1(-1), value(f) {}
            CommandPacket(Commands::Id i, int i0, int i1) : id(i), idx_0(i0), idx_1(i1) {}
            CommandPacket(Commands::Id i, int i0, int i1, float f) : id(i), idx_0(i0), idx_1(i1), value(f) {}
            Id id;
            int idx_0{};
            int idx_1{};
            float value{};
        };

        static Commands softcutCommands;

    private:
        boost::lockfree::spsc_queue <CommandPacket,
                boost::lockfree::capacity<200> > q;
    };

}

#endif //CRONE_COMMANDS_H
