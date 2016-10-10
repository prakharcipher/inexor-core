#pragma once

#include "inexor/server/server_sounds.hpp"
#include "inexor/server/server_guns.hpp"

namespace inexor {
namespace server {

    /// server side version of "entity" type
    struct server_entity
    {
        int type;
        int spawntime;
        bool spawned;
    };

    /// enumeration for icons
    enum
    {
        HICON_BLUE_ARMOUR = 0,
        HICON_GREEN_ARMOUR,
        HICON_YELLOW_ARMOUR,

        HICON_HEALTH,

        HICON_FIST,
        HICON_SG,
        HICON_CG,
        HICON_RL,
        HICON_RIFLE,
        HICON_GL,
        HICON_PISTOL,
        HICON_BOMB,

        HICON_QUAD,

        HICON_RED_FLAG,
        HICON_BLUE_FLAG,
        HICON_NEUTRAL_FLAG,

        HICON_TOKEN,

        // bomberman
        HICON_BOMBRADIUS,
        HICON_BOMBDELAY,

        HICON_X       = 20,
        HICON_Y       = 1650,
        HICON_TEXTY   = 1644,
        HICON_STEP    = 490,
        HICON_SIZE    = 120,
        HICON_SPACE   = 40
    };


    enum 
    {
        A_BLUE,
        A_GREEN,
        A_YELLOW
    };

    static struct itemstat 
    { 
        int add, max, sound; 
        const char *name; 
        int icon, info;
    };

    /// create an array of itemstat instances ('pickups')
    /// TODO: replace this hardcoded stuff and move on to JSON!
    itemstat itemstats[] =
    {
        {10,    30,    S_ITEMAMMO,   "SG", HICON_SG,            GUN_SG},
        {20,    60,    S_ITEMAMMO,   "CG", HICON_CG,            GUN_CG},
        {5,     15,    S_ITEMAMMO,   "RL", HICON_RL,            GUN_RL},
        {5,     15,    S_ITEMAMMO,   "RI", HICON_RIFLE,         GUN_RIFLE},
        {10,    30,    S_ITEMAMMO,   "GL", HICON_GL,            GUN_GL},
        {30,    120,   S_ITEMAMMO,   "PI", HICON_PISTOL,        GUN_PISTOL},
        {1,     12,    S_ITEMAMMO,   "BO", HICON_BOMB,          GUN_BOMB},
        {1,     10,    S_ITEMPUP,    "BR", HICON_BOMBRADIUS,    -1},
        {1,     7,     S_ITEMPUP,    "BD", HICON_BOMBDELAY,     -1},
        {25,    100,   S_ITEMHEALTH, "H",  HICON_HEALTH,        -1},
        {10,    1000,  S_ITEMHEALTH, "MH", HICON_HEALTH,        -1},
        {100,   100,   S_ITEMARMOUR, "GA", HICON_GREEN_ARMOUR,  A_GREEN},
        {200,   200,   S_ITEMARMOUR, "YA", HICON_YELLOW_ARMOUR, A_YELLOW},
        {20000, 30000, S_ITEMPUP,    "Q",  HICON_QUAD,          -1}
    };

};
};
