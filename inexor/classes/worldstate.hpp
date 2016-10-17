#pragma once

/// 
struct worldstate
{
    int uses, len;
    uchar *data;

    worldstate() : uses(0), len(0), data(NULL) {}

    void setup(int n) { len = n; data = new uchar[n]; }
    void cleanup() { DELETEA(data); len = 0; }
    bool contains(const uchar *p) const { return p >= data && p < &data[len]; }
};
