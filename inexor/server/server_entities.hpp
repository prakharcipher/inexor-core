#pragma once

#include "inexor/deprecated/vector_template.hpp"
#include "inexor/classes/clientinfo.hpp"
#include "inexor/server/server_gameplay.hpp"
#include "inexor/macros/gamemode_macros.hpp"
#include "inexor/classes/itemstats.hpp"
#include <enet/enet.h>


namespace inexor {
namespace server {

    /// server side version of "entity" type
    struct server_entity
    {
        int type;
        int spawntime;
        bool spawned;
    };

    vector<server_entity> sents;
   
    extern int gamemillis, gamelimit;
    extern ENetPacket *sendf(int cn, int chan, const char *format, ...);

    /// 
    bool canspawnitem(int type);

    /// 
    int spawntime(int type);

    /// 
    bool delayspawn(int type);

    /// server side item pickup, acknowledge first client that gets it
    bool pickup(int i, int sender);



};
};
