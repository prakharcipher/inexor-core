#include "inexor/server/server_gameplay.hpp"

namespace inexor {
namespace server {

    /// 
    int gamemode = 0;
    /// true when map has changed and waiting for clients to send item
    bool notgotitems = true;
    /// 
    int gamemillis = 0;
    /// 
    int gamelimit = 0;
    /// 
    int nextexceeded = 0;
    /// 
    int gamespeed = 100;
    /// 
    bool gamepaused = false;
    /// 
    bool teamspersisted = false;
    /// 
    bool shouldstep = true;

    /// 
    void suicide(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        if(gs.state!=CS_ALIVE) return;
        int fragvalue = smode ? smode->fragvalue(ci, ci) : -1;
        ci->state.frags += fragvalue;
        ci->state.deaths++;
        teaminfo *t = m_teammode ? teaminfos.access(ci->team) : NULL;
        if(t) t->frags += fragvalue;
        sendf(-1, 1, "ri5", N_DIED, ci->clientnum, ci->clientnum, gs.frags, t ? t->frags : 0);
        ci->position.setsize(0);
        if(smode) smode->died(ci, NULL);
        gs.state = CS_DEAD;
        gs.lastdeath = gamemillis;
        gs.respawn();

        if (m_lms) checklms();
    }


};
};
