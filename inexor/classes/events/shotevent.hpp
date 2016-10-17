#pragma once

/// 
struct shotevent : timedevent
{
    int id, gun;
    vec from, to;
    vector<hitinfo> hits;

    void process(clientinfo *ci);
};
