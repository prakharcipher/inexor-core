#pragma once

#include "inexor/engine/engine.hpp"

namespace inexor {
namespace server {

    /// 
    struct hitinfo
    {
        int target;
        int lifesequence;
        int rays;
        float dist;
        vec dir;
    };

};
};
