#pragma once

/// 
enum
{
    MDL_CULL_VFC = 1<<0,
    MDL_CULL_DIST = 1<<1,
    MDL_CULL_OCCLUDED = 1<<2,
    MDL_CULL_QUERY = 1<<3,
    MDL_SHADOW = 1<<4,
    MDL_DYNSHADOW = 1<<5,
    MDL_LIGHT = 1<<6,
    MDL_DYNLIGHT = 1<<7,
    MDL_FULLBRIGHT = 1<<8,
    MDL_NORENDER = 1<<9,
    MDL_LIGHT_FAST = 1<<10,
    MDL_HUD = 1<<11,
    MDL_GHOST = 1<<12
};
