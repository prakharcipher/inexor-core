#pragma once

#include "inexor/deprecated/type_definitions.hpp"
#include "inexor/deprecated/vector_template.hpp"
#include "inexor/deprecated/old_string.hpp"

namespace inexor {
namespace server {

    /// demos contain stored network messages of a game
    /// which can be replayed to review games
    struct demoheader
    {
        char magic[16];
        int version, protocol;
    };

    struct demofile
    {
        string info;
        uchar *data;
        int len;
    };

    vector<demofile> demos;

};
};
