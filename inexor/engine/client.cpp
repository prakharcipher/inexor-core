// fpsgame/client.cpp 
// client.cpp, mostly network related client game code
// implementation of connect and disconnect
// implementation of enet network parser

#include <stdarg.h>                                   // for va_end, va_list
#include <string.h>                                   // for strcmp
#include <memory>                                     // for __shared_ptr

#include "enet/enet.h"                                // for ENetEvent, enet...
#include "inexor/engine/engine.hpp"                   // for resolverwait
#include "inexor/io/Logging.hpp"                      // for Log, Logger
#include "inexor/network/SharedVar.hpp"               // for SharedVar
#include "inexor/network/legacy/buffer_types.hpp"     // for packetbuf
#include "inexor/network/legacy/cube_network.hpp"     // for make_file_packet
#include "inexor/network/legacy/game_types.hpp"       // for disconnectreason
#include "inexor/shared/command.hpp"                  // for ICOMMAND, intret
#include "inexor/shared/cube_tools.hpp"               // for ASSERT
#include "inexor/shared/cube_types.hpp"               // for string
#include "inexor/shared/igame.hpp"                    // for connectattempt
#include "inexor/ui/legacy/menus.hpp"                 // for mainmenu
#include "inexor/util/legacy_time.hpp"                // for totalmillis

struct stream;

using namespace inexor::util; //needed for quoted()

// (mostly) network related stuff

ENetHost *clienthost = nullptr;
ENetPeer *curpeer = nullptr, *connpeer = nullptr;
int connmillis = 0, connattempts = 0, discmillis = 0;

// is player in multiplayer
// also print multiplayer restricted game function warnings
bool multiplayer(bool msg)
{
    bool val = curpeer; 
    if(val && msg) Log.std->error("operation not available in multiplayer");
    return val;
}

// set network bandwidth rate (in kilobytes)
void setrate(int rate)
{
   if(!curpeer) return;
   enet_host_bandwidth_limit(clienthost, rate*1024, rate*1024);
}
VARF(rate, 0, 0, 1024, setrate(rate));

// forward of network throttle
void throttle();
VARF(throttle_interval, 0, 5, 30, throttle());
VARF(throttle_accel,    0, 2, 32, throttle());
VARF(throttle_decel,    0, 2, 32, throttle());

// implementation fo network throttle
void throttle()
{
    if(!curpeer) return;
    ASSERT(ENET_PEER_PACKET_THROTTLE_SCALE==32);
    enet_peer_throttle_configure(curpeer, throttle_interval*1000, throttle_accel, throttle_decel);
}

// is game connected or trying to connect
bool isconnected(bool attempt, bool local)
{
    return curpeer || (attempt && connpeer);
}
ICOMMAND(isconnected, "b", (int *attempt), intret(isconnected(*attempt > 0) ? 1 : 0));

// return the current network address
const ENetAddress *connectedpeer()
{
    return curpeer ? &curpeer->address : nullptr;
}

// return the ip of the current server
ICOMMAND(connectedip, "", (),
{
    const ENetAddress *address = connectedpeer();
    string hostname;
    result(address && enet_address_get_host_ip(address, hostname, sizeof(hostname)) >= 0 ? hostname : "");
});

// return the port of the current server
ICOMMAND(connectedport, "", (),
{
    const ENetAddress *address = connectedpeer();
    intret(address ? address->port : -1);
});

// servername and connection port (?)
SVARP(connectname, "");
VARP(connectport, 0, 0, 0xFFFF);

// abort attempt to connect
void abortconnect()
{
    if(!connpeer) return;
    game::connectfail();
    if(connpeer->state!=ENET_PEER_STATE_DISCONNECTED) enet_peer_reset(connpeer);
    connpeer = nullptr;
    if(curpeer) return;
    enet_host_destroy(clienthost);
    clienthost = nullptr;
}

// connect to a server (serverpassword only for mastermode 3 servers)
void connectserv(const char *servername, int serverport, const char *serverpassword, const char *mapwish, int modewish)
{   
    if(connpeer)
    {
        Log.std->info("aborting connection attempt");
        abortconnect();
    }

    if(serverport <= 0) serverport = server_port();

    ENetAddress address;
    address.port = serverport;

    if(servername)
    {
        if(strcmp(servername, connectname)) setsvar("connectname", servername);
        if(serverport != connectport) setvar("connectport", serverport);
        Log.std->info("attempting to connect to {0}:{1}", servername, serverport);
        if(!resolverwait(servername, &address))
        {
            Log.std->error("could not resolve server {0}", servername);
            return;
        }
    }
    else
    {
        setsvar("connectname", "");
        setvar("connectport", 0);
        Log.std->info("attempting to connect over LAN");
        address.host = ENET_HOST_BROADCAST;
    }

    if(!clienthost) 
    {
        clienthost = enet_host_create(nullptr, 2, NUM_ENET_CHANNELS, rate*1024, rate*1024);
        if(!clienthost)
        {
            Log.std->error("could not connect to server");
            return;
        }
        clienthost->duplicatePeers = 0;
    }

    connpeer = enet_host_connect(clienthost, &address, NUM_ENET_CHANNELS, 0);
    enet_host_flush(clienthost);
    connmillis = totalmillis;
    connattempts = 0;

    game::connectattempt(mapwish ? mapwish : "", modewish, serverpassword ? serverpassword : "");
}

// use stored data of last connected server to connect again
void reconnect(const char *serverpassword)
{
    if(!connectname[0] || connectport <= 0)
    {
        Log.std->error("no previous connection");
        return;
    }
    connectserv(connectname, connectport, serverpassword);
}

// disconnect from a server
void disconnect(bool async, bool cleanup)
{
    if(curpeer) 
    {
        if(!discmillis)
        {
            enet_peer_disconnect(curpeer, DISC_NONE);
            enet_host_flush(clienthost);
            discmillis = totalmillis;
        }
        if(curpeer->state!=ENET_PEER_STATE_DISCONNECTED)
        {
            if(async) return;
            enet_peer_reset(curpeer);
        }
        curpeer = nullptr;
        discmillis = 0;
        Log.std->info("disconnected");
        game::gamedisconnect(cleanup);
        mainmenu = 1;
        // inexor::ui::cef_app->GetUserInterface()->SetMainMenu(true);
    }
    if(!connpeer && clienthost)
    {
        enet_host_destroy(clienthost);
        clienthost = nullptr;
    }
}

// try to disconnect (abort connect try or disconnect)
void trydisconnect()
{
    if(connpeer)
    {
        Log.std->info("aborting connection attempt");
        abortconnect();
    } else if(curpeer)
    {
        Log.std->info("attempting to disconnect...");
        disconnect(!discmillis);// try to disconnect synchronously for a while then disconnect asynchronously
    } else Log.std->info("not connected");
}

// commands to establish and destroy network connections
ICOMMAND(connect, "sis", (char *name, int *port, char *pw), connectserv(name, *port, pw));
ICOMMAND(lanconnect, "is", (int *port, char *pw), connectserv(nullptr, *port, pw));

COMMAND(reconnect, "s");
ICOMMAND(disconnect, "", (), trydisconnect());

// send network packet to server
void sendclientpacket(ENetPacket *packet, int chan)
{
    if(curpeer) enet_peer_send(curpeer, chan, packet);
}

// empty network message queue (?)
void flushclient()
{
    if(clienthost) enet_host_flush(clienthost);
}

// print illegal network message to console (wrong protocol?)
void neterr(const char *s, bool disc)
{
    Log.std->error("illegal network message \"{0}\"", s);
    if(disc) disconnect();
}

// send ping to server (?)
void clientkeepalive()
{
    if(clienthost) enet_host_service(clienthost, nullptr, 0);
}

/// Send a file to all other clients.
ENetPacket *send_file(stream *file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    ENetPacket *packet = make_file_packet(file, format, args);
    va_end(args);

    sendclientpacket(packet, CHAN_FILE);
    return packet->referenceCount > 0 ? packet : nullptr;
}

// get updates from the server
void gets2c()
{
    ENetEvent event;
    if(!clienthost) return;
    if(connpeer && totalmillis/3000 > connmillis/3000)
    {
        Log.std->info("attempting to connect...");
        connmillis = totalmillis;
        ++connattempts;
        if(connattempts > 3)
        {
            Log.std->error("could not connect to server");
            abortconnect();
            return;
        }
    }
    while(clienthost && enet_host_service(clienthost, &event, 0)>0)
    switch(event.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
            disconnect(false, false);
            curpeer = connpeer;
            connpeer = nullptr;
            Log.std->info("connected to server");
            throttle();
            if(rate) setrate(rate);
            game::gameconnect();
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if(discmillis) Log.std->info("attempting to disconnect...");
            else {
                // processes any updates from the server
                packetbuf p(event.packet);
                game::parsepacketclient(event.channelID, p);
            }
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            if(event.data>=DISC_NUM) event.data = DISC_NONE;
            if(event.peer==connpeer)
            {
                Log.std->error("could not connect to server");
                abortconnect();
            }
            else
            {
                if(!discmillis || event.data)
                {
                    const char *msg = disconnectreason(event.data);
                    if(msg) Log.std->error("server network error, disconnecting ({0}) ...", msg);
                    else Log.std->error("server network error, disconnecting...");
                }
                disconnect();
            }
            return;

        default:
            break;
    }
}
