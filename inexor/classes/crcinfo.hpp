#pragma once

/// 
struct crcinfo
{
    int crc, matches;

    crcinfo() {}
    crcinfo(int crc, int matches) : crc(crc), matches(matches) {}

    static bool compare(const crcinfo &x, const crcinfo &y) { return x.matches > y.matches; }
};
