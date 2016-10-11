#pragma once

#include "inexor/engine/engine.hpp"
#include "inexor/server/server_enums.hpp"
#include "inexor/server/server_fpsstate.hpp"
/// struct teaminfo
#include "inexor/server/server_teams.hpp"
#include "inexor/server/server_core.hpp"
/// admin levels
#include "inexor/enumerations/enum_admin_levels.hpp"
/// struct clientinfo
#include "inexor/server/server_clientinfo.hpp"
/// struct gamestate
#include "inexor/server/server_gamestate.hpp"

namespace inexor {
namespace server {

    /// 
    static const int DEATHMILLIS = 300;

    /// 
    extern int gamemode, gamemillis, gamelimit, nextexceeded, gamespeed;
    extern bool notgotitems, shouldstep, gamepaused, teamspersisted;
    extern hashset<teaminfo> teaminfos;

    /// this structure will be declared below
    struct clientinfo;

    /// TODO: not sure if this is correct!
    extern vector<clientinfo *> connects, clients, bots;

    /// 
    struct hitinfo
    {
        int target;
        int lifesequence;
        int rays;
        float dist;
        vec dir;
    };


    /// 
    void suicide(clientinfo *ci);

    /// 
    clientinfo *getinfo(int n);

    /// 
    void dodamage(clientinfo *target, clientinfo *actor, int damage, int gun, const vec &hitpush = vec(0, 0, 0));

};
};
