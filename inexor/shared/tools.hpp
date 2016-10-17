#pragma once

#include <new>
#include <cstddef>
#include <algorithm>

#include <boost/algorithm/clamp.hpp>

#include "inexor/util/random.hpp"
#include "inexor/util.hpp"

#include "inexor/deprecated/type_definitions.hpp"


#ifdef _DEBUG //TODO remove
  #define ASSERT(c) assert(c)
#else
  #define ASSERT(c) if(c) {}
#endif

#if defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER >= 1400)
  #define RESTRICT __restrict
#else
  #define RESTRICT
#endif

#ifdef __GNUC__
  #define UNUSED __attribute__((unused)) //make sure unused code won't get optimized away.
#else
  #define UNUSED
#endif

using std::swap;
using std::min;
using std::max;

using boost::algorithm::clamp;

#ifdef __GNUC__
#define bitscan(mask) (__builtin_ffs(mask)-1)
#else
#ifdef WIN32
#pragma intrinsic(_BitScanForward)
static inline int bitscan(uint mask)
{
    ulong i;
    return _BitScanForward(&i, mask) ? i : -1;
}
#else
static inline int bitscan(uint mask)
{
    if(!mask) return -1;
    int i = 1;
    if(!(mask&0xFFFF)) { i += 16; mask >>= 16; }
    if(!(mask&0xFF)) { i += 8; mask >>= 8; }
    if(!(mask&0xF)) { i += 4; mask >>= 4; }
    if(!(mask&3)) { i += 2; mask >>= 2; }
    return i - (mask&1);
}
#endif
#endif

INEXOR_FUNCTION_ALIAS(rnd, inexor::util::rnd<int>);
INEXOR_FUNCTION_ALIAS(rndscale, inexor::util::rnd<float>);
INEXOR_FUNCTION_ALIAS(detrnd, inexor::util::deterministic_rnd<int>);

/// DEPRECATED: Do not use loop macros at all!
#include "inexor/macros/loop_macros.hpp"

#include "inexor/deprecated/memfree.hpp"

#include "inexor/deprecated/math_constants.hpp"

/// DEPRECATED: use std::string instead!
#include "inexor/deprecated/string.hpp"

#include "inexor/templates/databuf.hpp"

#include "inexor/classes/packetbuf.hpp"

#include "inexor/templates/heapscore.hpp"

#include "inexor/classes/sortless.hpp"

#include "inexor/classes/sortnameless.hpp"

/// DEPRECATED: use std::sort instead!
#include "inexor/templates/insertionsort.hpp"
#include "inexor/templates/quicksort.hpp"

#include "inexor/deprecated/isclass.hpp"

/// DEPRECATED: use std::map instead!
#include "inexor/deprecated/hashset.hpp"

/// DEPRECATED: use std::vector instead!
#include "inexor/deprecated/vector.hpp"

/// DEPRECATED: Use std::deque instead!
#include "inexor/deprecated/queue.hpp"

#include "inexor/deprecated/byteswap.hpp"

#include "inexor/deprecated/stream.hpp"

/// bitmask for text formatting (?)
enum
{
    CT_PRINT   = 1<<0,
    CT_SPACE   = 1<<1,
    CT_DIGIT   = 1<<2,
    CT_ALPHA   = 1<<3,
    CT_LOWER   = 1<<4,
    CT_UPPER   = 1<<5,
    CT_UNICODE = 1<<6
};


extern const uchar cubectype[256];
static inline int iscubeprint(uchar c) { return cubectype[c]&CT_PRINT; }
static inline int iscubespace(uchar c) { return cubectype[c]&CT_SPACE; }
static inline int iscubealpha(uchar c) { return cubectype[c]&CT_ALPHA; }
static inline int iscubealnum(uchar c) { return cubectype[c]&(CT_ALPHA|CT_DIGIT); }
static inline int iscubelower(uchar c) { return cubectype[c]&CT_LOWER; }
static inline int iscubeupper(uchar c) { return cubectype[c]&CT_UPPER; }
static inline int cube2uni(uchar c)
{ 
    extern const int cube2unichars[256]; 
    return cube2unichars[c]; 
}
static inline uchar uni2cube(int c)
{
    extern const int uni2cubeoffsets[8];
    extern const uchar uni2cubechars[];
    return uint(c) <= 0x7FF ? uni2cubechars[uni2cubeoffsets[c>>8] + (c&0xFF)] : 0;
}
static inline uchar cubelower(uchar c)
{
    extern const uchar cubelowerchars[256];
    return cubelowerchars[c];
}
static inline uchar cubeupper(uchar c)
{
    extern const uchar cubeupperchars[256];
    return cubeupperchars[c];
}

/// UTF-8 encoder/decoder function prototypes
extern size_t decodeutf8(uchar *dst, size_t dstlen, const uchar *src, size_t srclen, size_t *carry = NULL);
extern size_t encodeutf8(uchar *dstbuf, size_t dstlen, const uchar *srcbuf, size_t srclen, size_t *carry = NULL);

extern char *makerelpath(const char *dir, const char *file, const char *prefix = NULL, const char *cmd = NULL);
extern char *path(char *s);
extern char *path(const char *s, bool copy);
extern const char *parentdir(const char *directory);
extern bool fileexists(const char *path, const char *mode);
extern bool createdir(const char *path);
extern size_t fixpackagedir(char *dir);
extern const char *sethomedir(const char *dir);
extern const char *addpackagedir(const char *dir);
extern const char *findfile(const char *filename, const char *mode);
extern bool findzipfile(const char *filename);
extern stream *openrawfile(const char *filename, const char *mode);
extern stream *openzipfile(const char *filename, const char *mode);
extern stream *openfile(const char *filename, const char *mode);
extern stream *opentempfile(const char *filename, const char *mode);
extern stream *opengzfile(const char *filename, const char *mode, stream *file = NULL, int level = Z_BEST_COMPRESSION);
extern stream *openutf8file(const char *filename, const char *mode, stream *file = NULL);
extern char *loadfile(const char *fn, size_t *size, bool utf8 = true);
extern bool listdir(const char *dir, bool rel, const char *ext, vector<char *> &files);
extern int listfiles(const char *dir, const char *ext, vector<char *> &files);
extern int listzipfiles(const char *dir, const char *ext, vector<char *> &files);

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
template<size_t N> static inline void getstring(char (&t)[N], ucharbuf &p) { getstring(t, p, N); }
extern void filtertext(char *dst, const char *src, bool whitespace, bool forcespace, size_t len);
template<size_t N> static inline void filtertext(char (&dst)[N], const char *src, bool whitespace = true, bool forcespace = false) { filtertext(dst, src, whitespace, forcespace, N-1); }

/// structure to describe IPs
struct ipmask
{
    enet_uint32 ip, mask;

    void parse(const char *name);
    int print(char *buf) const;
    bool check(enet_uint32 host) const { return (host & mask) == ip; }
};
