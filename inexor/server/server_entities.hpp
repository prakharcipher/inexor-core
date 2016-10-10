#pragma once

namespace inexor {
namespace server {

    /// server side version of "entity" type
    struct server_entity
    {
        int type;
        int spawntime;
        bool spawned;
    };

};
};
