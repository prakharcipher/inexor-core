#pragma once

/// 
struct pickupevent : gameevent
{
    int ent;

    void process(clientinfo *ci);
};
