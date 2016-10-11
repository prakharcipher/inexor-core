#pragma once

#include "inexor/server/server_clientinfo.hpp"
#include "inexor/macros/loop_macros.hpp"
#include "inexor/enumerations/enum_bot_levels.hpp"
#include "inexor/macros/gamemode_macros.hpp"

#include "inexor/deprecated/cube2_vector.hpp"

namespace inexor {
namespace server {

    bool shouldcheckteamkills = false;

    struct teamkillinfo
    {
        uint ip;
        int teamkills;
    };
    vector<teamkillinfo> teamkills;

    void addteamkill(clientinfo *actor, clientinfo *victim, int n);

};
};
