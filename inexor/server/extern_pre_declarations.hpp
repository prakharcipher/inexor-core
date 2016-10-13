#pragma once

#include "inexor/deprecated/type_definitions.hpp"

namespace game {

    extern bool clientoption(const char *arg);

};

namespace server {

    extern bool serveroption(const char *arg);
};

extern uint getclientip(int n);