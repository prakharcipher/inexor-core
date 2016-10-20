#pragma once

#include "inexor/enumerations/material_flags.hpp"

/// 
enum
{
    MAT_AIR      = 0,                      // the default, fill the empty space with air
    MAT_WATER    = 1 << MATF_VOLUME_SHIFT, // fill with water, showing waves at the surface
    MAT_LAVA     = 2 << MATF_VOLUME_SHIFT, // fill with lava
    MAT_GLASS    = 3 << MATF_VOLUME_SHIFT, // behaves like clip but is blended blueish

    MAT_NOCLIP   = 1 << MATF_CLIP_SHIFT,  // collisions always treat cube as empty
    MAT_CLIP     = 2 << MATF_CLIP_SHIFT,  // collisions always treat cube as solid
    MAT_GAMECLIP = 3 << MATF_CLIP_SHIFT,  // game specific clip material

    MAT_DEATH    = 1 << MATF_FLAG_SHIFT,  // force player suicide
    MAT_ALPHA    = 4 << MATF_FLAG_SHIFT   // alpha blended
};
