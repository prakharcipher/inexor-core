#pragma once

#include "inexor/server/server_enums.hpp"

namespace inexor {
namespace server {

    /// scoreboard team block description
    struct teaminfo
    {
        char team[MAXTEAMLEN+1];
        int frags;
    };

};
};
