#pragma once

// some important mathematical constants:
#define PI  (3.1415927f)
#define PI2 (2*PI)
#define SQRT2 (1.4142136f)
#define SQRT3 (1.7320508f)
#define RAD (PI / 180.0f)

// some more (precise) mathematical constants
#ifdef WIN32
  #ifndef M_PI
    #define M_PI 3.14159265358979323846
  #endif
  #ifndef M_LN2
    #define M_LN2 0.693147180559945309417
  #endif
  /// Compare Strings, ignore case.
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
  /// Path divide character, \ on win, otherwise /.
  #define PATHDIV '\\'
#else
  // adapt macros to OS specifications
  #define __cdecl
  #define _vsnprintf vsnprintf
  /// Path divide character, \ on win, otherwise /.
  #define PATHDIV '/'
#endif
