#pragma once

#include "inexor/macros/deprecated_string_macro.hpp"
#include "inexor/macros/type_definitions.hpp" // uchar
#include "inexor/enumerations/enum_client_states.hpp"
#include "inexor/macros/memfree_macros.hpp"
#include "inexor/enumerations/enum_admin_levels.hpp"
#include "inexor/macros/define_null_macro.hpp"

#include <enet/enet.h>
#include <algorithm>

#include "inexor/server/server_gamestate.hpp"

#include "inexor/deprecated/vector_template.hpp"


namespace inexor {
namespace server {

    extern int gamemillis, nextexceeded;
    extern ENetPeer *getclientpeer(int i);
    extern void freechallenge(void *answer);

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
