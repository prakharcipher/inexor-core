#pragma once

#include "inexor/shared/tools.hpp"
#include "inexor/shared/iengine.hpp"
#include "inexor/fpsgame/game.hpp"

#include "inexor/enumerations/netmsg_ids.hpp"
#include "inexor/classes/clientinfo.hpp"
#include "inexor/classes/demofile.hpp"
#include "inexor/classes/demoheader.hpp"
#include <enet/enet.h>

// no namespace for this one
extern int curtime;

namespace server {
    
    /// extern declarations
    extern int nextplayback;
    extern int gamemode;

    // TODO: why SharedVar?
    extern SharedVar<int> maxdemos;
    extern SharedVar<int> maxdemosize;

    extern void sendservmsgf(const char *fmt, ...);
    extern void sendservmsg(const char *s);
    extern int welcomepacket(packetbuf &p, clientinfo *ci);
    extern void sendwelcome(clientinfo *ci);
    extern vector<clientinfo *> connects, clients, bots;
    extern string smapname;

    /// Implementation in demos.cpp
    extern int demomillis;
    extern bool demonextmatch;
    extern stream *demotmp;
    extern stream *demorecord;
    extern stream *demoplayback;
    extern vector<demofile> demos;

    
    /// 
    void prunedemos(int extra = 0);

    /// 
    void adddemo();

    /// 
    void enddemorecord();

    /// 
    void writedemo(int chan, void *data, int len);

    /// 
    void recordpacket(int chan, void *data, int len);

    /// 
    void setupdemorecord();

    /// 
    void listdemos(int cn);

    /// 
    void cleardemos(int n);

    /// 
    void freegetmap(ENetPacket *packet);

    /// 
    void freegetdemo(ENetPacket *packet);

    /// 
    void senddemo(clientinfo *ci, int num);

    /// 
    void enddemoplayback();

    /// 
    void setupdemoplayback();

    /// 
    void readdemo();

    /// 
    void stopdemo();

};
