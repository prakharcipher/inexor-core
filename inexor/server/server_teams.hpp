#pragma once

#include "inexor/macros/constants.hpp"
#include "inexor/deprecated/hashset_template.hpp"
#include "inexor/macros/loop_macros.hpp"
#include "inexor/classes/clientinfo.hpp"
#include "inexor/classes/servmode.hpp"

#include <enet/enet.h>


namespace inexor {
namespace server {

    extern int lastmillis;
    extern vector<clientinfo *> connects, clients, bots;
    extern ENetPacket *sendf(int cn, int chan, const char *format, ...);

    /// scoreboard team block description
    struct teaminfo
    {
        char team[MAXTEAMLEN+1];
        int frags;
    };

    /// create hash for hashsts
    static inline uint hthash(const teaminfo &t) 
    { 
        return hthash(t.team); 
    }

    /// compare two team names
    static inline bool htcmp(const char *team, const teaminfo &t)
    {
        return !strcmp(team, t.team);
    }


    static hashset<teaminfo> teaminfos;


    void clearteaminfo();


    bool teamhasplayers(const char *team);


    bool pruneteaminfo();


    teaminfo *addteaminfo(const char *team);


    clientinfo *choosebestclient(float &bestrank);


    void autoteam();


    /// 
    struct teamrank
    {
        const char *name;
        float rank;
        int clients;

        /// 
        teamrank(const char *name) : name(name),
                                     rank(0),
                                     clients(0)
        {
        }
    };


    const char *chooseworstteam(const char *suggest = NULL, clientinfo *exclude = NULL);

};
};
