#pragma once

#include "inexor/engine/engine.hpp"
#include "inexor/server/server_enums.hpp"
#include "inexor/server/server_fpsstate.hpp"

namespace inexor {
namespace server {

    /// 
    static const int DEATHMILLIS = 300;

    /// 
    extern int gamemode;
    extern int gamemillis, nextexceeded;

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
    struct clientinfo;

    /// 
    void suicide(clientinfo *ci);


    /// please note that tempalte must have their implementation in the header file!
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

    struct gameevent;

    /// 
    struct clientinfo
    {
        int clientnum, ownernum, connectmillis, sessionid, overflow;
        string name, tag, team, mapvote;
        int playermodel;
        int fov;
        int modevote;
        int privilege;
        bool connected, local, timesync;
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
        int connectauth;
        uint authreq;
        string authname, authdesc;
        void *authchallenge;
        int authkickvictim;
        char *authkickreason;

        clientinfo();
        ~clientinfo();

        void addevent(gameevent *e);

        enum
        {
            PUSHMILLIS = 3000
        };

        int calcpushrange();

        bool checkpushed(int millis, int range);

        void scheduleexceeded();

        void setexceeded();

        void setpushed();

        bool checkexceeded();

        void mapchange();

        void reassign();

        void cleanclipboard(bool fullclean = true);

        void cleanauthkick();

        void cleanauth(bool full = true);

        void reset();

        int geteventmillis(int servmillis, int clientmillis);

    };


};
};
