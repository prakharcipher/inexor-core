#pragma once

#include "inexor/classes/entitylight.hpp"
#include "inexor/enumerations/entity_bitflags.hpp"

/// 
struct extentity : entity                       // part of the entity that doesn't get saved to disk
{
    int flags;  // the only dynamic state of a map entity
    entitylight light;
    extentity *attached;

    extentity() : flags(0), attached(NULL) {}

    bool spawned() const { return (flags&EF_SPAWNED) != 0; }
    void setspawned(bool val) { if(val) flags |= EF_SPAWNED; else flags &= ~EF_SPAWNED; }
    void setspawned() { flags |= EF_SPAWNED; }
    void clearspawned() { flags &= ~EF_SPAWNED; }
};
