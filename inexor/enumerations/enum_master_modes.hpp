#pragma once

namespace inexor {
namespace server {
    
    /// master mode levels
    enum 
    { 
        MM_AUTH = -1,
        MM_OPEN = 0,
        MM_VETO,
        MM_LOCKED, 
        MM_PRIVATE, 
        MM_PASSWORD, 
        MM_START = MM_AUTH
    };

};
};
