#include "inexor/network/legacy/game_types.hpp"  // for ::N_PAUSEGAME
#include "inexor/server/client_management.hpp"   // for clientinfo
#include "inexor/server/game_management.hpp"
#include "inexor/server/network_send.hpp"        // for sendf

namespace server {

bool gamepaused = false;

void pausegame(bool val, clientinfo *ci)
{
    if(gamepaused==val) return;
    gamepaused = val;
    sendf(-1, 1, "riii", N_PAUSEGAME, gamepaused ? 1 : 0, ci ? ci->clientnum : -1);
}

} // ns server

