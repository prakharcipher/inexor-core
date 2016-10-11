#pragma once

#include <string>

namespace inexor {
namespace server {

    /// TODO: remove this duplicated code
    /// allow sorting multiple teams using team scores
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

};
};
