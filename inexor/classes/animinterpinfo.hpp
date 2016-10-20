#pragma once

#include "inexor/classes/animinfo.hpp"

/// used for animation blending of animated characters
struct animinterpinfo
{
    animinfo prev, cur;
    int lastswitch;
    void *lastmodel;

    animinterpinfo() : lastswitch(-1), lastmodel(NULL) {}

    void reset() { lastswitch = -1; }
};
