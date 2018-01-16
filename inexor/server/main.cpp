// server.cpp: little more than enhanced multicaster

#include <limits.h>                                   // for INT_MAX
#include <stdio.h>                                    // for fprintf, stderr
#include <stdlib.h>                                   // for exit, atexit
#include <algorithm>                                  // for max, min
#include <memory>                                     // for __shared_ptr
#include <string>                                     // for basic_string
#include <vector>                                     // for vector

#include "enet/enet.h"                                // for enet_socket_des...
#include "enet/types.h"                               // for enet_uint16
#include "enet/unix.h"                                // for ENET_SOCKET_NULL
#include "inexor/crashreporter/CrashReporter.hpp"     // for CrashReporter
#include "inexor/io/Logging.hpp"                      // for Log, log_manager
#include "inexor/io/Error.hpp"                      // for fatal
#include "inexor/network/SharedVar.hpp"               // for SharedVar
#include "inexor/network/legacy/buffer_types.hpp"     // for ucharbuf, packe...
#include "inexor/network/legacy/cube_network.hpp"     // for MAXCLIENTS, MAX...
#include "inexor/network/legacy/game_types.hpp"       // for server_port
#include "inexor/server/client_management.hpp"        // for client, disconn...
#include "inexor/shared/command.hpp"                  // for execfile, SVAR
#include "inexor/shared/cube_formatting.hpp"          // for defvformatstring
#include "inexor/shared/cube_loops.hpp"               // for i, loopi
#include "inexor/shared/cube_tools.hpp"               // for copystring, UNUSED
#include "inexor/shared/cube_types.hpp"               // for string, uchar
#include "inexor/shared/igame.hpp"                    // for sendpackets
#include "inexor/shared/tools.hpp"                    // for max, min
#include "inexor/util/StringFormatter.hpp"            // for StringFormatter
#include "inexor/util/Subsystem.hpp"                  // for Metasystem, SUB...
#include "inexor/util/legacy_time.hpp"                // for updatetime, tot...

const char *initscript = nullptr;

void conline(int type, const char *sf) {};

inexor::util::Metasystem metapp;

namespace server {
void cleanupserver();
}
// Called from fatal()
void cleanup_application()
{
    server::cleanupserver();
}

namespace server {

ENetHost *serverhost = nullptr;
int laststatus = 0;
ENetSocket pongsock = ENET_SOCKET_NULL, lansock = ENET_SOCKET_NULL;


void cleanupserver()
{
    if(serverhost) enet_host_destroy(serverhost);
    serverhost = nullptr;

    if(pongsock != ENET_SOCKET_NULL) enet_socket_destroy(pongsock);
    if(lansock != ENET_SOCKET_NULL) enet_socket_destroy(lansock);
    pongsock = lansock = ENET_SOCKET_NULL;
    metapp.stop("rpc");
}

void process(ENetPacket *packet, int sender, int chan);

int getservermtu() { return serverhost ? serverhost->mtu : -1; }

void process(ENetPacket *packet, int sender, int chan)   // sender may be -1
{
    packetbuf p(packet);
    server::parsepacket(sender, chan, p);
    if(p.overread()) { disconnect_client(sender, DISC_EOP); return; }
}

bool resolverwait(const char *name, ENetAddress *address)
{
    return enet_address_set_host(address, name) >= 0;
}

int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &remoteaddress)
{
    return enet_socket_connect(sock, &remoteaddress);
}

ENetAddress serveraddress = { ENET_HOST_ANY, ENET_PORT_ANY };

static ENetAddress pongaddr;

void sendserverinforeply(ucharbuf &p)
{
    ENetBuffer buf;
    buf.data = p.buf;
    buf.dataLength = p.length();
    enet_socket_send(pongsock, &pongaddr, &buf, 1);
}

#define MAXPINGDATA 32

/// Reply all server info requests
void checkserversockets()
{
    static ENetSocketSet readset, writeset;
    ENET_SOCKETSET_EMPTY(readset);
    ENET_SOCKETSET_EMPTY(writeset);
    ENetSocket maxsock = pongsock;
    ENET_SOCKETSET_ADD(readset, pongsock);

    if(lansock != ENET_SOCKET_NULL)
    {
        maxsock = max(maxsock, lansock);
        ENET_SOCKETSET_ADD(readset, lansock);
    }
    if(enet_socketset_select(maxsock, &readset, &writeset, 0) <= 0) return;

    ENetBuffer buf;
    uchar pong[MAXTRANS];
    loopi(2)
    {
        ENetSocket sock = i ? lansock : pongsock;
        if(sock == ENET_SOCKET_NULL || !ENET_SOCKETSET_CHECK(readset, sock)) continue;

        buf.data = pong;
        buf.dataLength = sizeof(pong);
        int len = enet_socket_receive(sock, &pongaddr, &buf, 1);
        if(len < 0 || len > MAXPINGDATA) continue;
        ucharbuf req(pong, len), p(pong, sizeof(pong));
        p.len += len;
        server::serverinforeply(req, p);
    }
}

VAR(serveruprate, 0, 0, INT_MAX);
SVAR(serverip, "");
VARF(serverport, 0, INEXOR_SERVER_PORT, MAX_POSSIBLE_PORT, { if(!serverport) serverport = server_port(); });


/// main server update
void serverslice(uint timeout)
{
    if(!serverhost)
    {
        server::serverupdate();
        server::sendpackets();
        return;
    }

    // below is network only

    metapp.tick();
    updatetime(server::ispaused(), server::gamespeed);
    server::serverupdate();

    checkserversockets();

    if(totalmillis-laststatus>60*1000)   // display bandwidth stats, useful for server ops
    {
        laststatus = totalmillis;
        if(has_clients() || serverhost->totalSentData || serverhost->totalReceivedData)
            Log.std->info("status: {0} remote clients, {1} send, {2} rec (K/sec)",
                                         get_num_clients(), (serverhost->totalSentData/60.0f/1024), (serverhost->totalReceivedData/60.0f/1024));
        serverhost->totalSentData = serverhost->totalReceivedData = 0;
    }

    ENetEvent event;
    bool serviced = false;
    while(!serviced)
    {
        if(enet_host_check_events(serverhost, &event) <= 0)
        {
            if(enet_host_service(serverhost, &event, timeout) <= 0) break;
            serviced = true;
        }
        switch(event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
            {
                client &c = add_client_connection(event.peer);
                string hn;
                copystring(c.hostname, (enet_address_get_host_ip(&c.peer->address, hn, sizeof(hn))==0) ? hn : "unknown");
                Log.std->info("client connected ({0})", c.hostname);
                int reason = server::clientconnect(c.num, c.peer->address.host);
                if(reason) disconnect_client(c.num, reason);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                client *c = (client *)event.peer->data;
                if(c) process(event.packet, c->num, event.channelID);
                if(event.packet->referenceCount==0) enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                client *c = (client *)event.peer->data;
                if(!c) break;
                Log.std->info("disconnected client ({0})", c->hostname);
                disconnect_client(c->num, DISC_NONE);
                break;
            }
            default:
                break;
        }
    }
    if(server::sendpackets()) enet_host_flush(serverhost);
}

void flushserver(bool force)
{
    if(server::sendpackets(force) && serverhost) enet_host_flush(serverhost);
}

void run_server()
{
    Log.std->info("dedicated server started, waiting for clients...");
#ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    for(;;)
    {
        MSG msg;
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT) exit(EXIT_SUCCESS);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        serverslice(5);
    }
#else
    for(;;) serverslice(5);
#endif
    }


bool servererror(const char *desc)
{
    fatal("%s", desc);
    return false;
}

bool setup_network_sockets()
{
    ENetAddress address ={ENET_HOST_ANY, enet_uint16(serverport <= 0 ? server_port() : serverport)};
    if(serverip[0])
    {
        if(enet_address_set_host(&address, serverip)<0) Log.std->warn("WARNING: server ip not resolved ({})", serverip);
        else serveraddress.host = address.host;
    }
    serverhost = enet_host_create(&address, min(maxclients + reserveclients(), MAXCLIENTS), NUM_ENET_CHANNELS, 0, serveruprate);
    if(!serverhost) return servererror("could not create servenet_address_set_hoster host");
    serverhost->duplicatePeers = maxdupclients ? maxdupclients : MAXCLIENTS;
    address.port = server_info_port(serverport > 0 ? serverport : -1);
    pongsock = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    if(pongsock != ENET_SOCKET_NULL && enet_socket_bind(pongsock, &address) < 0)
    {
        enet_socket_destroy(pongsock);
        pongsock = ENET_SOCKET_NULL;
    }
    if(pongsock == ENET_SOCKET_NULL) return servererror("could not create server info socket");
    else enet_socket_set_option(pongsock, ENET_SOCKOPT_NONBLOCK, 1);
    address.port = lan_info_port();
    lansock = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    if(lansock != ENET_SOCKET_NULL && (enet_socket_set_option(lansock, ENET_SOCKOPT_REUSEADDR, 1) < 0 || enet_socket_bind(lansock, &address) < 0))
    {
        enet_socket_destroy(lansock);
        lansock = ENET_SOCKET_NULL;
    }
    if(lansock == ENET_SOCKET_NULL) Log.std->warn("WARNING: could not create LAN server info socket");
    else enet_socket_set_option(lansock, ENET_SOCKOPT_NONBLOCK, 1);
    return true;
}
} // ns server

using namespace server;

int main(int argc, char **argv)
{
    UNUSED inexor::crashreporter::CrashReporter SingletonStackwalker; // We only need to initialize it, not use it.
    char *exe_name = argv[0];
    Log.logfile = exe_name;
    // Initialize the metasystem
    // Remote Procedure Call: communication with the scripting engine
    SUBSYSTEM_REQUIRE(rpc);

    metapp.start("rpc");
    metapp.initialize("rpc", argc, argv);

    if(enet_initialize()<0) fatal("Unable to initialise network module");
    atexit(enet_deinitialize);
    enet_time_set(0);

    serverinit();

    if(initscript) execfile(initscript);
    else execfile("server-init.cfg", false);

    setup_network_sockets();

    run_server(); // never returns
    return EXIT_SUCCESS;
}
