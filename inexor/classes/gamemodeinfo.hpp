#pragma once


/// structure for game mode description
static struct gamemodeinfo
{
    const char *name; /// game mode's name
    int flags;        /// a bitmask container (see flags above)
};

static gamemodeinfo gamemodes[] =
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
