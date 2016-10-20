#pragma once

#include "inexor/enumerations/sound_ids.hpp"
#include "inexor/enumerations/hud_icons.hpp"

/// pickup description structure
static struct itemstat 
{ 
	int add, max, sound; 
	const char *name; 
	int icon, info;
}

/// create an array of itemstat instances ('pickups')
itemstats[] =
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
