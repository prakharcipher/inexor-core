#pragma once

/// 
struct votecount
{
    char *map;
    int mode, count;
    votecount() {}
    votecount(char *s, int n) : map(s), mode(n), count(0) {}
};
