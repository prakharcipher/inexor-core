/// DO NOT USE THIS BECAUSE IT IS DEPRECATED
/// USE PROPER DEBUG/RELEASE PROJECT CONFIGURATIONS
/// AND assert!

#pragma once

#include <assert.h>

#ifdef _DEBUG //TODO remove
    #define ASSERT(c) assert(c)
#else
    #define ASSERT(c) if(c) {}
#endif