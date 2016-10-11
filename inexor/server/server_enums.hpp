#pragma once

/// particle types
#include "inexor/enumerations/enum_particle_types.hpp"
/// sounds
#include "inexor/enumerations/enum_sound_ids.hpp"
/// guns
#include "inexor/enumerations/enum_gun_ids.hpp"


namespace inexor {
namespace server {
    
   
    /// TODO: remove this duplicated code
    #define EXP_DISTSCALE 1.5f
    #define EXP_SELFDAMDIV 2

    #define DMF 16.0f   /// for world locations
    #define DNF 100.0f  /// for normalized vectors
    #define DVELF 1.0f  /// for playerspeed based velocity vectors

    /// important teamspecific declarations
    #define MAXTEAMS 128
    #define MAXNAMELEN 15  /// max player name length
    #define MAXTEAMLEN 4   /// max team name length
    #define MAXTAGLEN 8    /// max player tag length
    #define BOTTAG "Bot"   /// all bots share this tag

    /// 0
    #ifndef NULL
        #ifdef __cplusplus
            #define NULL 0
        #else
            #define NULL ((void *)0)
        #endif
    #endif

    /// 
    static const struct guninfo
    { 
        int sound, attackdelay, damage, spread, projspeed;
        int kickamount, range, rays, hitpush, exprad, ttl; 
        const char *name, *file; short part;
    };

    guninfo guns[NUMGUNS] =
    {
        { S_PUNCH1,    250,  50,   0,   0,  0,   14,  1,  80,   0,    0, "fist",            "chainsaw",        0 },
        { S_SG,       1400,  10, 400,   0, 20, 1024, 20,  80,   0,    0, "shotgun",         "shotgun",         0 },
        { S_CG,        100,  30, 100,   0,  7, 1024,  1,  80,   0,    0, "chaingun",        "chaingun",        0 },
        { S_RLFIRE,    800, 120,   0, 320, 10, 1024,  1, 160,  40,    0, "rocketlauncher",  "rocket",          0 },
        { S_RIFLE,    1500, 100,   0,   0, 30, 2048,  1,  80,   0,    0, "rifle",           "rifle",           0 },
        { S_FLAUNCH,   600,  90,   0, 200, 10, 1024,  1, 250,  45, 1500, "grenadelauncher", "grenadelauncher", 0 },
        { S_PISTOL,    500,  35,  50,   0,  7, 1024,  1,  80,   0,    0, "pistol",          "pistol",          0 },
        { S_FEXPLODE,  375, 200,   8,  20,  0, 1024,  1, 150,  40, 1500, "bomb",            "cannon",          0 },
        { S_FLAUNCH,   200,  20,   0, 200,  1, 1024,  1,  80,  40,    0, "fireball",        NULL,              PART_FIREBALL1 },
        { S_ICEBALL,   200,  40,   0, 120,  1, 1024,  1,  80,  40,    0, "iceball",         NULL,              PART_FIREBALL2 },
        { S_SLIMEBALL, 200,  30,   0, 640,  1, 1024,  1,  80,  40,    0, "slimeball",       NULL,              PART_FIREBALL3 },
        { S_PIGR1,     250,  50,   0,   0,  1,   12,  1,  80,   0,    0, "bite",            NULL,              0 },
        { -1,            0, 120,   0,   0,  0,    0,  1,  80,  40,    0, "barrel",          NULL,              0 },
        { S_FEXPLODE,  375, 200,   8,  20,  0, 1024,  1, 150,  40, 1500, "bomb_splinter",   NULL,              0 },

    };


};
};
