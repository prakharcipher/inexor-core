#pragma once

#include "inexor/server/server_enums.hpp"
#include "inexor/server/server_gamemodes.hpp"
#include "inexor/server/server_entities.hpp"
#include <algorithm>
#include "inexor/server/server_macros.hpp"

namespace inexor {
namespace server {

    /// 
    struct fpsstate
    {
        int health, maxhealth;
        int armour, armourtype;
        int quadmillis;
        int gunselect, gunwait;
        int ammo[NUMGUNS];
        int aitype, skill;
        int backupweapon; //no ammo - weapon
        int bombradius;
        int bombdelay;

        fpsstate() : maxhealth(100), aitype(AI_NONE), skill(0), backupweapon(GUN_FIST);

        /// set initial ammo
        void baseammo(int gun, int k = 2, int scale = 1);
        
        /// add ammo
        void addammo(int gun, int k = 1, int scale = 1);

        /// ammo limitation reached/exceeded?
        bool hasmaxammo(int type);
        
        /// check if I can pick up this item depending on the radius
        bool canpickup(int type);

        /// pick up this item
        void pickup(int type);

        /// reset all members when spawning
        void respawn();

        /// configure spawn settings (weapons, ammo, health...) depending on game mode
        void spawnstate(int gamemode);

        /// just subtract damage here, we can set death, etc. later in code calling this
        int dodamage(int damage);

        /// is there ammo left for this gun
        int hasammo(int gun, int exclude = -1);
        
    };

};
};
