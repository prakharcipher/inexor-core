#pragma once

#include "inexor/classes/animinterpinfo.hpp"

struct occludequery;
struct ragdolldata;

// animated characters, or characters that can receive input
struct dynent : physent
{
    bool k_left, k_right, k_up, k_down;         // see input code

    entitylight light;
    animinterpinfo animinterp[MAXANIMPARTS];
    ragdolldata *ragdoll;
    occludequery *query;
    int occluded, lastrendered;

    dynent() : ragdoll(NULL), query(NULL), occluded(0), lastrendered(0)
    { 
        reset(); 
    }

    ~dynent()
    {
        #ifndef STANDALONE
        extern void cleanragdoll(dynent *d);
        if(ragdoll) cleanragdoll(this);
        #endif
    }

    void stopmoving()
    {
        k_left = k_right = k_up = k_down = jumping = false;
        move = strafe = 0;
    }

    void reset()
    {
        physent::reset();
        stopmoving();
        loopi(MAXANIMPARTS) animinterp[i].reset();
    }

    vec abovehead() { return vec(o).add(vec(0, 0, aboveeye+4)); }
};
