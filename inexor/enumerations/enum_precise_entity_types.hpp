#pragma once

#include "inexor/enumerations/enum_entity_types.hpp"

enum
{
    NOTUSED = ET_EMPTY,         /// entity slot not in used in maps
    LIGHT = ET_LIGHT,           /// lightsource, attr1 = radius, attr2 = intensity
    MAPMODEL = ET_MAPMODEL,     /// attr1 = z-angle, attr2 = idx
    PLAYERSTART,                /// attr1 = z-angle, attr2 = team
    ENVMAP = ET_ENVMAP,         /// attr1 = radius
    PARTICLES = ET_PARTICLES,   /// particles (may becomes deprecated because of the new particle system)
    MAPSOUND = ET_SOUND,        /// sounds
    SPOTLIGHT = ET_SPOTLIGHT,   /// cone-shaped spotlights

    /// prefix I_ stands for "Inexor"...
    I_SHELLS, I_BULLETS, I_ROCKETS, I_ROUNDS, I_GRENADES, I_CARTRIDGES, /// ammo pickups
    I_BOMBS = ET_BOMBS,         /// bomberman game mode
    I_BOMBRADIUS,               /// bomb radius (see bomberman game mode)
    I_BOMBDELAY,                /// bomberman game mode
	I_HEALTH, I_BOOST,          /// bomberman game mode
    I_GREENARMOUR, I_YELLOWARMOUR, /// bomberman game mode
    I_QUAD,                     /// bomberman game mode

    TELEPORT,                   /// attr1 = idx, attr2 = model, attr3 = tag
    TELEDEST,                   /// attr1 = z-angle, attr2 = idx
    MONSTER,                    /// attr1 = z-angle, attr2 = monstertype
    CARROT,                     /// attr1 = tag, attr2 = type
    JUMPPAD,                    /// attr1 = z-push, attr2 = y-push, attr3 = x-push
    BASE,                       /// base (regencapture and capture game modes)
    RESPAWNPOINT,               /// singleplayer: respawn points ('respawnpoint set' :))
    BOX,                        /// attr1 = z-angle, attr2 = idx, attr3 = weight
    BARREL,                     /// attr1 = z-angle, attr2 = idx, attr3 = weight, attr4 = health
    PLATFORM,                   /// attr1 = z-angle, attr2 = idx, attr3 = tag, attr4 = speed
    ELEVATOR,                   /// attr1 = z-angle, attr2 = idx, attr3 = tag, attr4 = speed
    FLAG,                       /// attr1 = z-angle, attr2 = team
    OBSTACLE = ET_OBSTACLE,     /// attr1 = z-angle, attr2 = idx (mapmodel index), attr3 = health, attr4 = weight, attr5 = respawnmillis
    MAXENTTYPES
};