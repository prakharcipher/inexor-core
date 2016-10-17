#pragma once

/// 
struct savedscore
{
    uint ip;
    string name;
    int maxhealth, frags, flags, deaths, teamkills, shotdamage, damage;
    int timeplayed;
    float effectiveness;
    int bombradius;
    int bombdelay;

    void save(gamestate &gs)
    {
        maxhealth = gs.maxhealth;
        frags = gs.frags;
        flags = gs.flags;
        deaths = gs.deaths;
        teamkills = gs.teamkills;
        shotdamage = gs.shotdamage;
        damage = gs.damage;
        timeplayed = gs.timeplayed;
        effectiveness = gs.effectiveness;
        bombradius = gs.bombradius;
        bombdelay = gs.bombdelay;
    }

    void restore(gamestate &gs)
    {
        if(gs.health==gs.maxhealth) gs.health = maxhealth;
        gs.maxhealth = maxhealth;
        gs.frags = frags;
        gs.flags = flags;
        gs.deaths = deaths;
        gs.teamkills = teamkills;
        gs.shotdamage = shotdamage;
        gs.damage = damage;
        gs.timeplayed = timeplayed;
        gs.effectiveness = effectiveness;
        gs.bombradius = bombradius;
        gs.bombdelay = bombdelay;
    }
};
