#pragma once

#include "inexor/classes/cubescript/tagval.hpp"

struct nullval : tagval
{
    nullval()
    {
        setnull();
    }
};

// TODO: rename this structure or the instance!
extern struct nullval nullval;
extern tagval noret;
extern tagval *commandret;
