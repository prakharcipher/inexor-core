#pragma once

/// ALL TEMPLATE CLASSES IN THIS FILE ARE DEPRECATED!
/// THEY HAVE BEEN CREATED IN A TIME WHEN C++ DID NOT HAVE
/// A PROPER STANDARD TEMPLATE LIBRARY.
/// GET RID OF THESE AND USE ONLY THE C++ STANDARD LIBRARY TEMPLATES !!!

#include <new>
#include <cstddef>
#include <algorithm>
#include <boost/algorithm/clamp.hpp>
#include "inexor/util/random.hpp"
#include "inexor/util.hpp"

/// new server code refactoring separated these dependencies from tools.hpp.
/// the final goal is to replace these with the C++ standard template library.

#include "inexor/macros/loop_macros.hpp"
#include "inexor/macros/memfree_macros.hpp"
#include "inexor/macros/constants.hpp"

#include "inexor/deprecated/type_definitions.hpp"
#include "inexor/deprecated/vector_template.hpp"
#include "inexor/deprecated/databuf_template.hpp"
#include "inexor/deprecated/isclass.hpp"
#include "inexor/deprecated/hashset_template.hpp"
#include "inexor/deprecated/stringslice.hpp"
#include "inexor/deprecated/queue_template.hpp"
#include "inexor/deprecated/old_string.hpp"
#include "inexor/deprecated/packetbuf.hpp"
#include "inexor/deprecated/heapscore.hpp"
#include "inexor/deprecated/sortless.hpp"
#include "inexor/deprecated/sortnameless.hpp"
#include "inexor/deprecated/quicksort_template.hpp"
#include "inexor/deprecated/insertionsort_template.hpp"
#include "inexor/deprecated/reverse_queue_template.hpp"
#include "inexor/deprecated/ipmask.hpp"
#include "inexor/deprecated/stream_template.hpp"
#include "inexor/deprecated/swap_bytes_template.hpp"
#include "inexor/deprecated/streambuf_template.hpp"
#include "inexor/deprecated/cubectype.hpp"


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


// workaround for some C platforms that have these two functions as macros - not used anywhere
#ifdef getchar
  #undef getchar
#endif
#ifdef putchar
  #undef putchar
#endif


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
