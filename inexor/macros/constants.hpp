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

    #define M_PI 3.14159265358979323846
    #define M_LN2 0.693147180559945309417

};
};
