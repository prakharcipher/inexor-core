#pragma once

namespace inexor {
namespace server {

    /// 
    enum
    {
        M_TEAM       = 1<<0,   /// game mode contains teams
        M_NOITEMS    = 1<<1,   /// game mode has no items
        M_NOAMMO     = 1<<2,   /// game mode has no ammo?
        M_INSTA      = 1<<3,   /// game mode has an instagib modifier
        M_EFFICIENCY = 1<<4,   /// game mode has an efficiency modifier
        M_TACTICS    = 1<<5,   /// game mode offers random spawn weapons (see tactics mode)
        M_CAPTURE    = 1<<6,   /// game mode is about capturing bases
        M_REGEN      = 1<<7,   /// game mode is about capturing supply bases (see regencapture mode)
        M_CTF        = 1<<8,   /// game mode is about capturing a flag
        M_PROTECT    = 1<<9,   /// game mode is about protecting a flag
        M_HOLD       = 1<<10,  /// game mode is about holding a flag (for 20 seconds)
        M_OVERTIME   = 1<<11,  /// game mode allows overtime (?)
        M_EDIT       = 1<<12,  /// game mode allows cooperative editing (coopedit)
        M_DEMO       = 1<<13,  /// game mode is a demo playback
        M_LOCAL      = 1<<14,  /// game mode is played in singleplayer only (locally)
        M_LOBBY      = 1<<15,  /// game mode does not imply certain grouped gameplay but also allows to built lobbys (pseudoteams working against each other)
        M_DMSP       = 1<<16,  /// death match single player
        M_CLASSICSP  = 1<<17,  /// classic singleplayer
        M_SLOWMO     = 1<<18,  /// game mode is played in slow motion
        M_COLLECT    = 1<<19,  /// game mode is about collecting skulls

        M_LMS        = 1<<20,  /// 
        M_BOMB       = 1<<21,  /// 
        M_TIMEFORWARD= 1<<22,  ///
        M_OBSTACLES  = 1<<23,  ///
        M_HIDEANDSEEK= 1<<24,  /// 
                               //M_RACE       = 1<<25,
    };

};
};
