
#include <ctype.h>                                 // for isdigit
#include <stdarg.h>                                // for va_arg, va_end

#include "inexor/network/legacy/buffer_types.hpp"  // for packetbuf
#include "inexor/network/legacy/cube_network.hpp"  // for putint, make_file_...
#include "inexor/network/legacy/game_types.hpp"    // for ::N_SERVMSG
#include "inexor/server/client_management.hpp"     // for client_connections
#include "inexor/server/demos.hpp"                 // for recordpacket
#include "inexor/server/network_send.hpp"
#include "inexor/shared/cube_formatting.hpp"       // for defvformatstring
#include "inexor/shared/cube_loops.hpp"            // for i, loopi, loopv
#include "inexor/shared/cube_types.hpp"            // for uchar
#include "inexor/shared/cube_vector.hpp"           // for vector
#include "inexor/fpsgame/server.hpp"

struct stream;

using namespace server; // TODO move this in there

void sendpacket(int n, int chan, ENetPacket *packet, int exclude)
{
    if(n<0)
    {
        recordpacket(chan, packet->data, packet->dataLength);
        loopv(client_connections) if(i!=exclude && allowbroadcast(i)) sendpacket(i, chan, packet);
        return;
    }
    if(client_connections[n]->connected) enet_peer_send(client_connections[n]->peer, chan, packet);
}

// broadcast if cn = -1
ENetPacket *sendf(int cn, int chan, const char *format, ...)
{
    int exclude = -1;
    bool reliable = false;
    if(*format=='r') { reliable = true; ++format; }
    packetbuf p(MAXTRANS, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    va_list args;
    va_start(args, format);
    while(*format) switch(*format++)
    {
        case 'x':
            exclude = va_arg(args, int);
            break;

        case 'v':
        {
            int n = va_arg(args, int);
            int *v = va_arg(args, int *);
            loopi(n) putint(p, v[i]);
            break;
        }

        case 'i':
        {
            int n = isdigit(*format) ? *format++-'0' : 1;
            loopi(n) putint(p, va_arg(args, int));
            break;
        }
        case 'f':
        {
            int n = isdigit(*format) ? *format++-'0' : 1;
            loopi(n) putfloat(p, (float)va_arg(args, double));
            break;
        }
        case 's': sendstring(va_arg(args, const char *), p); break;
        case 'm':
        {
            int n = va_arg(args, int);
            p.put(va_arg(args, uchar *), n);
            break;
        }
    }
    va_end(args);
    ENetPacket *packet = p.finalize();
    sendpacket(cn, chan, packet, exclude);
    return packet->referenceCount > 0 ? packet : nullptr;
}

void sendservmsg(const char *s)
{
    sendf(-1, 1, "ris", N_SERVMSG, s);
}

void sendservmsgf(const char *fmt, ...)
{
    defvformatstring(s, fmt, fmt);
    sendf(-1, 1, "ris", N_SERVMSG, s);
}

ENetPacket *sendfile(int cn, int chan, stream *file, const char *format, ...)
{
    if(!client_connections.inrange(cn)) return nullptr;

    va_list args;
    va_start(args, format);
    ENetPacket *packet = make_file_packet(file, format, args);
    va_end(args);

    sendpacket(cn, chan, packet, -1);

    return packet->referenceCount > 0 ? packet : nullptr;
}
