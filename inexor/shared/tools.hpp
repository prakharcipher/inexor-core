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

#include "inexor/macros/loop_macros.hpp"

#include "inexor/deprecated/memfree.hpp"

#include "inexor/deprecated/math_constants.hpp"

#include "inexor/deprecated/old_string.hpp"

#include "inexor/templates/databuf.hpp"

#include "inexor/classes/packetbuf.hpp"

#include "inexor/templates/heapscore.hpp"

#include "inexor/classes/sortless.hpp"

#include "inexor/classes/sortnameless.hpp"

#include "inexor/templates/insertionsort.hpp"


/// template implementation of quicksort (one of the fastest sorting algorithms)
template<class T, class F>
static inline void quicksort(T *start, T *end, F fun)
{
    while(end-start > 10)
    {
        T *mid = &start[(end-start)/2], *i = start+1, *j = end-2, pivot;
        if(fun(*start, *mid)) /* start < mid */
        {
            if(fun(end[-1], *start)) { pivot = *start; *start = end[-1]; end[-1] = *mid; } /* end < start < mid */
            else if(fun(end[-1], *mid)) { pivot = end[-1]; end[-1] = *mid; } /* start <= end < mid */
            else { pivot = *mid; } /* start < mid <= end */
        }
        else if(fun(*start, end[-1])) { pivot = *start; *start = *mid; } /*mid <= start < end */
        else if(fun(*mid, end[-1])) { pivot = end[-1]; end[-1] = *start; *start = *mid; } /* mid < end <= start */
        else { pivot = *mid; swap(*start, end[-1]); }  /* end <= mid <= start */
        *mid = end[-2];
        do
        {
            while(fun(*i, pivot)) if(++i >= j) goto partitioned;
            while(fun(pivot, *--j)) if(i >= j) goto partitioned;
            swap(*i, *j);
        }
        while(++i < j);
    partitioned:
        end[-2] = *i;
        *i = pivot;

        if(i-start < end-(i+1))
        {
            quicksort(start, i, fun);
            start = i+1;
        }
        else
        {
            quicksort(i+1, end, fun);
            end = i;
        }
    }
    insertionsort(start, end, fun);
}
template<class T, class F>
static inline void quicksort(T *buf, int n, F fun)
{
    quicksort(buf, buf+n, fun);
}
template<class T>
static inline void quicksort(T *buf, int n)
{
    quicksort(buf, buf+n, sortless());
}


/// I have no idea what that is supposed to be...
/// Is it checking if a data type is a class ??
template<class T> struct isclass
{
    template<class C> static char test(void (C::*)(void));
    template<class C> static int test(...);
    enum 
	{
		yes = sizeof(test<T>(0)) == 1 ? 1 : 0,
		no = yes^1
	};
};

/// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// sorting algorithms templates

// simple bernstein hashing algorithm
// invented by Dan Bernstein
static inline uint hthash(const char *key)
{
    uint h = 5381;
    for(int i = 0, k; (k = key[i]); i++) h = ((h<<5)+h)^k;    /// bernstein k=33 xor
    return h;
}

static inline bool htcmp(const char *x, const char *y)
{
    return !strcmp(x, y);
}

struct stringslice
{
    const char *str;
    int len;
    stringslice() {}
    stringslice(const char *str, int len) : str(str), len(len) {}
    stringslice(const char *str, const char *end) : str(str), len(int(end-str)) {}

    const char *end() const { return &str[len]; }
};

inline char *newstring(const stringslice &s) { return newstring(s.str, s.len); }
inline const char *stringptr(const char *s) { return s; }
inline const char *stringptr(const stringslice &s) { return s.str; }
inline int stringlen(const char *s) { return int(strlen(s)); }
inline int stringlen(const stringslice &s) { return s.len; }

inline char *copystring(char *d, const stringslice &s, size_t len)
{
    size_t slen = min(size_t(s.len), len-1);
    memcpy(d, s.str, slen);
    d[slen] = 0;
    return d;
}
template<size_t N> inline char *copystring(char (&d)[N], const stringslice &s) { return copystring(d, s, N); }

static inline uint memhash(const void *ptr, int len)
{
    const uchar *data = (const uchar *)ptr;
    uint h = 5381;
    loopi(len) h = ((h<<5)+h)^data[i];
    return h;
}

static inline uint hthash(const stringslice &s) { return memhash(s.str, s.len); }

static inline bool htcmp(const stringslice &x, const char *y)
{
    return x.len == (int)strlen(y) && !memcmp(x.str, y, x.len);
}

static inline uint hthash(int key)
{
    return key;
}

/// compares values of x and y
static inline bool htcmp(int x, int y)
{
    return x==y;
}


/// ?
#ifndef STANDALONE
static inline uint hthash(GLuint key)
{
    return key;
}

static inline bool htcmp(GLuint x, GLuint y)
{
    return x==y;
}
#endif


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// Vector template
/// @brief a manual implementation of vector templates (self managing dynamic arrays which store one type).
/// @see std::vector
template <class T, int MINSIZE = 8> struct vector {

	/// data pointer
    T *buf;

	/// ALLOCATED memory size
    int alen;
	/// USED memory size
	int ulen;

    /// Default constructor
	/// @brief sets all members to 0
	vector() : buf(NULL), alen(0), ulen(0)
    {
    }

	/// Copy constructor
	/// @brief this constructor initialises the vector by copying another vector.
	/// @param v the vector from which data will be copied (call by reference).
    vector(const vector &v) : buf(NULL), alen(0), ulen(0)
    {
        *this = v;
    }

	/// Destructor
	/// @brief Deletes all allocated memory and sets vector size to 0.
	/// @sideeffects Calling this method manually means that all data contained in this vector will be lost.
    ~vector() 
	{
		shrink(0);
		if(buf) free(buf);
	}

	/// Operator =
	/// @brief Resets own memory and copies all data from the other vector.
	/// @param v The vector from which data will be copied.
	/// @return Returns a pointer to itself.
    vector<T> &operator=(const vector<T> &v)
    {
        shrink(0);
        if(v.length() > alen) growbuf(v.length());
        loopv(v) add(v[i]);
        return *this;
    }

	/// Add new index to vector
	/// @brief Pushs a new element at the end of the vector. Allocates memory automaticly if neccesary.
	/// @param x Element which will be added at the end
	/// @sideeffects May allocates a lot of memory without your notice
	/// @see growbuf
	/// @return The last element of the vector (which is parameter x)
    T &add(const T &x)
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T(x);
        return buf[ulen++];
    }

	/// Add new EMPTY index to vector.
	/// @brief Pushs a new EMPTY element at the end of the vector. Allocates memory automaticly if neccesary.
	/// @param x Element which will be added at the end.
	/// @see growbuf
	/// @sideeffects May allocates a lot of memory without your notice.
	/// @return The last element of the vector (which is parameter x).
    T &add()
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T;
        return buf[ulen++];
    }

	/// Duplicate vector's last index.
	/// @brief Duplicates last index of the vector and appends it to the end. Allocates memory automaticly if neccesary.
	/// @see growbuf
	/// @sideeffects May allocates a lot of memory without your notice.
	/// @return The last element of the vector.
    T &dup()
    {
        if(ulen==alen) growbuf(ulen+1);
        new (&buf[ulen]) T(buf[ulen-1]);
        return buf[ulen++];
    }

	/// copy vector from vector reference
    void move(vector<T> &v)
    {
        if(!ulen)
        {
            swap(buf, v.buf);
            swap(ulen, v.ulen);
            swap(alen, v.alen);
        }
        else
        {
            growbuf(ulen+v.ulen);
            if(v.ulen) memcpy(&buf[ulen], (void  *)v.buf, v.ulen*sizeof(T));
            ulen += v.ulen;
            v.ulen = 0;
        }
    }

	/// safety check: tests if index i exists in this vector
    bool inrange(size_t i) const { return i<size_t(ulen); }
	/// safety check: tests if index i exists in this vector
	/// integer version: i must be greater (or equal to) 0
    bool inrange(int i) const { return i>=0 && i<ulen; }

	/// get the last index and decrement the vector's size
    T &pop() { return buf[--ulen]; }
	/// get the last index
    T &last() { return buf[ulen-1]; }

	/// decrement vector's size and call the DESTRUCTOR of the template in the last index
    void drop()
	{
		ulen--;
		buf[ulen].~T(); /// call template's destructor
	}

	/// is this vector empty?
    bool empty() const { return ulen==0; }

	/// return size of reserved memory
    int capacity() const { return alen; }
	/// return size of used memory
    int length() const { return ulen; }

	/// [] array access operators
    T &operator[](int i) { ASSERT(i>=0 && i<ulen); return buf[i]; }
    const T &operator[](int i) const { ASSERT(i >= 0 && i<ulen); return buf[i]; }

	/// resets all members to 0
	/// @warning This member does NOT clean up its memory!
    void disown()
	{ 
		buf = NULL; 
		alen = ulen = 0;
	}
	
	/// shrink vector memory size AND DELETE UNUSED MEMORY
    void shrink(int i) { ASSERT(i<=ulen); if(isclass<T>::no) ulen = i; else while(ulen>i) drop(); }
    /// shrink vector memory size
	void setsize(int i) { ASSERT(i<=ulen); ulen = i; }

    void deletecontents() { while(!empty()) delete   pop(); }
    void deletearrays() { while(!empty()) delete[] pop(); }

	/// get the whole vector
    T *getbuf() { return buf; }
	/// get the whole vector as const value
    const T *getbuf() const { return buf; }
    bool inbuf(const T *e) const { return e >= buf && e < &buf[ulen]; }

	/// sort the vector using quicksort template and sort criteria F
    template<class F>
    void sort(F fun, int i = 0, int n = -1)
    {
        quicksort(&buf[i], n < 0 ? ulen-i : n, fun);
    }

    void sort() { sort(sortless()); }
    void sortname() { sort(sortnameless()); }

	/// mix the vector's indices randomly
    void shuffle(){
    	for(int i = 0; i < ulen; i++){
    		int indx = rnd(ulen);
    		T temp = buf[i];
    		buf[i] = buf[indx];
    		buf[indx] = temp;
    	}
    }

	/// reallocate memory for vector (its size)
    void growbuf(int sz)
    {
        int olen = alen;
        if(!alen) alen = max(MINSIZE, sz);
        else while(alen < sz) alen += alen/2;
        if(alen <= olen) return;
        buf = (T *)realloc(buf, alen*sizeof(T));
        if(!buf) abort();
    }

	/// reserved memory and returns vector of the reserved memory
    databuf<T> reserve(int sz)
    {
        if(alen-ulen < sz) growbuf(ulen+sz);
        return databuf<T>(&buf[ulen], sz);
    }

	/// increase value of used bytes manually (?)
    void advance(int sz)
    {
        ulen += sz;
    }

	/// increase value of used bytes by size of a vector
    void addbuf(const databuf<T> &p)
    {
        advance(p.length());
    }

	/// 
    T *pad(int n)
    {
        T *buf = reserve(n).buf;
        advance(n);
        return buf;
    }

	/// write bytes to vector
    void put(const T &v) 
	{
		add(v);
	}
    void put(const T *v, int n)
    {
        databuf<T> buf = reserve(n);
        buf.put(v, n);
        addbuf(buf);
    }

	/// remove indices from vector
	/// remove ordered?
    void remove(int i, int n)
    {
        for(int p = i+n; p<ulen; p++) buf[p-n] = buf[p];
        ulen -= n;
    }

    /// remove element from given position.
    /// @sideeffect 
    /// @return the removed element.
    T remove(int i)
    {
        T e = buf[i];
        for(int p = i+1; p<ulen; p++) buf[p-1] = buf[p];
        ulen--;
        return e;
    }
    T removeunordered(int i)
    {
        T e = buf[i];
        ulen--;
        if(ulen>0) buf[i] = buf[ulen];
        return e;
    }

	/// find (first) index in vector.
    template<class U>
    int find(const U &o)
    {
        loopi(ulen) if(buf[i]==o) return i;
        return -1;
    }

	/// only add new element if it is unique.
    void addunique(const T &o)
    {
        if(find(o) < 0) add(o);
    }

	/// remove an element if equal to to given one.
    void removeobj(const T &o)
    {
        loopi(ulen) if(buf[i] == o)
        {
            int dst = i;
            for(int j = i+1; j < ulen; j++) if(!(buf[j] == o)) buf[dst++] = buf[j];
            setsize(dst);
            break;
        }
    }

	/// replace an index with the last vector index using a template parameter key
    void replacewithlast(const T &o)
    {
        if(!ulen) return;
        loopi(ulen-1) if(buf[i]==o)
        {
            buf[i] = buf[ulen-1];
            break;
        }
        ulen--;
    }

	/// insertion functions
    T &insert(int i, const T &e)
    {
        add(T());
        for(int p = ulen-1; p>i; p--) buf[p] = buf[p-1];
        buf[i] = e;
        return buf[i];
    }
    T *insert(int i, const T *e, int n)
    {
        if(alen-ulen < n) growbuf(ulen+n);
        loopj(n) add(T());
        for(int p = ulen-1; p>=i+n; p--) buf[p] = buf[p-n];
        loopj(n) buf[i+j] = e[j];
        return &buf[i];
    }

	/// reverse all indices (first becomes last and so on...)
    void reverse()
    {
        loopi(ulen/2) swap(buf[i], buf[ulen-1-i]);
    }

	/// ?
    static int heapparent(int i) { return (i - 1) >> 1; }
    static int heapchild(int i) { return (i << 1) + 1; }

	/// ?
    void buildheap()
    {
        for(int i = ulen/2; i >= 0; i--) downheap(i);
    }

	/// ?
    int upheap(int i)
    {
        float score = heapscore(buf[i]);
        while(i > 0)
        {
            int pi = heapparent(i);
            if(score >= heapscore(buf[pi])) break;
            swap(buf[i], buf[pi]);
            i = pi;
        }
        return i;
    }

	/// ?
    T &addheap(const T &x)
    {
        add(x);
        return buf[upheap(ulen-1)];
    }

	/// ?
    int downheap(int i)
    {
        float score = heapscore(buf[i]);
        for(;;)
        {
            int ci = heapchild(i);
            if(ci >= ulen) break;
            float cscore = heapscore(buf[ci]);
            if(score > cscore)
            {
               if(ci+1 < ulen && heapscore(buf[ci+1]) < cscore) { swap(buf[ci+1], buf[i]); i = ci+1; }
               else { swap(buf[ci], buf[i]); i = ci; }
            }
            else if(ci+1 < ulen && heapscore(buf[ci+1]) < score) { swap(buf[ci+1], buf[i]); i = ci+1; }
            else break;
        }
        return i;
    }

	/// ?
    T removeheap()
    {
        T e = removeunordered(0);
        if(ulen) downheap(0);
        return e;
    }

	/// similar to find but uses hashtable keys
    template<class K> 
    int htfind(const K &key)
    {
        loopi(ulen) if(htcmp(key, buf[i])) return i;
        return -1;
    }
};

template<class H, class E, class K, class T> struct hashbase
{
    typedef E elemtype;
    typedef K keytype;
    typedef T datatype;

    enum { CHUNKSIZE = 64 };

    struct chain 
    {
        E elem; 
        chain *next;
    };
    struct chainchunk 
    {
        chain chains[CHUNKSIZE];
        chainchunk *next;
    };

    int size;
    int numelems;
    chain **chains;

    chainchunk *chunks;
    chain *unused;

    enum { DEFAULTSIZE = 1<<10 };

    hashbase(int size = DEFAULTSIZE)
      : size(size)
    {
        numelems = 0;
        chunks = NULL;
        unused = NULL;
        chains = new chain *[size];
        memset(chains, 0, size*sizeof(chain *));
    }

    ~hashbase()
    {
        DELETEA(chains);
        deletechunks();
    }

    chain *insert(uint h)
    {
        if(!unused)
        {
            chainchunk *chunk = new chainchunk;
            chunk->next = chunks;
            chunks = chunk;
            loopi(CHUNKSIZE-1) chunk->chains[i].next = &chunk->chains[i+1];
            chunk->chains[CHUNKSIZE-1].next = unused;
            unused = chunk->chains;
        }
        chain *c = unused;
        unused = unused->next;
        c->next = chains[h];
        chains[h] = c;
        numelems++;
        return c;
    }

    template<class U>
    T &insert(uint h, const U &key)
    {
        chain *c = insert(h);
        H::setkey(c->elem, key);
        return H::getdata(c->elem);
    }

    #define HTFIND(success, fail) \
        uint h = hthash(key)&(this->size-1); \
        for(chain *c = this->chains[h]; c; c = c->next) \
        { \
            if(htcmp(key, H::getkey(c->elem))) return success H::getdata(c->elem); \
        } \
        return (fail);

    template<class U>
    T *access(const U &key)
    {
        HTFIND(&, NULL);
    }

    template<class U, class V>
    T &access(const U &key, const V &elem)
    {
        HTFIND( , insert(h, key) = elem);
    }

    template<class U>
    T &operator[](const U &key)
    {
        HTFIND( , insert(h, key));
    }

    template<class U>
    T &find(const U &key, T &notfound)
    {
        HTFIND( , notfound);
    }

    template<class U>
    const T &find(const U &key, const T &notfound)
    {
        HTFIND( , notfound);
    }

    template<class U>
    bool remove(const U &key)
    {
        uint h = hthash(key)&(size-1);
        for(chain **p = &chains[h], *c = chains[h]; c; p = &c->next, c = c->next)
        {
            if(htcmp(key, H::getkey(c->elem)))
            {
                *p = c->next;
                c->elem.~E();
                new (&c->elem) E;
                c->next = unused;
                unused = c;
                numelems--;
                return true;
            }
        }
        return false;
    }

    void deletechunks()
    {
        for(chainchunk *nextchunk; chunks; chunks = nextchunk)
        {
            nextchunk = chunks->next;
            delete chunks;
        }
    }

    void clear()
    {
        if(!numelems) return;
        memset(chains, 0, size*sizeof(chain *));
        numelems = 0;
        unused = NULL;
        deletechunks();
    }

    static inline chain *enumnext(void *i) { return ((chain *)i)->next; }
    static inline K &enumkey(void *i) { return H::getkey(((chain *)i)->elem); }
    static inline T &enumdata(void *i) { return H::getdata(((chain *)i)->elem); }
};

template<class T> struct hashset : hashbase<hashset<T>, T, T, T>
{
    typedef hashbase<hashset<T>, T, T, T> basetype;

    hashset(int size = basetype::DEFAULTSIZE) : basetype(size) {}

    static inline const T &getkey(const T &elem) { return elem; }
    static inline T &getdata(T &elem) { return elem; }
    template<class K> static inline void setkey(T &elem, const K &key) {}

    template<class V>
    T &add(const V &elem)
    {
        return basetype::access(elem, elem);
    }
};

template<class T> struct hashnameset : hashbase<hashnameset<T>, T, const char *, T>
{
    typedef hashbase<hashnameset<T>, T, const char *, T> basetype;

    hashnameset(int size = basetype::DEFAULTSIZE) : basetype(size) {}

    template<class U> static inline const char *getkey(const U &elem) { return elem.name; }
    template<class U> static inline const char *getkey(U *elem) { return elem->name; }
    static inline T &getdata(T &elem) { return elem; }
    template<class K> static inline void setkey(T &elem, const K &key) {}

    template<class V>
    T &add(const V &elem)
    {
        return basetype::access(getkey(elem), elem);
    }
};

template<class K, class T> struct hashtableentry
{
    K key;
    T data;
};

template<class K, class T> struct hashtable : hashbase<hashtable<K, T>, hashtableentry<K, T>, K, T>
{
    typedef hashbase<hashtable<K, T>, hashtableentry<K, T>, K, T> basetype;
    typedef typename basetype::elemtype elemtype;

    hashtable(int size = basetype::DEFAULTSIZE) : basetype(size) {}

    static inline K &getkey(elemtype &elem) { return elem.key; }
    static inline T &getdata(elemtype &elem) { return elem.data; }
    template<class U> static inline void setkey(elemtype &elem, const U &key) { elem.key = key; }
};

#define enumeratekt(ht,k,e,t,f,b) loopi((ht).size) for(void *ec = (ht).chains[i]; ec;) { k &e = (ht).enumkey(ec); t &f = (ht).enumdata(ec); ec = (ht).enumnext(ec); b; }
#define enumerate(ht,t,e,b)       loopi((ht).size) for(void *ec = (ht).chains[i]; ec;) { t &e = (ht).enumdata(ec); ec = (ht).enumnext(ec); b; }

struct unionfind
{
    struct ufval
    {
        int rank, next;

        ufval() : rank(0), next(-1) {}
    };

    vector<ufval> ufvals;

    int find(int k)
    {
        if(k>=ufvals.length()) return k;
        while(ufvals[k].next>=0) k = ufvals[k].next;
        return k;
    }

    int compressfind(int k)
    {
        if(ufvals[k].next<0) return k;
        return ufvals[k].next = compressfind(ufvals[k].next);
    }

    void unite (int x, int y)
    {
        while(ufvals.length() <= max(x, y)) ufvals.add();
        x = compressfind(x);
        y = compressfind(y);
        if(x==y) return;
        ufval &xval = ufvals[x], &yval = ufvals[y];
        if(xval.rank < yval.rank) xval.next = y;
        else
        {
            yval.next = x;
            if(xval.rank==yval.rank) yval.rank++;
        }
    }
};


// manual implementation of queues
// DEPRECATED! please use std::deque instead!
template <class T, int SIZE> struct queue
{
    int head, tail, len;
    T data[SIZE];

    queue() { clear(); }

    void clear() { head = tail = len = 0; }

    int length() const { return len; }
    bool empty() const { return !len; }
    bool full() const { return len == SIZE; }

    bool inrange(size_t i) const { return i<size_t(len); }
    bool inrange(int i) const { return i>=0 && i<len; }

    T &added() { return data[tail > 0 ? tail-1 : SIZE-1]; }
    T &added(int offset) { return data[tail-offset > 0 ? tail-offset-1 : tail-offset-1 + SIZE]; }
    T &adding() { return data[tail]; }
    T &adding(int offset) { return data[tail+offset >= SIZE ? tail+offset - SIZE : tail+offset]; }
    T &add()
    {
        T &t = data[tail];
        tail++;
        if(tail >= SIZE) tail -= SIZE;
        if(len < SIZE) len++;
        return t;
    }
    T &add(const T &e) { return add() = e; }

    T &pop()
    {
        tail--;
        if(tail < 0) tail += SIZE;
        len--;
        return data[tail];
    }

    T &removing() { return data[head]; }
    T &removing(int offset) { return data[head+offset >= SIZE ? head+offset - SIZE : head+offset]; }
    T &remove()
    {
        T &t = data[head];
        head++;
        if(head >= SIZE) head -= SIZE;
        len--; 
        return t;
    }

    T &operator[](int offset) { return removing(offset); }
    const T &operator[](int offset) const { return removing(offset); }
};


/// reversequeue is the same as std::deque
template <class T, int SIZE> struct reversequeue : queue<T, SIZE>
{
    T &operator[](int offset) { return queue<T, SIZE>::added(offset); }
    const T &operator[](int offset) const { return queue<T, SIZE>::added(offset); }
};

const int islittleendian = 1;

#ifdef SDL_BYTEORDER
  #define endianswap16 SDL_Swap16
  #define endianswap32 SDL_Swap32
  #define endianswap64 SDL_Swap64
#else
  inline ushort endianswap16(ushort n) { return (n<<8) | (n>>8); }
  inline uint endianswap32(uint n) { return (n<<24) | (n>>24) | ((n>>8)&0xFF00) | ((n<<8)&0xFF0000); }
  inline ullong endianswap64(ullong n) { return endianswap32(uint(n >> 32)) | ((ullong)endianswap32(uint(n)) << 32); }
#endif
  template<class T> inline T endianswap(T n) { union { T t; uint i; } conv; conv.t = n; conv.i = endianswap32(conv.i); return conv.t; }
  template<> inline ushort endianswap<ushort>(ushort n) { return endianswap16(n); }
  template<> inline short endianswap<short>(short n) { return endianswap16(n); }
  template<> inline uint endianswap<uint>(uint n) { return endianswap32(n); }
  template<> inline int endianswap<int>(int n) { return endianswap32(n); }
  template<> inline ullong endianswap<ullong>(ullong n) { return endianswap64(n); }
  template<> inline llong endianswap<llong>(llong n) { return endianswap64(n); }
  template<> inline double endianswap<double>(double n) { union { double t; uint i; } conv; conv.t = n; conv.i = endianswap64(conv.i); return conv.t; }
  template<class T> inline void endianswap(T *buf, size_t len) { for(T *end = &buf[len]; buf < end; buf++) *buf = endianswap(*buf); }
  template<class T> inline T endiansame(T n) { return n; }
  template<class T> inline void endiansame(T *buf, size_t len) {}
#ifdef SDL_BYTEORDER
  #if SDL_BYTEORDER == SDL_LIL_ENDIAN
    #define lilswap endiansame
    #define bigswap endianswap
  #else
    #define lilswap endianswap
    #define bigswap endiansame
  #endif
#else
  template<class T> inline T lilswap(T n) { return *(const uchar *)&islittleendian ? n : endianswap(n); }
  template<class T> inline void lilswap(T *buf, size_t len) { if(!*(const uchar *)&islittleendian) endianswap(buf, len); }
  template<class T> inline T bigswap(T n) { return *(const uchar *)&islittleendian ? endianswap(n) : n; }
  template<class T> inline void bigswap(T *buf, size_t len) { if(*(const uchar *)&islittleendian) endianswap(buf, len); }
#endif


// workaround for some C platforms that have these two functions as macros - not used anywhere
#ifdef getchar
  #undef getchar
#endif
#ifdef putchar
  #undef putchar
#endif

struct stream
{
#ifdef WIN32
#ifdef __GNUC__
    typedef off64_t offset;
#else
    typedef __int64 offset;
#endif
#else
    typedef off_t offset;
#endif

    virtual ~stream() {}
    virtual void close() = 0;
    virtual bool end() = 0;
    virtual offset tell() { return -1; }
    virtual offset rawtell() { return tell(); }
    virtual bool seek(offset pos, int whence = SEEK_SET) { return false; }
    virtual offset size();
    virtual offset rawsize() { return size(); }
    virtual size_t read(void *buf, size_t len) { return 0; }
    virtual size_t write(const void *buf, size_t len) { return 0; }
    virtual bool flush() { return true; }
    virtual int getchar() { uchar c; return read(&c, 1) == 1 ? c : -1; }
    virtual bool putchar(int n) { uchar c = n; return write(&c, 1) == 1; }
    virtual bool getline(char *str, size_t len);
    virtual bool putstring(const char *str) { size_t len = strlen(str); return write(str, len) == len; }
    virtual bool putline(const char *str) { return putstring(str) && putchar('\n'); }
    virtual size_t printf(const char *fmt, ...) PRINTFARGS(2, 3);
    virtual uint getcrc() { return 0; }

    template<class T> size_t put(const T *v, size_t n) { return write(v, n*sizeof(T))/sizeof(T); } 
    template<class T> bool put(T n) { return write(&n, sizeof(n)) == sizeof(n); }
    template<class T> bool putlil(T n) { return put<T>(lilswap(n)); }
    template<class T> bool putbig(T n) { return put<T>(bigswap(n)); }

    template<class T> size_t get(T *v, size_t n) { return read(v, n*sizeof(T))/sizeof(T); }
    template<class T> T get() { T n; return read(&n, sizeof(n)) == sizeof(n) ? n : 0; }
    template<class T> T getlil() { return lilswap(get<T>()); }
    template<class T> T getbig() { return bigswap(get<T>()); }

#ifndef STANDALONE
    SDL_RWops *rwops();
#endif
};

template<class T>
struct streambuf
{
    stream *s;

    streambuf(stream *s) : s(s) {}

    T get() { return s->get<T>(); }
    size_t get(T *vals, size_t numvals) { return s->get(vals, numvals); }
    void put(const T &val) { s->put(&val, 1); }
    void put(const T *vals, size_t numvals) { s->put(vals, numvals); } 
    size_t length() { return s->size(); }
};


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
