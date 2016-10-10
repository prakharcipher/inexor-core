#pragma once

#include "inexor/server/server_gameplay.hpp"
#include "inexor/shared/tools.hpp"

namespace inexor {
namespace server {

    /// pre-declaration
    struct clientinfo;

    /// abstract base class for events
    struct gameevent
    {
        virtual ~gameevent() {}

        virtual bool flush(clientinfo *ci, int fmillis);
        virtual void process(clientinfo *ci) {}

        virtual bool keepable() const { return false; }
    };

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


};
};
