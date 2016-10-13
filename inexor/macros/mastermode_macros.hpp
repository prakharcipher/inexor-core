#pragma once

namespace inexor {
namespace server {

    /// mastermode macros
    #define MM_MODE 0xF
    #define MM_AUTOAPPROVE 0x1000
    #define MM_PRIVSERV (MM_MODE | MM_AUTOAPPROVE)
    #define MM_PUBSERV ((1<<MM_OPEN) | (1<<MM_VETO))
    #define MM_COOPSERV (MM_AUTOAPPROVE | MM_PUBSERV | (1<<MM_LOCKED))


    static const char * const mastermodenames[] =  { "auth",   "open",   "veto",       "locked",     "private",    "password" };
    static const char * const mastermodecolors[] = { "",    COL_GREEN,  COL_YELLOW,   COL_YELLOW,     COL_RED,    COL_RED};
    static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };
};
};
