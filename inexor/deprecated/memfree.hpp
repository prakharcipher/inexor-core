#pragma once


/// Delete Pointer, Wrapper around delete, sets pointer to NULL afterwards(!).
#define DELETEP(p) if(p) { delete   p; p = 0; }

/// Delete Array, Wrapper around delete[], sets pointer to NULL afterwards(!).
#define DELETEA(p) if(p) { delete[] p; p = 0; }
