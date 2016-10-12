#pragma once

//#include "inexor/deprecated/type_definitions.hpp"
/// TODO: migrate to std::vector
#include "inexor/deprecated/vector_template.hpp"
#include "inexor/macros/gamemode_macros.hpp"
#include "inexor/enumerations/enum_bot_types.hpp"

#include "inexor/classes/clientinfo.hpp"


namespace inexor {
namespace server {

    extern bool shouldcheckteamkills;
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
