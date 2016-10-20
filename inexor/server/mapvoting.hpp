#pragma once

#include "inexor/deprecated/string.hpp"
#include "inexor/deprecated/vector.hpp"

#include "inexor/enumerations/gamemode_bitmask.hpp"

#include "inexor/classes/maprotation.hpp"

#include "inexor/macros/loop_macros.hpp"
#include "inexor/macros/gamemode_macros.hpp"

#include "inexor/fpsgame/game.hpp"

#include <algorithm>
using std::min;
using std::max;

namespace server {

    extern SharedVar<int> lockmaprotation;

    /// 
    void maprotationreset();

    /// 
    void nextmaprotation();

    /// 
    int findmaprotation(int mode, const char *map);

    /// 
    bool searchmodename(const char *haystack, const char *needle);

    /// 
    int genmodemask(vector<char *> &modes);

    /// 
    bool addmaprotation(int modemask, const char *map);

    /// 
    void addmaprotations(tagval *args, int numargs);

};
