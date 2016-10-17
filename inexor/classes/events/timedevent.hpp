#pragma once

/// 
struct timedevent : gameevent
{
    int millis;

    bool flush(clientinfo *ci, int fmillis);
};
