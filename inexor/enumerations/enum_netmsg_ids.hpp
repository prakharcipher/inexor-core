#pragma once

namespace inexor {
namespace server {

    enum
    {
        N_CONNECT = 0,          /// C2S      send connection request to server
        N_SERVINFO,             /// S2C      send connection attempt answer (can be denied in case of wrong protocol or server password protection)
        N_WELCOME,              /// S2C      we are now connected. also close my GUI
        N_INITCLIENT,           /// S2C      another client connected or existing client changed name, team, or player model
        N_POS,                  /// C2S|C2S  send player position and rotation
        N_TEXT,                 /// C2S      send chat message to global game chat
        N_SOUND,                /// C2S      send sound signal
        N_CDIS,                 /// S2C      a client disconnected
        N_SHOOT,                /// C2S      a shot was fired
        N_EXPLODE,              /// C2S      an explosion was triggered (grenades, rockets..)
        N_SUICIDE,              /// C2S      I suicided (other clients receive N_DIED where actor = victim)
        N_DIED,                 /// S2C      a player got killed (or suicided)
        N_DAMAGE,               /// S2C      a player got damages
        N_HITPUSH,              /// S2C      a player got pushed back
        N_SHOTFX,               /// S2C      the EFFECT OF A SHOT (see N_SHOT)
        N_EXPLODEFX,            /// S2C      the EFFECT OF AN EXPLOSION (see N_EXPLODE)
        N_TRYSPAWN,             /// C2S      a players tries to spawn
        N_SPAWNSTATE,           /// S2C      send clients spawn information
        N_SPAWN,                /// C2S|S2C  a client is now spawning
        N_FORCEDEATH,           /// S2C      force a client to die
        N_GUNSELECT,            /// C2S      a client selects a weapon
        N_TAUNT,                /// C2S|S2C (?)  a client sent "Im gonna kill you" animation
        N_FOV,                  /// C2S|S2C  a client has changed its field of view.
        N_MAPCHANGE,            /// S2C      server changed map
        N_MAPVOTE,              /// C2S      client suggests a map/mode
        N_TEAMINFO,             /// S2C      send information about teams and their frags
        N_ITEMSPAWN,            /// S2C      an item has spawned
        N_ITEMPICKUP,           /// C2S      I just picked up this item
        N_ITEMACC,              /// S2C      item pickup was acknowledged for a client (item is occupied/despawned. wait for spawn)
        N_ITEMPUSH,             /// C2S      lose items when killed (BOMBERMAN)
        N_TELEPORT,             /// C2S|S2C  a player in game has teleported to a teleport destination
        N_JUMPPAD,              /// C2S|S2C  a player in game has used a jumppad
        N_PING,                 /// C2S      client sends ping packet to server
        N_PONG,                 /// S2C      servers answers ping packet
        N_CLIENTPING,           /// C2S|S2C  send client ping mesaure request / servers sends clients' pings
        N_TIMEUP,               /// S2C      change remaining time
        N_FORCEINTERMISSION,    /// S2C      server ended current game: set game time to 0 ("intermission")
        N_SERVMSG,              /// S2C      server messages can be colored and will be rendered in game console
        N_ITEMLIST,             /// C2S|S2C  client request a list of items available in the current match / server answers with list
        N_RESUME,               /// S2C      resume transmission of new data from lagged client (NOT: RESUMING A PAUSED GAME!)

                                // edit mode specific network messages
        N_EDITMODE,             /// C2S|S2C  a player toggled his edit mode on/off (requires editmode)
        N_EDITENT,              /// S2C      a player creates a new entity (requires editmode)
        N_EDITF,                /// C2S|S2C  a player changed a FACE (requires editmode)
        N_EDITT,                /// C2S|S2C  a player changed a TEXTURE (requires editmode)
        N_EDITM,                /// C2S|S2C  a player edited MATERIAL (requires editmode)
        N_FLIP,                 /// C2S|S2C  a player flipped the current selection (requires editmode)
        N_COPY,                 /// C2S|S2C  (?) a player wants to copy a certain selection (requires editmode)
        N_PASTE,                /// C2S|S2C  (?) send clipboard to other palyers (requires editmode)
        N_ROTATE,               /// C2S|S2C  a player rotated a selection (requires editmode)
        N_REPLACE,              /// C2S|S2C  a player wants to replace a selection (requires editmode)
        N_DELCUBE,              /// C2S|S2C  a player wants to delete a selection (requires editmode)
        N_REMIP,                /// C2S|S2C  a client forcedremip (requires editmode, no administrative levels required)
        N_EDITVSLOT,            /// C2S|S2C  a client used a v.. command (vscale, vcolor..) to modify a texture
        N_UNDO,                 /// C2S|S2C  send undo edit message
        N_REDO,                 /// C2S|S2C  send redo edit message
        N_NEWMAP,               /// C2S|S2C  a client started a new map (requires editmode)
        N_GETMAP,               /// C2S      a client downloaded the current map from server's map buffer (NOT ALWAYS UP TO DATE! MAP MUST BE SENT BEFORE DOWNLOADING!)
        N_SENDMAP,              /// S2C      server sends map to client (requires coop mode. YOU CAN'T SEND MAPS IN INSTACTF e.g. (YET))
        N_CLIPBOARD,            /// C2S      send copied data from your clipboard to server
        N_EDITVAR,              /// C2S|S2C  set map var value (requires editmode)
        N_MASTERMODE,           /// C2S      change master mode (requires permissions)
        N_KICK,                 /// C2S      kick a specific player
        N_CLEARBANS,            /// C2S      clear ban list
        N_CURRENTMASTER,        /// S2C      server sent information about who is the current game master
        N_SPECTATOR,            /// C2S|S2C  toggle spectator status
        N_SETMASTER,            /// C2S      claim game master
        N_SETTEAM,              /// C2S|S2C  team chat

                                // capture mode specific network messages
        N_BASES,                /// S2C      send a list of available bases in capture mode
        N_BASEINFO,             /// S2C      send extended information about bases
        N_BASESCORE,            /// S2C      send base score to client
        N_REPAMMO,              /// S2C      replace ammo around bases in capture mode
        N_BASEREGEN,            /// S2C      regen capture: refill health and ammo of players near captured bases
        N_ANNOUNCE,             /// S2C      announce spawn of quad damage or health boost (normal capture only)

                                // demo specific network messages
        N_LISTDEMOS,            /// C2S      request a list of demos available for download
        N_SENDDEMOLIST,         /// S2C      send a list of demos available for download
        N_GETDEMO,              /// C2S      a client requests to download a demo
        N_SENDDEMO,             /// S2C      server sends a demo
        N_DEMOPLAYBACK,         /// S2C      finish demo playback
        N_RECORDDEMO,           /// C2S      advise the server to record demo
        N_STOPDEMO,             /// C2S      finish demo recording add add recorded material to demo list
        N_CLEARDEMOS,           /// C2S      clear the last n / all demos

                                // ctf/hold specific network messages
        N_TAKEFLAG,             /// S2C      a player took a flag
        N_RETURNFLAG,           /// S2C      a player returned a flag
        N_RESETFLAG,            /// S2C      a flag has been reset
        N_INVISFLAG,            /// S2C      send how long flag will stay transparent in hold mode ? (vistime)
        N_TRYDROPFLAG,          /// C2S      tell the server that you would like to drop your flag
        N_DROPFLAG,             /// S2C      a client has dropped the flag
        N_SCOREFLAG,            /// S2C      a client has scored the flag
        N_INITFLAGS,            /// S2C      send a list of flags available in game

                                // text, auth, bots, options, gamespeed
        N_SAYTEAM,              /// C2S|S2C  team chat
        N_PRIVMSG,              /// C2S|S2C  private/personal message
        N_HUDANNOUNCE,          /// S2C      BOMBERMAN Announcement
        N_CLIENT,               /// S2C      client synchronisation
        N_AUTHTRY,              /// C2S      try to authentificate using auth key(s)
        N_AUTHKICK,             /// C2S      try to authentificate using my auth key(s), kick a specific person [hacker] and then relinquish master again
        N_AUTHCHAL,             /// C2S      authentification challenge
        N_AUTHANS,              /// S2C      authentification response
        N_REQAUTH,              /// S2C      this nick name requires authentification

        N_PAUSEGAME,            /// C2S|S2C  server paused game. stop player movement and actions.
        N_GAMESPEED,            /// C2S|S2C  change game speed (and broadcast that change)
        N_PERSISTTEAMS,         /// C2S|S2C  do not shuffle teams when enabled.

        N_ADDBOT,               /// -S-      add a bot to the current game
        N_DELBOT,               /// -S-      remove a bot from the current game
        N_INITAI,               /// S2C      commit AI settings to server. bots are still server side
        N_FROMAI,               /// C2S      take client number from bot x?
        N_BOTLIMIT,             /// -S-      set the bot limit
        N_BOTBALANCE,           /// -S-      set bot balance
        N_MAPCRC,               /// C2S      send map CRC32 hash value
        N_CHECKMAPS,            /// C2S      force server to check client maps manually (requires permissions)
        N_SWITCHNAME,           /// C2S|S2C  a player has changed his name
        N_SWITCHMODEL,          /// C2S|S2C  a player has changed his player model
        N_SWITCHTEAM,           /// C2S|S2C  a player has switched his team (some game modes have more than 2 teams!)

                                // collect mode messages
        N_INITTOKENS,
        N_TAKETOKEN,
        N_EXPIRETOKENS,
        N_DROPTOKENS,
        N_DEPOSITTOKENS,
        N_STEALTOKENS,

        N_SERVCMD,              /// S2C      servers could send advanced messages to clients. standard clients do not interpret this custom message
        N_DEMOPACKET,           /// S2C      send a requested demo packet
        N_SPAWNLOC,             /// S2C      BOMBERMAN spawn location?
        NUMMSG
    };

};
};
