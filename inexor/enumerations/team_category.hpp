#pragma once

/// team category
enum
{
    TEAM_NONE,
    TEAM_OWN,
    TEAM_OPPONENT,
    TEAM_NUM
};

/// const radar blip colors
static const char * const teamblipcolor[TEAM_NUM] = 
{
    "_neutral", /// = 'gray'
    "_blue",
    "_red"
};
