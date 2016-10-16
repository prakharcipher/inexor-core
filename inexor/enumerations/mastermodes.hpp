#pragma once

/// master modes
enum 
{ 
    MM_AUTH = -1,
    MM_OPEN = 0,
    MM_VETO,
    MM_LOCKED, 
    MM_PRIVATE, 
    MM_PASSWORD, 
    MM_START = MM_AUTH
};

/// static strings for server description in master server list
static const char * const mastermodenames[] =  { "auth",   "open",   "veto",       "locked",     "private",    "password" };
static const char * const mastermodecolors[] = { "",    COL_GREEN,  COL_YELLOW,   COL_YELLOW,     COL_RED,    COL_RED};
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };
