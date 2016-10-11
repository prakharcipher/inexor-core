#pragma once

#include "inexor/enumerations/enum_gamemode_bitmask.hpp"
#include "inexor/server/server_gamemodes.hpp"

namespace inexor {
namespace server {

    /// 
    extern int gamemode;

    /// 
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
