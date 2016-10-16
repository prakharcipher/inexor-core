#pragma once

struct sortless
{
    template<class T> bool operator()(const T &x, const T &y) const { return x < y; }
    bool operator()(char *x, char *y) const { return strcmp(x, y) < 0; }
    bool operator()(const char *x, const char *y) const { return strcmp(x, y) < 0; }
};
