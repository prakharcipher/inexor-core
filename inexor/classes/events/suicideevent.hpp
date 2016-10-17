#pragma once

namespace server {

    /// 
    struct suicideevent : gameevent
    {
        void process(clientinfo *ci);
    };

};
