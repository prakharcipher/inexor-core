#pragma once

/// gamemode byteflag
#include "inexor/enumerations/enum_gamemode_bitmask.hpp"

namespace inexor {
namespace server {

    /// 
    extern int gamemode;

    /// structure for game mode description
    static struct gamemodeinfo
    {
        const char *name; /// game mode's name
        int flags;        /// a bitmask container (see flags above)
    };
    
    gamemodeinfo gamemodes[] =
    {
        { "SP", M_LOCAL | M_CLASSICSP},
        { "DMSP", M_LOCAL | M_DMSP},
        { "demo", M_DEMO | M_LOCAL},
        { "ffa", M_LOBBY},
        { "coop edit", M_EDIT},
        { "teamplay", M_TEAM},
        { "instagib", M_NOITEMS | M_INSTA},
        { "insta team", M_NOITEMS | M_INSTA | M_TEAM},
        { "efficiency", M_NOITEMS | M_EFFICIENCY},
        { "effic team", M_NOITEMS | M_EFFICIENCY | M_TEAM},
        { "tactics", M_NOITEMS | M_TACTICS},
        { "tac team", M_NOITEMS | M_TACTICS | M_TEAM},
        { "capture", M_NOAMMO | M_TACTICS | M_CAPTURE | M_TEAM},
        { "regen capture", M_NOITEMS | M_CAPTURE | M_REGEN | M_TEAM},
        { "ctf", M_CTF | M_TEAM},
        { "insta ctf", M_NOITEMS | M_INSTA | M_CTF | M_TEAM},
        { "protect", M_CTF | M_PROTECT | M_TEAM},
        { "insta protect", M_NOITEMS | M_INSTA | M_CTF | M_PROTECT | M_TEAM},
        { "hold", M_CTF | M_HOLD | M_TEAM},
        { "insta hold", M_NOITEMS | M_INSTA | M_CTF | M_HOLD | M_TEAM},
        { "effic ctf", M_NOITEMS | M_EFFICIENCY | M_CTF | M_TEAM},
        { "effic protect", M_NOITEMS | M_EFFICIENCY | M_CTF | M_PROTECT | M_TEAM},
        { "effic hold", M_NOITEMS | M_EFFICIENCY | M_CTF | M_HOLD | M_TEAM},
        { "collect", M_COLLECT | M_TEAM},
        { "insta collect", M_NOITEMS | M_INSTA | M_COLLECT | M_TEAM},
        { "effic collect", M_NOITEMS | M_EFFICIENCY | M_COLLECT | M_TEAM},
        { "bomberman", M_LMS | M_BOMB | M_OBSTACLES},
        { "bomberman team", M_LMS | M_BOMB | M_TEAM | M_OBSTACLES},
        { "hideandseek"},
    };


    /// game mode validation and attribute handling

    /// the first 3 game modes are not used in multiplayer
    #define STARTGAMEMODE (-3)

    /// macro to determine the amount of available game modes
    /// division: (size of array) / (size of one gamemodeinfo instance)
    #define NUMGAMEMODES ((int)(sizeof(gamemodes)/sizeof(gamemodes[0])))

    /// validate game mode number (array index)
    #define m_valid(mode)          ((mode) >= STARTGAMEMODE && (mode) < STARTGAMEMODE + NUMGAMEMODES)
    /// validate game mode number and attribute (to check if this gamemode has items or bases e.g.)
    #define m_check(mode, flag)    (m_valid(mode) && gamemodes[(mode) - STARTGAMEMODE].flags&(flag))
    /// validate game mode number and check if game mode does NOT have these attribuges
    #define m_checknot(mode, flag) (m_valid(mode) && !(gamemodes[(mode) - STARTGAMEMODE].flags&(flag)))
    /// validate game mode number and check if game mode supports parameter flag bit masks
    /// to check if this game mode supports multiple attributes (EFFICIENCY | CTF  e.g.)
    #define m_checkall(mode, flag) (m_valid(mode) && (gamemodes[(mode) - STARTGAMEMODE].flags&(flag)) == (flag))




    /// those game mode check macros are built on top of the layer above
    #define m_noitems      (m_check(gamemode, M_NOITEMS))
    #define m_noammo       (m_check(gamemode, M_NOAMMO|M_NOITEMS))
    #define m_insta        (m_check(gamemode, M_INSTA))
    #define m_tactics      (m_check(gamemode, M_TACTICS))
    #define m_efficiency   (m_check(gamemode, M_EFFICIENCY))
    #define m_capture      (m_check(gamemode, M_CAPTURE))
    #define m_regencapture (m_checkall(gamemode, M_CAPTURE | M_REGEN))
    #define m_ctf          (m_check(gamemode, M_CTF))
    #define m_protect      (m_checkall(gamemode, M_CTF | M_PROTECT))
    #define m_hold         (m_checkall(gamemode, M_CTF | M_HOLD))
    #define m_collect      (m_check(gamemode, M_COLLECT))
    #define m_teammode     (m_check(gamemode, M_TEAM))
    #define m_overtime     (m_check(gamemode, M_OVERTIME))
    #define isteam(a,b)    (m_teammode && strcmp(a, b)==0)

    #define m_lms          (m_check(gamemode, M_LMS))
    #define m_bomb         (m_check(gamemode, M_BOMB))
    #define m_hideandseek  (m_check(gamemode, M_HIDEANDSEEK))

    #define m_obstacles    (m_check(gamemode, M_OBSTACLES))
    #define m_timeforward  (m_check(gamemode, M_TIMEFORWARD))

    #define m_demo         (m_check(gamemode, M_DEMO))
    #define m_edit         (m_check(gamemode, M_EDIT))
    #define m_lobby        (m_check(gamemode, M_LOBBY))
    #define m_timed        (m_checknot(gamemode, M_DEMO|M_EDIT|M_LOCAL))
    #define m_botmode      (m_checknot(gamemode, M_DEMO|M_LOCAL))
    #define m_mp(mode)     (m_checknot(mode, M_LOCAL))

    #define m_sp           (m_check(gamemode, M_DMSP | M_CLASSICSP))
    #define m_dmsp         (m_check(gamemode, M_DMSP))
    #define m_classicsp    (m_check(gamemode, M_CLASSICSP))

};
};