#pragma once

#include "inexor/server/server_projectilestate.hpp"
#include "inexor/server/server_fpsstate.hpp"

namespace inexor {
namespace server {

    /// 
    struct gamestate : fpsstate
    {
        vec o;
        int state, editstate;
        int lastdeath, deadflush, lastspawn, lifesequence;
        int lastshot;
        projectilestate<8> rockets, grenades, bombs;
        int frags, flags, deaths, teamkills, 
            shotdamage, //all damage your shots could have made
            damage, tokens;
        int lasttimeplayed, timeplayed;
        float effectiveness;

        /// 
        gamestate();

        /// 
        bool isalive(int gamemillis);

        /// 
        bool waitexpired(int gamemillis);

        /// 
        void reset();

        /// 
        void respawn();

        /// 
        void reassign();

        /// 
        void setbackupweapon(int weapon);

    };

};
};
