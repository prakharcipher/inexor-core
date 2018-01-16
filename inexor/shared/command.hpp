/// @file command.hpp
/// Cubescript API
/// Deprecated.

#pragma once

#include "inexor/network/SharedTree.hpp"
#include "inexor/shared/cube_vector.hpp"
#include "inexor/shared/cube_formatting.hpp"
#include "inexor/shared/cube_types.hpp"
#include "inexor/shared/cube_tools.hpp"

enum 
{
	VAL_NULL = 0,
	VAL_INT,
	VAL_FLOAT,
	VAL_STR,
	VAL_ANY,
	VAL_CODE,
	VAL_MACRO,
	VAL_IDENT
};

enum
{
    CODE_START = 0,
    CODE_OFFSET,
    CODE_POP,
    CODE_ENTER, CODE_EXIT, CODE_VAL,
    CODE_VALI,
    CODE_MACRO,
    CODE_BOOL,
    CODE_BLOCK,
    CODE_COMPILE,
    CODE_FORCE,
    CODE_RESULT,
    CODE_IDENT, CODE_IDENTU, CODE_IDENTARG,
    CODE_COM, CODE_COMD, CODE_COMC, CODE_COMV,
    CODE_CONC, CODE_CONCW, CODE_CONCM, CODE_DOWN,
    CODE_SVAR, CODE_SVAR1,
    CODE_IVAR, CODE_IVAR1, CODE_IVAR2, CODE_IVAR3,
    CODE_FVAR, CODE_FVAR1,
    CODE_LOOKUP, CODE_LOOKUPU, CODE_LOOKUPARG, CODE_ALIAS, CODE_ALIASU, CODE_ALIASARG, CODE_CALL, CODE_CALLU, CODE_CALLARG,
    CODE_PRINT,
    CODE_LOCAL,

    CODE_OP_MASK = 0x3F,
    CODE_RET = 6,
    CODE_RET_MASK = 0xC0,

    /* return type flags */
    RET_NULL   = VAL_NULL<<CODE_RET,
    RET_STR    = VAL_STR<<CODE_RET,
    RET_INT    = VAL_INT<<CODE_RET,
    RET_FLOAT  = VAL_FLOAT<<CODE_RET,
};

enum
{
    ID_VAR,
    ID_FVAR,
    ID_SVAR,
    ID_NOSYNC_VAR, // only cubescript var, but no flex binding (InexorTreeAPI)
    ID_COMMAND,
    ID_ALIAS,
    ID_LOCAL
};

enum 
{
	IDF_PERSIST = 1<<0,
	IDF_OVERRIDE = 1<<1,
	IDF_HEX = 1<<2,
	IDF_READONLY = 1<<3,
	IDF_OVERRIDDEN = 1<<4,
	IDF_UNKNOWN = 1<<5,
	IDF_ARG = 1<<6
};

struct ident;

struct identval
{
    union
    {
        int i;      // ID_VAR, VAL_INT
        float f;    // ID_FVAR, VAL_FLOAT
        char *s;    // ID_SVAR, VAL_STR
        const uint *code; // VAL_CODE
        ident *id;  // VAL_IDENT
    };
};

struct tagval : identval
{
    int type;

    void setint(int val) { type = VAL_INT; i = val; }
    void setfloat(float val) { type = VAL_FLOAT; f = val; }
    void setstr(char *val) { type = VAL_STR; s = val; }
    void setnull() { type = VAL_NULL; i = 0; }
    void setcode(const uint *val) { type = VAL_CODE; code = val; }
    void setmacro(const uint *val) { type = VAL_MACRO; code = val; }
    void setident(ident *val) { type = VAL_IDENT; id = val; }

    const char *getstr() const;
    int getint() const;
    float getfloat() const;
    bool getbool() const;

    void cleanup();
};
        
struct identstack
{
    identval val;
    int valtype;
    identstack *next;
};

union identvalptr
{
    SharedVar<int>   *i;   // ID_VAR
    SharedVar<float> *f;   // ID_FVAR
    SharedVar<char*> *s;   // ID_SVAR
    // Non-Tree-API synced vars (only cubescript, no Tree syncing):
    int *iold; // ID_NOSYNC_VAR
};

typedef void (__cdecl *identfun)();

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
    ident(int t, const char *n, int m, int x, SharedVar<int> *s, void *f = nullptr, int flags = 0)
        : type(t), flags(flags | (m > x ? IDF_READONLY : 0)), name(n), minval(m), maxval(x), fun((identfun)f)
    { storage.i = s; }
    // ID_VAR oldschool (without InexorTree binding)
    ident(int t, const char *n, int m, int x, int *s, void *f = nullptr, int flags = 0)
        : type(t), flags(flags | (m > x ? IDF_READONLY : 0)), name(n), minval(m), maxval(x), fun((identfun)f)
    { storage.iold = s; }
    // ID_FVAR
    ident(int t, const char *n, float m, float x, SharedVar<float> *s, void *f = nullptr, int flags = 0)
        : type(t), flags(flags | (m > x ? IDF_READONLY : 0)), name(n), minvalf(m), maxvalf(x), fun((identfun)f)
    { storage.f = s; }
    // ID_SVAR
    ident(int t, const char *n, SharedVar<char*> *s, void *f = nullptr, int flags = 0)
        : type(t), flags(flags), name(n), fun((identfun)f)
    { storage.s = s; }
    // ID_ALIAS
    ident(int t, const char *n, char *a, int flags)
        : type(t), valtype(VAL_STR), flags(flags), name(n), code(nullptr), stack(nullptr)
    { val.s = a; }
    ident(int t, const char *n, int a, int flags)
        : type(t), valtype(VAL_INT), flags(flags), name(n), code(nullptr), stack(nullptr)
    { val.i = a; }
    ident(int t, const char *n, float a, int flags)
        : type(t), valtype(VAL_FLOAT), flags(flags), name(n), code(nullptr), stack(nullptr)
    { val.f = a; }
    ident(int t, const char *n, int flags)
        : type(t), valtype(VAL_NULL), flags(flags), name(n), code(nullptr), stack(nullptr)
    {}
    ident(int t, const char *n, const tagval &v, int flags)
        : type(t), valtype(v.type), flags(flags), name(n), code(nullptr), stack(nullptr)
    { val = v; }
    // ID_COMMAND
    ident(int t, const char *n, const char *args, uint argmask, int numargs, void *f = nullptr, int flags = 0)
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
    return int(strtoul(s, nullptr, 0));
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


extern int variable(const char *name, int min, int cur, int max, SharedVar<int> *storage, identfun fun, int flags);
extern int variable(const char *name, int min, int cur, int max, int *storage, identfun fun, int flags);
extern float fvariable(const char *name, float min, float cur, float max, SharedVar<float> *storage, identfun fun, int flags);
extern char *svariable(const char *name, const char *cur, SharedVar<char*> *storage, identfun fun, int flags);
extern void setvar(const char *name, int i, bool dofunc = true, bool doclamp = true);
extern void setfvar(const char *name, float f, bool dofunc = true, bool doclamp = true);
extern void setsvar(const char *name, const char *str, bool dofunc = true);
extern void setvarchecked(ident *id, int val);
extern void setfvarchecked(ident *id, float val);
extern void setsvarchecked(ident *id, const char *val);
extern void touchvar(const char *name);
extern int getvar(const char *name);
extern int getvarmin(const char *name);
extern int getvarmax(const char *name);
extern bool identexists(const char *name);
extern ident *getident(const char *name);
extern ident *newident(const char *name, int flags = 0);
extern ident *readident(const char *name);
extern ident *writeident(const char *name, int flags = 0);
extern bool addcommand(const char *name, identfun fun, const char *narg);
extern bool addkeyword(int type, const char *name);
extern uint *compilecode(const char *p);
extern void keepcode(uint *p);
extern void freecode(uint *p);
extern void executeret(const uint *code, tagval &result = *commandret);
extern void executeret(const char *p, tagval &result = *commandret);
extern char *executestr(const uint *code);
extern char *executestr(const char *p);
extern int execute(const uint *code);
extern int execute(const char *p);
extern bool executebool(const uint *code);
extern bool executebool(const char *p);
extern bool execfile(const char *cfgfile, bool msg = true);
extern const char *getcurexecdir();
extern void alias(const char *name, const char *action);
extern void alias(const char *name, tagval &v);
extern const char *getalias(const char *name);
extern const char *escapestring(const char *s);
extern const char *escapeid(const char *s);
static inline const char *escapeid(ident &id) { return escapeid(id.name); }
extern bool validateblock(const char *s);
extern void explodelist(const char *s, vector<char *> &elems, int limit = -1);
extern char *indexlist(const char *s, int pos);
extern int listlen(const char *s);
extern void printvar(ident *id);
extern void printvar(ident *id, int i);
extern void printfvar(ident *id, float f);
extern void printsvar(ident *id, const char *s);
extern int clampvar(ident *id, int i, int minval, int maxval);
extern float clampfvar(ident *id, float f, float minval, float maxval);
extern void loopiter(ident *id, identstack &stack, const tagval &v);
extern void loopend(ident *id, identstack &stack);

#define loopstart(id, stack) if((id)->type != ID_ALIAS) return; identstack stack;
static inline void loopiter(ident *id, identstack &stack, int i) { tagval v; v.setint(i); loopiter(id, stack, v); }
static inline void loopiter(ident *id, identstack &stack, float f) { tagval v; v.setfloat(f); loopiter(id, stack, v); }
static inline void loopiter(ident *id, identstack &stack, const char *s) { tagval v; v.setstr(newstring(s)); loopiter(id, stack, v); }


// nasty macros for registering script functions, abuses globals to avoid excessive infrastructure
#define KEYWORD(name, type) UNUSED static bool __dummy_##name = addkeyword(type, #name)
#define COMMANDN(name, fun, nargs) UNUSED static bool __dummy_##fun = addcommand(#name, (identfun)fun, nargs); SharedFunc(name)
#define COMMAND(name, nargs) COMMANDN(name, name, nargs)

#define _VAR(name, global, min, cur, max, persist) SharedVar<int> global((int)cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, NULL, persist)
/// Version of VAR which doesnt sync with the tree api.
#define _VAR_NOSYNC(name, global, min, cur, max, persist) int global((int)cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, NULL, persist)

#define VARN(name, global, min, cur, max) _VAR(name, global, min, cur, max, 0)
#define VARN_NOSYNC(name, global, min, cur, max) _VAR_NOSYNC(name, global, min, cur, max, 0)
#define VARNP(name, global, min, cur, max) _VAR(name, global, min, cur, max, IDF_PERSIST)
#define VARNR(name, global, min, cur, max) _VAR(name, global, min, cur, max, IDF_OVERRIDE)
#define VAR(name, min, cur, max) _VAR(name, name, min, cur, max, 0)
#define VAR_NOSYNC(name, min, cur, max) _VAR_NOSYNC(name, name, min, cur, max, 0)
#define VARP(name, min, cur, max) _VAR(name, name, min, cur, max, IDF_PERSIST)
#define VARR(name, min, cur, max) _VAR(name, name, min, cur, max, IDF_OVERRIDE)
#define _VARF(name, global, min, cur, max, body, persist)  void var_##name(); SharedVar<int> global((int)cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, var_##name, persist); void var_##name() { body; }
#define VARFN(name, global, min, cur, max, body) _VARF(name, global, min, cur, max, body, 0)
#define VARF(name, min, cur, max, body) _VARF(name, name, min, cur, max, body, 0)
#define VARFP(name, min, cur, max, body) _VARF(name, name, min, cur, max, body, IDF_PERSIST)
#define VARFR(name, min, cur, max, body) _VARF(name, name, min, cur, max, body, IDF_OVERRIDE)

#define _HVAR(name, global, min, cur, max, persist) SharedVar<int> global(cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, NULL, persist | IDF_HEX)
#define HVARN(name, global, min, cur, max) _HVAR(name, global, min, cur, max, 0)
#define HVARNP(name, global, min, cur, max) _HVAR(name, global, min, cur, max, IDF_PERSIST)
#define HVARNR(name, global, min, cur, max) _HVAR(name, global, min, cur, max, IDF_OVERRIDE)
#define HVAR(name, min, cur, max) _HVAR(name, name, min, cur, max, 0)
#define HVARP(name, min, cur, max) _HVAR(name, name, min, cur, max, IDF_PERSIST)
#define HVARR(name, min, cur, max) _HVAR(name, name, min, cur, max, IDF_OVERRIDE)
#define _HVARF(name, global, min, cur, max, body, persist)  void var_##name(); SharedVar<int> global((int)cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, var_##name, persist | IDF_HEX); void var_##name() { body; }
#define HVARFN(name, global, min, cur, max, body) _HVARF(name, global, min, cur, max, body, 0)
#define HVARF(name, min, cur, max, body) _HVARF(name, name, min, cur, max, body, 0)
#define HVARFP(name, min, cur, max, body) _HVARF(name, name, min, cur, max, body, IDF_PERSIST)
#define HVARFR(name, min, cur, max, body) _HVARF(name, name, min, cur, max, body, IDF_OVERRIDE)

#define _FVAR(name, global, min, cur, max, persist) SharedVar<float> global((float)cur); UNUSED float dummy_register_##global = fvariable(#name, min, cur, max, &global, NULL, persist)
#define FVARN(name, global, min, cur, max) _FVAR(name, global, min, cur, max, 0)
#define FVARNP(name, global, min, cur, max) _FVAR(name, global, min, cur, max, IDF_PERSIST)
#define FVARNR(name, global, min, cur, max) _FVAR(name, global, min, cur, max, IDF_OVERRIDE)
#define FVAR(name, min, cur, max) _FVAR(name, name, min, cur, max, 0)
#define FVARP(name, min, cur, max) _FVAR(name, name, min, cur, max, IDF_PERSIST)
#define FVARR(name, min, cur, max) _FVAR(name, name, min, cur, max, IDF_OVERRIDE)
#define _FVARF(name, global, min, cur, max, body, persist) void var_##name(); SharedVar<float> global((float)cur); UNUSED float dummy_register_##global = fvariable(#name, min, cur, max, &global, var_##name, persist); void var_##name() { body; }
#define FVARFN(name, global, min, cur, max, body) _FVARF(name, global, min, cur, max, body, 0)
#define FVARF(name, min, cur, max, body) _FVARF(name, name, min, cur, max, body, 0)
#define FVARFP(name, min, cur, max, body) _FVARF(name, name, min, cur, max, body, IDF_PERSIST)
#define FVARFR(name, min, cur, max, body) _FVARF(name, name, min, cur, max, body, IDF_OVERRIDE)


#define _SVAR(name, global, cur, persist) SharedVar<char*> global((char *)cur); UNUSED char* dummy_register_##global = *global = (char*)svariable(#name, cur, &global, NULL, persist)
#define SVARN(name, global, cur) _SVAR(name, global, cur, 0)
#define SVARNP(name, global, cur) _SVAR(name, global, cur, IDF_PERSIST)
#define SVARNR(name, global, cur) _SVAR(name, global, cur, IDF_OVERRIDE)
#define SVAR(name, cur) _SVAR(name, name, cur, 0)
#define SVARP(name, cur) _SVAR(name, name, cur, IDF_PERSIST)
#define SVARR(name, cur) _SVAR(name, name, cur, IDF_OVERRIDE)
#define _SVARF(name, global, cur, body, persist) void var_##name(); SharedVar<char*> global((char*)cur); UNUSED char* dummy_register_##global = *global = (char*)svariable(#name, cur, &global, var_##name, persist); void var_##name() { body; }
#define SVARFN(name, global, cur, body) _SVARF(name, global, cur, body, 0)
#define SVARF(name, cur, body) _SVARF(name, name, cur, body, 0)
#define SVARFP(name, cur, body) _SVARF(name, name, cur, body, IDF_PERSIST)
#define SVARFR(name, cur, body) _SVARF(name, name, cur, body, IDF_OVERRIDE)

// anonymous inline commands, uses nasty template trick with line numbers to keep names unique
#define ICOMMANDNS(name, cmdname, nargs, proto, b)\
template<int N> struct cmdname;                 \
template<> struct cmdname<__LINE__>             \
{                                               \
    static bool init;                           \
    static void run proto;                      \
};                                              \
bool cmdname<__LINE__>::init = addcommand(name, (identfun)cmdname<__LINE__>::run, nargs); \
void cmdname<__LINE__>::run proto \
{ b; }

#define ICOMMANDN(name, cmdname, nargs, proto, b) ICOMMANDNS(#name, cmdname, nargs, proto, b)
#define ICOMMANDNAME(name) _icmd_##name
#define ICOMMAND(name, nargs, proto, b) ICOMMANDN(name, ICOMMANDNAME(name), nargs, proto, b)
#define ICOMMANDSNAME _icmds_
#define ICOMMANDS(name, nargs, proto, b) ICOMMANDNS(name, ICOMMANDSNAME, nargs, proto, b)


#include "inexor/util/InexorException.hpp"

/// Version of the ICOMMAND macro that automatically catches
/// errors and prints them to the console.
#define ICOMMANDERR(name, nargs, proto, b)                                                   \
  ICOMMAND(name, nargs, proto,                                                               \
      try {                                                                                  \
          b;                                                                                 \
      } catch (inexor::util::InexorException &e) {                                           \
          Log.std->error("EXCEPTION in {0}: {1}", #name, e.message());         \
      }                                                                                      \
  )

enum { KR_CONSOLE = 1<<0, KR_GUI = 1<<1, KR_EDITMODE = 1<<2 };


extern hashnameset<ident> idents;
extern int identflags;

extern void clearoverrides();
extern void writecfg(const char *name = nullptr);
extern void loadhistory();
extern void writehistory();

extern void checksleep(int millis);
extern void clearsleep(bool clearoverrides = true);


