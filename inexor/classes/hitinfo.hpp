#pragma once

#include "inexor/deprecated/fixed_geom.hpp"

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
