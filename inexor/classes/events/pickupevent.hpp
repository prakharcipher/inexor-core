#pragma once

namespace server {

    /// 
    struct pickupevent : gameevent
    {
        int ent;

        void process(clientinfo *ci);
    };

};
