#pragma once

/// 
struct userkey
{
    char *name;
    char *desc;

    userkey() : name(NULL), desc(NULL) {}
    userkey(char *name, char *desc) : name(name), desc(desc) {}
};

static inline uint hthash(const userkey &k) { return ::hthash(k.name); }
static inline bool htcmp(const userkey &x, const userkey &y) { return !strcmp(x.name, y.name) && !strcmp(x.desc, y.desc); }
