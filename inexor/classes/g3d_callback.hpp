#pragma once

#include "inexor/classes/g3d_gui.hpp"

extern int totalmillis;

/// 
struct g3d_callback
{
    virtual ~g3d_callback() {}

    int starttime() { return totalmillis; }

    virtual void gui(g3d_gui &g, bool firstpass) = 0;
};
