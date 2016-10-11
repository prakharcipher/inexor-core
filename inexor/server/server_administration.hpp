#pragma once

#include "inexor/server/server_clientinfo.hpp"
#include "inexor/macros/loop_macros.hpp"
#include "inexor/enumerations/enum_bot_levels.hpp"
#include "inexor/macros/gamemode_macros.hpp"
#include "inexor/macros/type_definitions.hpp"
#include "inexor/deprecated/vector_template.hpp"

namespace inexor {
namespace server {

    bool shouldcheckteamkills = false;
    extern uint getclientip(int n);

    struct teamkillinfo
    {
        uint ip;
        int teamkills;
    };
    vector<teamkillinfo> teamkills;

    void addteamkill(clientinfo *actor, clientinfo *victim, int n);

};
};
