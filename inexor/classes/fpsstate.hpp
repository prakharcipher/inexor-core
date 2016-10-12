#pragma once


#include "inexor/enumerations/enum_gun_ids.hpp"
#include "inexor/enumerations/enum_bot_types.hpp"

#include "inexor/server/server_entities.hpp"

#include "inexor/util/random.hpp"

#include <algorithm>


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

        fpsstate();
        
        /// set initial ammo
        void baseammo(int gun, int k, int scale);
        
        /// add ammo
        void addammo(int gun, int k, int scale);

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
        int hasammo(int gun, int exclude);
        
    };

};
};
