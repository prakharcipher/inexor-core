#pragma once

#include "inexor/rpc/SharedTree.hpp"

#include "inexor/enumerations/cubescript/cubescript_val_types.hpp"

#include "inexor/enumerations/cubescript/cubescript_block_types.hpp"

#include "inexor/enumerations/cubescript/cubescript_entity_ids.hpp"

#include "inexor/enumerations/cubescript/cubescript_ident_flags.hpp"

#include "inexor/classes/cubescript/identval.hpp"

#include "inexor/classes/cubescript/tagval.hpp"
        
#include "inexor/classes/cubescript/identstack.hpp"

#include "inexor/classes/cubescript/identvalptr.hpp"

#include "inexor/classes/cubescript/ident.hpp"

extern void addident(ident *id);
extern tagval *commandret;
extern const char *intstr(int v);
extern void intret(int v);
extern const char *floatstr(float v);
extern void floatret(float v);
extern void stringret(char *s);
extern void result(tagval &v);
extern void result(const char *s);

static inline int parseint(const char *s)
{
    return int(strtoul(s, NULL, 0));
}

static inline float parsefloat(const char *s)
{
    // not all platforms (windows) can parse hexadecimal integers via strtod
    char *end;
    double val = strtod(s, &end);
    return val || end==s || (*end!='x' && *end!='X') ? float(val) : float(parseint(s));
}

static inline void intformat(char *buf, int v, int len = 20) { nformatstring(buf, len, "%d", v); }
static inline void floatformat(char *buf, float v, int len = 20) { nformatstring(buf, len, v==int(v) ? "%.1f" : "%.7g", v); }

static inline const char *getstr(const identval &v, int type) 
{
    switch(type)
    {
        case VAL_STR: case VAL_MACRO: return v.s;
        case VAL_INT: return intstr(v.i);
        case VAL_FLOAT: return floatstr(v.f);
        default: return "";
    }
}
inline const char *tagval::getstr() const { return ::getstr(*this, type); }
inline const char *ident::getstr() const { return ::getstr(val, valtype); }

static inline int getint(const identval &v, int type)
{
    switch(type)
    {
        case VAL_INT: return v.i;
        case VAL_FLOAT: return int(v.f);
        case VAL_STR: case VAL_MACRO: return parseint(v.s); 
        default: return 0;
    }
}
inline int tagval::getint() const { return ::getint(*this, type); }
inline int ident::getint() const { return ::getint(val, valtype); }

static inline float getfloat(const identval &v, int type)
{
    switch(type)
    {
        case VAL_FLOAT: return v.f;
        case VAL_INT: return float(v.i);
        case VAL_STR: case VAL_MACRO: return parsefloat(v.s);
        default: return 0.0f;
    }
}
inline float tagval::getfloat() const { return ::getfloat(*this, type); }
inline float ident::getfloat() const { return ::getfloat(val, valtype); } 

inline void ident::getval(tagval &v) const
{
    switch(valtype)
    {
        case VAL_STR: case VAL_MACRO: v.setstr(newstring(val.s)); break;
        case VAL_INT: v.setint(val.i); break;
        case VAL_FLOAT: v.setfloat(val.f); break;
        default: v.setnull(); break;
    }
}

#include "inexor/macros/cubescript/registercmd_macros.hpp"
