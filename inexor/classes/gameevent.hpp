#pragma once

/// 
struct gameevent
{
    virtual ~gameevent() {}

    virtual bool flush(clientinfo *ci, int fmillis);
    virtual void process(clientinfo *ci) {}

    virtual bool keepable() const { return false; }
};
