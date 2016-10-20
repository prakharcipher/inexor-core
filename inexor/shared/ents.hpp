#pragma once

#include "inexor/macros/constants.hpp"

#include "inexor/enumerations/entity_base_types.hpp"

#include "inexor/classes/entity.hpp"

#include "inexor/classes/entitylight.hpp"

#include "inexor/enumerations/entity_bitflags.hpp"

#include "inexor/classes/extentity.hpp"

#include "inexor/enumerations/client_states.hpp"

#include "inexor/enumerations/physics_types.hpp"

#include "inexor/enumerations/entity_types.hpp"

#include "inexor/enumerations/collision_types.hpp"

#include "inexor/classes/physent.hpp"

#include "inexor/enumerations/animation_types.hpp"

static const char * const animnames[] =
{
    "dead", "dying", "idle",
    "forward", "backward", "left", "right",
    "hold 1", "hold 2", "hold 3", "hold 4", "hold 5", "hold 6", "hold 7",
    "attack 1", "attack 2", "attack 3", "attack 4", "attack 5", "attack 6", "attack 7",
    "pain",
    "jump", "sink", "swim",
    "edit", "lag", "taunt", "win", "lose",
    "gun idle", "gun shoot",
    "vwep idle", "vwep shoot", "shield", "powerup",
    "mapmodel", "trigger"
};

#include "inexor/macros/animation_macros.hpp"

#include "inexor/classes/animinfo.hpp"

#include "inexor/classes/animinterpinfo.hpp"

#include "inexor/classes/dynent.hpp"
