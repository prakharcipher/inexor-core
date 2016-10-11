#pragma once

#include "inexor/server/server_gamestate.hpp"

namespace inexor {
namespace server {

    gamestate::gamestate() : state(CS_DEAD),
                             editstate(CS_DEAD),
                             lifesequence(0)
    {
    }

    bool gamestate::isalive(int gamemillis)
    {
        return state==CS_ALIVE || (state==CS_DEAD && gamemillis - lastdeath <= DEATHMILLIS);
    }

    bool gamestate::waitexpired(int gamemillis)
    {
        return gamemillis - lastshot >= gunwait;
    }

    void gamestate::reset()
    {
        if(state!=CS_SPECTATOR) state = editstate = CS_DEAD;
        maxhealth = 100;
        rockets.reset();
        grenades.reset();
        bombs.reset();
        timeplayed = 0;
        effectiveness = 0;
        frags = flags = deaths = teamkills = shotdamage = damage = tokens = 0;
        lastdeath = 0;
        respawn();
    }

    void gamestate::respawn()
    {
        fpsstate::respawn();
        o = vec(-1e10f, -1e10f, -1e10f);
        deadflush = 0;
        lastspawn = -1;
        lastshot = 0;
        tokens = 0;
    }

    void gamestate::reassign()
    {
        respawn();
        rockets.reset();
        grenades.reset();
        bombs.reset();
    }

    void gamestate::setbackupweapon(int weapon)
    {
        fpsstate::backupweapon = weapon;
    }


};
};
