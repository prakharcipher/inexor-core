#pragma once

/// 
struct teamrank
{
    const char *name;
    float rank;
    int clients;

    teamrank(const char *name) : name(name), rank(0), clients(0) {}
};
