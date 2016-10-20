#pragma once

#include "inexor/enumerations/cubescript/cubescript_ident_flags.hpp"
#include "inexor/enumerations/cubescript/cubescript_val_types.hpp"
#include "inexor/macros/cubescript/identfun.hpp"
#include "inexor/classes/cubescript/identval.hpp"
#include "inexor/classes/cubescript/identstack.hpp"
#include "inexor/classes/cubescript/identvalptr.hpp"
#include "inexor/classes/cubescript/tagval.hpp"
#include "inexor/rpc/SharedTree.hpp"

/// 
struct ident
{
    uchar type; // one of ID_* above
    union
    {
        uchar valtype; // ID_ALIAS
        uchar numargs; // ID_COMMAND
    };
    ushort flags;
    int index;
    const char *name;
    union
    {
        struct // ID_VAR, ID_FVAR, ID_SVAR
        {
            union
            {
                struct { int minval, maxval; };     // ID_VAR
                struct { float minvalf, maxvalf; }; // ID_FVAR
            };
            identvalptr storage;
            identval overrideval;
        };
        struct // ID_ALIAS
        {
            uint *code;
            identval val;
            identstack *stack;
        };
        struct // ID_COMMAND
        {
            const char *args;
            uint argmask;
        };
    };
    identfun fun; // ID_VAR, ID_FVAR, ID_SVAR, ID_COMMAND

    ident() {}
    // ID_VAR
    ident(int t, const char *n, int m, int x, SharedVar<int> *s, void *f = NULL, int flags = 0)
        : type(t), flags(flags | (m > x ? IDF_READONLY : 0)), name(n), minval(m), maxval(x), fun((identfun)f)
    { storage.i = s; }
    // ID_FVAR
    ident(int t, const char *n, float m, float x, SharedVar<float> *s, void *f = NULL, int flags = 0)
        : type(t), flags(flags | (m > x ? IDF_READONLY : 0)), name(n), minvalf(m), maxvalf(x), fun((identfun)f)
    { storage.f = s; }
    // ID_SVAR
    ident(int t, const char *n, SharedVar<char*> *s, void *f = NULL, int flags = 0)
        : type(t), flags(flags), name(n), fun((identfun)f)
    { storage.s = s; }
    // ID_ALIAS
    ident(int t, const char *n, char *a, int flags)
        : type(t), valtype(VAL_STR), flags(flags), name(n), code(NULL), stack(NULL)
    { val.s = a; }
    ident(int t, const char *n, int a, int flags)
        : type(t), valtype(VAL_INT), flags(flags), name(n), code(NULL), stack(NULL)
    { val.i = a; }
    ident(int t, const char *n, float a, int flags)
        : type(t), valtype(VAL_FLOAT), flags(flags), name(n), code(NULL), stack(NULL)
    { val.f = a; }
    ident(int t, const char *n, int flags)
        : type(t), valtype(VAL_NULL), flags(flags), name(n), code(NULL), stack(NULL)
    {}
    ident(int t, const char *n, const tagval &v, int flags)
        : type(t), valtype(v.type), flags(flags), name(n), code(NULL), stack(NULL)
    { val = v; }
    // ID_COMMAND
    ident(int t, const char *n, const char *args, uint argmask, int numargs, void *f = NULL, int flags = 0)
        : type(t), numargs(numargs), flags(flags), name(n), args(args), argmask(argmask), fun((identfun)f)
    {}

    void changed() { if(fun) fun(); }

    void setval(const tagval &v)
    {
        valtype = v.type;
        val = v;
    }

    void setval(const identstack &v)
    {
        valtype = v.valtype;
        val = v.val;
    }

    void forcenull()
    {
        if(valtype==VAL_STR) delete[] val.s;
        valtype = VAL_NULL;
    }

    float getfloat() const;
    int getint() const;
    const char *getstr() const;
    void getval(tagval &v) const;
};
