#pragma once

#include <enet/enet.h>                             // for ENetPacket
#include <stdarg.h>                                // for va_list
#include <stddef.h>                                // for size_t
#include <algorithm>

#include "enet/types.h"                            // for enet_uint32
#include "inexor/io/legacy/stream.hpp"
#include "inexor/network/legacy/buffer_types.hpp"  // for ucharbuf, packetbu...
#include "inexor/shared/cube_tools.hpp"
#include "inexor/shared/cube_types.hpp"            // for uchar
#include "inexor/shared/cube_vector.hpp"           // for vector

struct stream;

#define MAXCLIENTS 128                 // DO NOT set this any higher
#define MAXTRANS 5000                  // max amount of data to swallow in 1 go



/// network quantization scale
#define DMF 16.0f   /// for world locations
#define DNF 100.0f  /// for normalized vectors
#define DVELF 1.0f  /// for playerspeed based velocity vectors

extern void putint(ucharbuf &p, int n);
extern void putint(packetbuf &p, int n);
extern void putint(vector<uchar> &p, int n);
extern int getint(ucharbuf &p);
extern void putuint(ucharbuf &p, int n);
extern void putuint(packetbuf &p, int n);
extern void putuint(vector<uchar> &p, int n);
extern int getuint(ucharbuf &p);
extern void putfloat(ucharbuf &p, float f);
extern void putfloat(packetbuf &p, float f);
extern void putfloat(vector<uchar> &p, float f);
extern float getfloat(ucharbuf &p);
extern void sendstring(const char *t, ucharbuf &p);
extern void sendstring(const char *t, packetbuf &p);
extern void sendstring(const char *t, vector<uchar> &p);
extern void getstring(char *t, ucharbuf &p, size_t len);
template<size_t N> static inline void getstring(char(&t)[N], ucharbuf &p) { getstring(t, p, N); }
extern void filtertext(char *dst, const char *src, bool whitespace, bool forcespace, size_t len);
template<size_t N> static inline void filtertext(char(&dst)[N], const char *src, bool whitespace = true, bool forcespace = false) { filtertext(dst, src, whitespace, forcespace, N-1); }

/// structure to describe IPs
struct ipmask
{
    enet_uint32 ip, mask;

    void parse(const char *name);
    int print(char *buf) const;
    bool check(enet_uint32 host) const { return (host & mask) == ip; }
};

/// Puts a file into a ENet packet.
/// args is just a forward of "...", see C argument forwarding.
extern ENetPacket *make_file_packet(stream *file, const char *format, va_list args);

