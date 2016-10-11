#pragma once

#include "inexor/macros/constants.hpp"

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
