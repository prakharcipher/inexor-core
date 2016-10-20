#pragma once

#include "inexor/classes/cubescript/identval.hpp"

/// 
struct identstack
{
    identval val;
    int valtype;
    identstack *next;
};
