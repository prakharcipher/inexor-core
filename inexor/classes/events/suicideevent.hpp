#pragma once

namespace server {

    struct clientinfo;
    extern void suicide(clientinfo *ci);

    /// 
    struct suicideevent : gameevent
    {
        void process(clientinfo *ci);
    };

    /// 
    void suicideevent::process(clientinfo *ci)
    {
        suicide(ci);
    }

};
