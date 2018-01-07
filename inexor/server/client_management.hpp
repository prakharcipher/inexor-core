#pragma once

#include "inexor/shared/cube_types.hpp"
#include "inexor/shared/cube_vector.hpp"
#include "inexor/shared/ents.hpp"
#include "inexor/fpsgame/fpsstate.hpp"
#include "inexor/network/SharedTree.hpp"
#include "inexor/network/legacy/administration.hpp"
#include "inexor/util/legacy_time.hpp"

#include <enet/enet.h>

#include <algorithm>


#define DEFAULTCLIENTS 16


namespace server {

extern ENetPeer *getclientpeer(int i);
extern uint getclientip(int n);

extern SharedVar<int> maxclients;
extern SharedVar<int> maxdupclients;
extern int reserveclients();

/// server side version of "dynent" type
struct client
{
    bool connected;
    int num;
    ENetPeer *peer;
    string hostname;
    void *info;
};
extern vector<client *> client_connections;

extern client &add_client_connection(ENetPeer *peer);

/// After some period of time without response we disconnect a client.
extern void check_clients_timed_out();

extern void disconnect_client(int n, int reason);
extern bool has_clients();
extern int get_num_clients();

/// List all connected clients (game players)
/// @param exclude exclude the player with this client number from the counter.
/// @param nospec exclude spectators (expect they have priviledge leveles and parameter priv is true)
/// @param priv see nospec: count priviledges users (master, admin, local) even when they are in spectator mode.
/// @param noai exclude bots from the counter.
extern int numclients(int exclude = -1, bool nospec = true, bool noai = true, bool priv = false);

static constexpr int DEATHMILLIS = 300;

extern int nextexceeded;

template <int N>
struct projectilestate
{
    int projs[N];
    int numprojs;

    projectilestate() : numprojs(0) {}

    void reset() { numprojs = 0; }

    void add(int val)
    {
        if(numprojs>=N) numprojs = 0;
        projs[numprojs++] = val;
    }

    bool remove(int val)
    {
        loopi(numprojs) if(projs[i]==val)
        {
            projs[i] = projs[--numprojs];
            return true;
        }
        return false;
    }
};

/// server specialisation of the fpsstate (?)
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

    gamestate() : state(CS_DEAD), editstate(CS_DEAD), lifesequence(0) {}

    bool isalive(int gamemillis)
    {
        return state==CS_ALIVE || (state==CS_DEAD && gamemillis - lastdeath <= DEATHMILLIS);
    }

    bool waitexpired(int gamemillis)
    {
        return gamemillis - lastshot >= gunwait;
    }

    void reset()
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

    void respawn()
    {
        fpsstate::respawn();
        o = vec(-1e10f, -1e10f, -1e10f);
        deadflush = 0;
        lastspawn = -1;
        lastshot = 0;
        tokens = 0;
    }

    void reassign()
    {
        respawn();
        rockets.reset();
        grenades.reset();
        bombs.reset();
    }

    void setbackupweapon(int weapon)
    {
        fpsstate::backupweapon = weapon;
    }
};

struct clientinfo;

// Events

struct gameevent
{
    virtual ~gameevent() {}

    virtual bool flush(clientinfo *ci, int fmillis);
    virtual void process(clientinfo *ci) {}

    virtual bool keepable() const { return false; }
};

struct timedevent : gameevent
{
    int millis;

    bool flush(clientinfo *ci, int fmillis) override;
};

struct hitinfo
{
    int target;
    int lifesequence;
    int rays;
    float dist;
    vec dir;
};

struct shotevent : timedevent
{
    int id, gun;
    vec from, to;
    vector<hitinfo> hits;

    void process(clientinfo *ci) override;
};

struct explodeevent : timedevent
{
    int id, gun;
    vector<hitinfo> hits;

    bool keepable() const override { return true; }

    void process(clientinfo *ci) override;
};

struct suicideevent : gameevent
{
    void process(clientinfo *ci) override;
};

struct pickupevent : gameevent
{
    int ent;

    void process(clientinfo *ci) override;
};


struct clientinfo
{
    int clientnum, ownernum, connectmillis, sessionid, overflow;
    string name, tag, team, mapvote;
    int playermodel;
    int fov;
    int modevote;
    int privilege;
    bool connected, timesync;
    int gameoffset, lastevent, pushed, exceeded;
    gamestate state;
    vector<gameevent *> events;
    vector<uchar> position, messages;
    uchar *wsdata;
    int wslen;
    vector<clientinfo *> bots;
    int ping, aireinit;
    string clientmap;
    int mapcrc;
    bool warned, gameclip;
    ENetPacket *getdemo, *getmap, *clipboard;
    int lastclipboard, needclipboard;

    clientinfo() : getdemo(NULL), getmap(NULL), clipboard(NULL) { reset(); }
    ~clientinfo() { events.deletecontents(); cleanclipboard(); }

    void addevent(gameevent *e)
    {
        if(state.state==CS_SPECTATOR || events.length()>100) delete e;
        else events.add(e);
    }

    enum
    {
        PUSHMILLIS = 3000
    };

    int calcpushrange()
    {
        ENetPeer *peer = getclientpeer(ownernum);
        return PUSHMILLIS + (peer ? peer->roundTripTime + peer->roundTripTimeVariance : ENET_PEER_DEFAULT_ROUND_TRIP_TIME);
    }

    bool checkpushed(int millis, int range)
    {
        return millis >= pushed - range && millis <= pushed + range;
    }

    void scheduleexceeded()
    {
        if(state.state!=CS_ALIVE || !exceeded) return;
        int range = calcpushrange();
        if(!nextexceeded || exceeded + range < nextexceeded) nextexceeded = exceeded + range;
    }

    void setexceeded()
    {
        if(state.state==CS_ALIVE && !exceeded && !checkpushed(gamemillis, calcpushrange())) exceeded = gamemillis;
        scheduleexceeded();
    }

    void setpushed()
    {
        pushed = std::max(pushed, gamemillis);
        if(exceeded && checkpushed(exceeded, calcpushrange())) exceeded = 0;
    }

    bool checkexceeded()
    {
        return state.state==CS_ALIVE && exceeded && gamemillis > exceeded + calcpushrange();
    }

    void mapchange()
    {
        mapvote[0] = 0;
        modevote = INT_MAX;
        state.reset();
        events.deletecontents();
        overflow = 0;
        timesync = false;
        lastevent = 0;
        exceeded = 0;
        pushed = 0;
        clientmap[0] = '\0';
        mapcrc = 0;
        warned = false;
        gameclip = false;
    }

    void reassign()
    {
        state.reassign();
        events.deletecontents();
        timesync = false;
        lastevent = 0;
    }

    void cleanclipboard(bool fullclean = true)
    {
        if(clipboard) { if(--clipboard->referenceCount <= 0) enet_packet_destroy(clipboard); clipboard = NULL; }
        if(fullclean) lastclipboard = 0;
    }

    void reset()
    {
        name[0] = team[0] = tag[0] = 0;
        playermodel = -1;
        fov = 100;
        privilege = PRIV_NONE;
        connected = false;
        position.setsize(0);
        messages.setsize(0);
        ping = 0;
        aireinit = 0;
        needclipboard = 0;
        cleanclipboard();
        mapchange();
    }

    int geteventmillis(int servmillis, int clientmillis)
    {
        if(!timesync || (events.empty() && state.waitexpired(servmillis)))
        {
            timesync = true;
            gameoffset = servmillis - clientmillis;
            return servmillis;
        } else return gameoffset + clientmillis;
    }
};

/// colorful version of the clients name
/// (if two people share the same name or ci is a bot)
extern const char *colorname(clientinfo *ci, const char *name = nullptr);

extern bool player_connected(clientinfo *ci, const char *password, const char *mapwish, int modewish);

// TODO remove connects array and merge with client_connections
extern vector<clientinfo *> connects, clients, bots;

extern clientinfo *get_client_info(int n, bool findbots = true);

/// New map: we throw away the scores of disconnected players.
extern void resetdisconnectedplayerscores();

namespace aiman
{
extern void removeai(clientinfo *ci);
extern void clearai();
extern void checkai();
extern void reqadd(clientinfo *ci, int skill);
extern void reqdel(clientinfo *ci);
extern void setbotlimit(clientinfo *ci, int limit);
extern void setbotbalance(clientinfo *ci, bool balance);
extern void changemap();
extern void addclient(clientinfo *ci);
extern void changeteam(clientinfo *ci);
}

extern int mastermode, mastermask;
extern int get_mastermode_int();
extern bool setmaster(clientinfo *ci, bool val, const char *pass = "", bool force = false, bool trial = false);
extern void change_mastermode(int mm, int sendernum, clientinfo *actor);

////// Bans

extern void addban(uint ip, int expire);

extern void clearbans(clientinfo *actor);
extern void check_bans_expired();
extern bool trykick(clientinfo *ci, int victim, const char *reason = NULL, bool trial = false);
/// Kick all clients with this IP
/// (including bots and people in the same LAN)
extern void kickclients(uint ip, clientinfo *actor = NULL, int priv = PRIV_NONE);

} // ns server

#define MM_MODE 0xF
#define MM_AUTOAPPROVE 0x1000
#define MM_PRIVSERV (MM_MODE | MM_AUTOAPPROVE)
#define MM_PUBSERV ((1<<MM_OPEN) | (1<<MM_VETO))
#define MM_COOPSERV (MM_AUTOAPPROVE | MM_PUBSERV | (1<<MM_LOCKED))

