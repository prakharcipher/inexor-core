#pragma once

#include "inexor/deprecated/cube2_vector.hpp"

#include "inexor/enumerations/enum_sound_ids.hpp"
#include "inexor/enumerations/enum_hudicon_ids.hpp"
#include "inexor/enumerations/enum_gun_ids.hpp"
#include "inexor/enumerations/enum_armor_ids.hpp"
#include "inexor/enumerations/enum_netmsg_ids.hpp"
#include "inexor/enumerations/enum_entity_types.hpp"

#include "inexor/server/server_clientinfo.hpp"
#include "inexor/server/server_gameplay.hpp"

#include "inexor/macros/gamemode_macros.hpp"

namespace inexor {
namespace server {

    /// server side version of "entity" type
    struct server_entity
    {
        int type;
        int spawntime;
        bool spawned;
    };

    vector<server_entity> sents;

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
    
    extern int gamemillis, gamelimit;

    bool canspawnitem(int type)
    {
        if(m_bomb) return (type>=I_BOMBS && type<=I_BOMBDELAY);
        else return !m_noitems && (type>=I_SHELLS && type<=I_QUAD && (!m_noammo || type<I_SHELLS || type>I_CARTRIDGES));
    }

    /// 
    int spawntime(int type);

    /// 
    bool delayspawn(int type);

    /// server side item pickup, acknowledge first client that gets it
    bool pickup(int i, int sender);



};
};
