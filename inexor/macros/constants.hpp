#pragma once

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

    /// constant protocol and version definitions
    #define INEXOR_SERVINFO_PORT 31413 /// will probably be merged with the server port
    #define INEXOR_LANINFO_PORT 31414
    #define INEXOR_SERVER_PORT 31415
    #define INEXOR_MASTER_PORT 31416

    #define PROTOCOL_VERSION 303            // bump when protocol changes last sauerbraten protocol was 259
    #define DEMO_VERSION 1                  // bump when demo format changes
    #define DEMO_MAGIC "INEXOR_DEMO"
    
    // some more (precise) mathematical constants
    #ifdef WIN32
    #ifndef M_PI
    #define M_PI 3.14159265358979323846
    #endif
    #ifndef M_LN2
    #define M_LN2 0.693147180559945309417
    #endif
    /// Compare Strings, ignore case.
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
    /// Path divide character, \ on win, otherwise /.
    #define PATHDIV '\\'
    #else
    // adapt macros to OS specifications
    #define __cdecl
    #define _vsnprintf vsnprintf
    /// Path divide character, \ on win, otherwise /.
    #define PATHDIV '/'
    #endif

    #define PI  (3.1415927f)
    #define PI2 (2*PI)
    #define SQRT2 (1.4142136f)
    #define SQRT3 (1.7320508f)
    #define RAD (PI / 180.0f)

};
};
