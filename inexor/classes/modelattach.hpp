#pragma once

#include "inexor/shared/geom.hpp"

struct model;
struct modelattach
{
    const char *tag, *name;
    int anim, basetime;
    vec *pos;
    model *m;

    modelattach() : tag(NULL), name(NULL), anim(-1), basetime(0), pos(NULL), m(NULL) {}
    modelattach(const char *tag, const char *name, int anim = -1, int basetime = 0) : tag(tag), name(name), anim(anim), basetime(basetime), pos(NULL), m(NULL) {}
    modelattach(const char *tag, vec *pos) : tag(tag), name(NULL), anim(-1), basetime(0), pos(pos), m(NULL) {}
};
