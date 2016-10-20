#pragma once

// nasty macros for registering script functions, abuses globals to avoid excessive infrastructure
#define KEYWORD(name, type) UNUSED static bool __dummy_##name = addkeyword(type, #name)
#define COMMANDN(name, fun, nargs) UNUSED static bool __dummy_##fun = addcommand(#name, (identfun)fun, nargs)
#define COMMAND(name, nargs) COMMANDN(name, name, nargs)

#define _VAR(name, global, min, cur, max, persist) INEXOR_SHARED_TREE(/cubescript/name) SharedVar<int> global((int)cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, NULL, persist)
/// Version of VAR which doesnt sync with the tree api.
#define _VAR_NOSYNC(name, global, min, cur, max, persist) SharedVar<int> global((int)cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, NULL, persist)

#define VARN(name, global, min, cur, max) _VAR(name, global, min, cur, max, 0)
#define VARN_NOSYNC(name, global, min, cur, max) _VAR_NOSYNC(name, global, min, cur, max, 0)
#define VARNP(name, global, min, cur, max) _VAR(name, global, min, cur, max, IDF_PERSIST)
#define VARNR(name, global, min, cur, max) _VAR(name, global, min, cur, max, IDF_OVERRIDE)
#define VAR(name, min, cur, max) _VAR(name, name, min, cur, max, 0)
#define VAR_NOSYNC(name, min, cur, max) _VAR_NOSYNC(name, name, min, cur, max, 0)
#define VARP(name, min, cur, max) _VAR(name, name, min, cur, max, IDF_PERSIST)
#define VARR(name, min, cur, max) _VAR(name, name, min, cur, max, IDF_OVERRIDE)
#define _VARF(name, global, min, cur, max, body, persist)  void var_##name(); INEXOR_SHARED_TREE(/cubescript/name) SharedVar<int> global((int)cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, var_##name, persist); void var_##name() { body; }
#define VARFN(name, global, min, cur, max, body) _VARF(name, global, min, cur, max, body, 0)
#define VARF(name, min, cur, max, body) _VARF(name, name, min, cur, max, body, 0)
#define VARFP(name, min, cur, max, body) _VARF(name, name, min, cur, max, body, IDF_PERSIST)
#define VARFR(name, min, cur, max, body) _VARF(name, name, min, cur, max, body, IDF_OVERRIDE)

#define _HVAR(name, global, min, cur, max, persist) INEXOR_SHARED_TREE(/cubescript/name) SharedVar<int> global(cur); UNUSED int dummy_register_##global = variable(#name, min, cur, max, &global, NULL, persist | IDF_HEX)
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

#define _FVAR(name, global, min, cur, max, persist) INEXOR_SHARED_TREE(/cubescript/name) SharedVar<float> global((float)cur); UNUSED float dummy_register_##global = fvariable(#name, min, cur, max, &global, NULL, persist)
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


#define _SVAR(name, global, cur, persist) INEXOR_SHARED_TREE(/cubescript/name) SharedVar<char*> global; UNUSED char* dummy_register_##global = *global = (char*)svariable(#name, cur, &global, NULL, persist)
#define SVARN(name, global, cur) _SVAR(name, global, cur, 0)
#define SVARNP(name, global, cur) _SVAR(name, global, cur, IDF_PERSIST)
#define SVARNR(name, global, cur) _SVAR(name, global, cur, IDF_OVERRIDE)
#define SVAR(name, cur) _SVAR(name, name, cur, 0)
#define SVARP(name, cur) _SVAR(name, name, cur, IDF_PERSIST)
#define SVARR(name, cur) _SVAR(name, name, cur, IDF_OVERRIDE)
#define _SVARF(name, global, cur, body, persist) void var_##name(); INEXOR_SHARED_TREE(/cubescript/name) SharedVar<char*> global; UNUSED char* dummy_register_##global = *global = (char*)svariable(#name, cur, &global, var_##name, persist); void var_##name() { body; }
#define SVARFN(name, global, cur, body) _SVARF(name, global, cur, body, 0)
#define SVARF(name, cur, body) _SVARF(name, name, cur, body, 0)
#define SVARFP(name, cur, body) _SVARF(name, name, cur, body, IDF_PERSIST)
#define SVARFR(name, cur, body) _SVARF(name, name, cur, body, IDF_OVERRIDE)

// anonymous inline commands, uses nasty template trick with line numbers to keep names unique
#define ICOMMANDNS(name, cmdname, nargs, proto, b) template<int N> struct cmdname; template<> struct cmdname<__LINE__> { static bool init; static void run proto; }; bool cmdname<__LINE__>::init = addcommand(name, (identfun)cmdname<__LINE__>::run, nargs); void cmdname<__LINE__>::run proto \
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
          spdlog::get("global")->error() << "EXCEPTION in " << #name << ": " << e.message(); \
      }                                                                                      \
  )

