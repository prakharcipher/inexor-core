#pragma once

#include "inexor/deprecated/type_definitions.hpp"

/// template implementation of buffers (networking e.g.).
/// work like streams: you put stuff at the end, you get stuff from the end
template <class T>
struct databuf
{
	/// buffer missuse flags.
    enum
    {
        OVERREAD  = 1<<0,
        OVERWROTE = 1<<1
    };

    /// the pointer to the data.
    T *buf;
    int len, maxlen;
    uchar flags;

    databuf() : buf(NULL), len(0), maxlen(0), flags(0) 
	{
	}

    template<class U>
    databuf(T *buf, U maxlen) : buf(buf), len(0), maxlen((int)maxlen), flags(0) {}

    void reset()
    {
        len = 0;
        flags = 0;
    }

    void reset(T *buf_, int maxlen_)
    {
        reset();
        buf = buf_;
        maxlen = maxlen_;
    }

	/// get one byte from the buffer and 
    const T &get()
    {
        static T overreadval = 0;
        if(len<maxlen) return buf[len++];
        flags |= OVERREAD;
        return overreadval;
    }

	/// create a sub buffer copy from this buffer (from the current position)
    databuf subbuf(int sz)
    {
        sz = clamp(sz, 0, maxlen-len);
        len += sz;
        return databuf(&buf[len-sz], sz);
    }

    T *pad(int numvals)
    {
        T *vals = &buf[len];
        len += min(numvals, maxlen-len);
        return vals;
    }

	/// put one element at the end of the buffer.
    void put(const T &val)
    {
        if(len<maxlen) buf[len++] = val;
        else flags |= OVERWROTE;
    }

	/// put a given number of elements at the end of the buffer.
    void put(const T *vals, int numvals)
    {
        if(maxlen-len<numvals) flags |= OVERWROTE;
        memcpy(&buf[len], (const void *)vals, min(maxlen-len, numvals)*sizeof(T));
        len += min(maxlen-len, numvals);
    }

	/// receive the given amount of elements from the buffer to given array.
    int get(T *vals, int numvals)
    {
        int read = min(maxlen-len, numvals);
        if(read<numvals) flags |= OVERREAD;
        memcpy(vals, (void *)&buf[len], read*sizeof(T));
        len += read;
        return read;
    }

	/// skip the first n elemnts of the buffer.
    void offset(int n)
    {
        n = min(n, maxlen);
        buf += n;
        maxlen -= n;
        len = max(len-n, 0);
    }

    T *getbuf() const { return buf; }

    bool empty() const { return len==0; }
    /// receive amount of values in buffer.
	int length() const { return len; }
	/// receive remaining space in buffer.
    int remaining() const { return maxlen-len; }
	/// did we try to read too much?
    bool overread() const { return (flags&OVERREAD)!=0; }
	/// did we try to write too much into the buffer?
    bool overwrote() const { return (flags&OVERWROTE)!=0; }

    bool check(int n) { return remaining() >= n; }

	/// force buffer to skip all free memory space
    void forceoverread()
    {
        len = maxlen;
        flags |= OVERREAD;
    }
};

typedef databuf<char> charbuf;
typedef databuf<uchar> ucharbuf;
