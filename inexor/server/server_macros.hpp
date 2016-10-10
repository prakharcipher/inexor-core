#pragma once

namespace inexor {
namespace server {

    #define loop(v,m) for(int v = 0; v < int(m); ++v)
    #define loopi(m) loop(i,m)
    #define loopj(m) loop(j,m)
    #define loopk(m) loop(k,m)
    #define loopl(m) loop(l,m)
    #define looprev(v,m) for(int v = int(m); --v >= 0;)
    #define loopirev(m) looprev(i,m)
    #define loopjrev(m) looprev(j,m)
    #define loopkrev(m) looprev(k,m)
    #define looplrev(m) looprev(l,m)


    /// Delete Pointer, Wrapper around delete, sets pointer to NULL afterwards(!).
    #define DELETEP(p) if(p) { delete   p; p = 0; }

    /// Delete Array, Wrapper around delete[], sets pointer to NULL afterwards(!).
    #define DELETEA(p) if(p) { delete[] p; p = 0; }

    // some important mathematical constants:
    #define PI  (3.1415927f)
    #define PI2 (2*PI)
    #define SQRT2 (1.4142136f)
    #define SQRT3 (1.7320508f)
    #define RAD (PI / 180.0f)

};
};
