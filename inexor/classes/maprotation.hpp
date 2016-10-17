#pragma once

/// 
struct maprotation
{
    static int exclude;
    long modes;
    string map;

    long calcmodemask() const { return modes&((long)1<<NUMGAMEMODES) ? modes & ~exclude : modes; }
    bool hasmode(int mode, int offset = STARTGAMEMODE) const { return (calcmodemask() & (1 << (mode-offset))) != 0; }

    int findmode(int mode) const
    {
        if(!hasmode(mode)) loopi(NUMGAMEMODES) if(hasmode(i, 0)) return i+STARTGAMEMODE;
        return mode;
    }

    bool match(int reqmode, const char *reqmap) const
    {
        return hasmode(reqmode) && (!map[0] || !reqmap[0] || !strcmp(map, reqmap));
    }

    bool includes(const maprotation &rot) const
    {
        return rot.modes == modes ? rot.map[0] && !map[0] : (rot.modes & modes) == rot.modes;
    }
};

int maprotation::exclude = 0;
vector<maprotation> maprotations;
int curmaprotation = 0;
