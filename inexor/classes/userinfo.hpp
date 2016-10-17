#pragma once

/// 
struct userinfo : userkey
{
    void *pubkey;
    int privilege;

    userinfo() : pubkey(NULL), privilege(PRIV_NONE) {}
    ~userinfo() { delete[] name; delete[] desc; if(pubkey) freepubkey(pubkey); }
};
