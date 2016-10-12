#include "inexor/server/server_entities.hpp"

namespace inexor {
namespace server {


    bool canspawnitem(int type)
    {
        if(m_bomb) return (type>=I_BOMBS && type<=I_BOMBDELAY);
        else return !m_noitems && (type>=I_SHELLS && type<=I_QUAD && (!m_noammo || type<I_SHELLS || type>I_CARTRIDGES));
    }


    int spawntime(int type)
    {
        if(m_classicsp) return INT_MAX;
        int np = numclients(-1, true, false);
        np = np<3 ? 4 : (np>4 ? 2 : 3);         // spawn times are dependent on number of players
        int sec = 0;
        switch(type)
        {
            case I_SHELLS:
            case I_BULLETS:
            case I_ROCKETS:
            case I_ROUNDS:
            case I_GRENADES:
            case I_CARTRIDGES: sec = np*4; break;
            case I_BOMBS:
            case I_BOMBRADIUS: sec = np*9; break;
            case I_BOMBDELAY: sec = np*7; break;
            case I_HEALTH: sec = np*5; break;
            case I_GREENARMOUR: sec = 20; break;
            case I_YELLOWARMOUR: sec = 30; break;
            case I_BOOST: sec = 60; break;
            case I_QUAD: sec = 70; break;
        }
        return sec*1000;
    }


    bool delayspawn(int type)
    {
        switch(type)
        {
            case I_GREENARMOUR:
            case I_YELLOWARMOUR:
                return !m_classicsp;
            case I_BOOST:
            case I_QUAD:
                return true;
            default:
                return false;
        }
    }


    bool pickup(int i, int sender)
    {
        if((m_timed && gamemillis>=gamelimit) || !sents.inrange(i) || !sents[i].spawned) return false;
        clientinfo *ci = getinfo(sender);
        if(!ci || (!ci->local && !ci->state.canpickup(sents[i].type))) return false;
        sents[i].spawned = false;
        sents[i].spawntime = spawntime(sents[i].type);
        sendf(-1, 1, "ri3", N_ITEMACC, i, sender);
        ci->state.pickup(sents[i].type);
        return true;
    }

};
};
