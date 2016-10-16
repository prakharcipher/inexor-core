#pragma once

namespace inexor {
namespace server {

    #define DMF 16.0f
    #define DNF 100.0f
    #define DVELF 1.0f

    /// constant protocol and version definitions
    #define INEXOR_SERVINFO_PORT 31413 /// will probably be merged with the server port
    #define INEXOR_LANINFO_PORT 31414
    #define INEXOR_SERVER_PORT 31415
    #define INEXOR_MASTER_PORT 31416

    #define PROTOCOL_VERSION 303            // bump when protocol changes last sauerbraten protocol was 259
    #define DEMO_VERSION 1                  // bump when demo format changes
    #define DEMO_MAGIC "INEXOR_DEMO"
        
    /// important teamspecific declarations
    #define MAXTEAMS 128
    #define MAXNAMELEN 15  /// max player name length
    #define MAXTEAMLEN 4   /// max team name length
    #define MAXTAGLEN 8    /// max player tag length
    #define BOTTAG "Bot"   /// all bots share this tag

    /// Bomberman constants
    #define MAXRAYS 20
    #define EXP_SELFDAMDIV 2
    #define EXP_SELFPUSH 2.5f
    #define EXP_DISTSCALE 1.5f
    #define BOMB_DAMRAD 20

    /// teams
    #define MAXNAMELEN 15
    #define MAXTEAMLEN 4
    #define MAXTEAMS 128

};
};
