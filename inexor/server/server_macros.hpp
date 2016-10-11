#pragma once

namespace inexor {
namespace server {

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
