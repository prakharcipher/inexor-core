#pragma once

#include "inexor/deprecated/type_definitions.hpp"
#include "inexor/shared/geom.hpp"

/// 
struct entity                                   
{
    vec o;                                      // position
    short attr1, attr2, attr3, attr4, attr5;
    uchar type;                                 // type is one of the above
    uchar reserved;
};
