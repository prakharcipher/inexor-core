#pragma once

#include "inexor/deprecated/vector_template.hpp"

#include "inexor/enumerations/enum_gun_ids.hpp"
#include "inexor/enumerations/enum_netmsg_ids.hpp"
#include "inexor/enumerations/enum_client_states.hpp"

#include "inexor/classes/gamestate.hpp"
#include "inexor/classes/hitinfo.hpp"
#include "inexor/server/server_guns.hpp"
#include "inexor/server/server_entities.hpp"

#include "inexor/macros/constants.hpp"
#include "inexor/macros/gamemode_macros.hpp"


#include "inexor/classes/gameevent.hpp"

#include "inexor/engine/engine.hpp"

#include <enet/enet.h>


namespace inexor {
namespace server {

   
    extern ENetPacket *sendf(int cn, int chan, const char *format, ...);
    extern clientinfo *getinfo(int n);

    /// 
    struct timedevent : gameevent
    {
        int millis;

        bool flush(clientinfo *ci, int fmillis);
    };

    /// 
    struct shotevent : timedevent
    {
        int id, gun;
        vec from, to;
        vector<hitinfo> hits;

        void process(clientinfo *ci);
    };

    /// 
    struct explodeevent : timedevent
    {
        int id, gun;
        vector<hitinfo> hits;

        bool keepable() const { return true; }

        void process(clientinfo *ci);
    };

    /// 
    struct suicideevent : gameevent
    {
        void process(clientinfo *ci);
    };

    /// 
    struct pickupevent : gameevent
    {
        int ent;

        void process(clientinfo *ci);
    };

    bool gameevent::flush(clientinfo *ci, int fmillis)
    {
        process(ci);
        return true;
    }

    bool timedevent::flush(clientinfo *ci, int fmillis)
    {
        if(millis > fmillis) return false;
        else if(millis >= ci->lastevent)
        {
            ci->lastevent = millis;
            process(ci);
        }
        return true;
    }


    void clearevent(clientinfo *ci)
    {
        delete ci->events.remove(0);
    }

    void flushevents(clientinfo *ci, int millis)
    {
        while(ci->events.length())
        {
            gameevent *ev = ci->events[0];
            if(ev->flush(ci, millis)) clearevent(ci);
            else break;
        }
    }

    extern int gamemillis;

    void processevents()
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(curtime>0 && ci->state.quadmillis) ci->state.quadmillis = max(ci->state.quadmillis-curtime, 0);
            flushevents(ci, gamemillis);
        }
    }

    void cleartimedevents(clientinfo *ci)
    {
        int keep = 0;
        loopv(ci->events)
        {
            if(ci->events[i]->keepable())
            {
                if(keep < i)
                {
                    for(int j = keep; j < i; j++) delete ci->events[j];
                    ci->events.remove(keep, i - keep);
                    i = keep;
                }
                keep = i+1;
                continue;
            }
        }
        while(ci->events.length() > keep) delete ci->events.pop();
        ci->timesync = false;
    }


};
};
