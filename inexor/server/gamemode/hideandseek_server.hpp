#pragma once
#include "inexor/fpsgame/game.hpp"
#include "inexor/gamemode/hideandseek_common.hpp"
#include "inexor/server/gamemode/gamemode_server.hpp"

#define ishider(ci) (strcmp(ci->team, TEAM_HIDE) == 0 && ci->state.state != CS_SPECTATOR ? true : false)
#define isseeker(ci) (strcmp(ci->team, TEAM_SEEK) == 0 && ci->state.state != CS_SPECTATOR ? true : false)

namespace server {

struct hideandseekservermode : servmode, hideandseekmode
{
    struct seekersinfo
    {
        int cn;
        bool freezed;
        seekersinfo(int cn_, bool freezed_) : cn(cn_), freezed(freezed_) {}
    };

    vector<seekersinfo*> seekersinfos;
    int lastupdatecheck;

    void setup();

    void initplayers();

    void initplayer(clientinfo *ci)
    {
        sethider(ci);
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        setseeker(ci);
    }

    void connected(clientinfo *ci)
    {
        setseeker(ci);
    }

    void leavegame(clientinfo *ci, bool disconnecting);

    void cleanup()
    {
        seekersinfos.deletecontents();
    }

    bool checkfinished();

    void update();

    void intermission()
    {
    }

    void spawned(fpsent *d)
    {
    }

    bool canspawn(clientinfo *ci, bool connecting)
    {
        return true;
    }

    bool canhit(clientinfo *target, clientinfo *actor);

    void died(clientinfo *target, clientinfo *actor);

    bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        return false;
    }

    void setseeker(clientinfo *ci);

    void sethider(clientinfo *ci);

    vector<clientinfo*> getactiveplayers();

    int getremaininghiders();

    bool isfreezed(clientinfo *ci);

    void setfreezedstate(clientinfo *ci, bool state);

    int getnonfreezedseekers();

    void announceseekers(char* msg);

    bool parse_network_message(int type, clientinfo *ci, clientinfo *cq, packetbuf &p) override
    {
        switch(type)
        {
            /*
            case N_RACEFINISH:
            {
            break;
            }
            */
            default: break;
        }
        return false;
    }
};




} // ns server
