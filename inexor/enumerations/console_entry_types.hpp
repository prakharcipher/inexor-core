#pragma once

namespace inexor {
namespace server {

    /// game console entry types
    enum
    {
        CON_CHAT       = 1<<8,
        CON_TEAMCHAT   = 1<<9,
        CON_GAMEINFO   = 1<<10,
        CON_FRAG_SELF  = 1<<11,
        CON_FRAG_OTHER = 1<<12,
        CON_TEAMKILL   = 1<<13
    };

};
};
