#pragma once

#include "inexor/enumerations/client_states.hpp"
#include "inexor/enumerations/entity_types.hpp"
#include "inexor/enumerations/collision_types.hpp"
#include "inexor/enumerations/physics_types.hpp"


// base entity type, can be affected by physics
struct physent                                  
{
    vec o, vel, falling;                        // origin, velocity
    vec deltapos, newpos;                       // movement interpolation
    float yaw, pitch, roll;
    float maxspeed;                             // cubes per second, 100 for player
    int timeinair;
    float radius, eyeheight, aboveeye;          // bounding box size
    float xradius, yradius, zmargin;
    vec floor;                                  // the normal of floor the dynent is on

    int inwater;
    bool jumping;
    char move, strafe;

    uchar physstate;                            // one of PHYS_* above
    uchar state, editstate;                     // one of CS_* above
    uchar type;                                 // one of ENT_* above
    uchar collidetype;                          // one of COLLIDE_* above           

    bool blocked;                               // used by physics to signal ai

    physent() : o(0, 0, 0), deltapos(0, 0, 0), newpos(0, 0, 0), yaw(0), pitch(0), roll(0), maxspeed(100), 
        radius(4.1f), eyeheight(14), aboveeye(1), xradius(4.1f), yradius(4.1f), zmargin(0),
        state(CS_ALIVE), editstate(CS_ALIVE), type(ENT_PLAYER),
        collidetype(COLLIDE_ELLIPSE),
        blocked(false)
    { reset(); }

    void resetinterp()
    {
        newpos = o;
        deltapos = vec(0, 0, 0);
    }

    void reset()
    {
        inwater = 0;
        timeinair = 0;
        jumping = false;
        strafe = move = 0;
        physstate = PHYS_FALL;
        vel = falling = vec(0, 0, 0);
        floor = vec(0, 0, 1);
    }

    vec feetpos(float offset = 0) const { return vec(o).add(vec(0, 0, offset - eyeheight)); }
    vec headpos(float offset = 0) const { return vec(o).add(vec(0, 0, offset)); }

    bool maymove() const { return timeinair || physstate < PHYS_FLOOR || vel.squaredlen() > 1e-4f || deltapos.squaredlen() > 1e-4f; } 
};
