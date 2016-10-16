#pragma once

/// TODO: document!
struct teamscore
{
    const char *team;
    int score;
    teamscore() {}
    teamscore(const char *s, int n) : team(s), score(n) {}

    /// used for quicksort template to compare teams
    static bool compare(const teamscore &x, const teamscore &y)
    {
        if(x.score > y.score) return true;
        if(x.score < y.score) return false;
        return strcmp(x.team, y.team) < 0;
    }
};


/// create hashes to access hashmaps
static inline uint hthash(const teamscore &t) 
{
    return hthash(t.team); 
}

/// compare two teamnames
static inline bool htcmp(const char *key, const teamscore &t) 
{
    return htcmp(key, t.team);
}

/// scoreboard team block description
struct teaminfo
{
    char team[MAXTEAMLEN+1];
    int frags;
};

/// create hash for hashsts
static inline uint hthash(const teaminfo &t) 
{ 
    return hthash(t.team); 
}

/// compare two team names
static inline bool htcmp(const char *team, const teaminfo &t)
{
    return !strcmp(team, t.team);
}
