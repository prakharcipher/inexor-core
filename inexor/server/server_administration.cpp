#include "inexor/server/server_administration.hpp"

namespace inexor {
namespace server {

    void addteamkill(clientinfo *actor, clientinfo *victim, int n)
    {
        if(!m_timed || actor->state.aitype != AI_NONE || actor->local || actor->privilege || (victim && victim->state.aitype != AI_NONE)) return;
        shouldcheckteamkills = true;
        uint ip = getclientip(actor->clientnum);
        loopv(teamkills) if(teamkills[i].ip == ip) 
        { 
            teamkills[i].teamkills += n;
            return;
        }
        teamkillinfo &tk = teamkills.add();
        tk.ip = ip;
        tk.teamkills = n;
    }

};
};
