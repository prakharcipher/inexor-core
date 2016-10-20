#pragma once

#include <boost/algorithm/clamp.hpp>
using boost::algorithm::clamp;

extern SharedVar<int> worldsize;

/// 
struct selinfo
{
    int corner;
    int cx, cxs, cy, cys;
    ivec o, s;
    int grid, orient;
    selinfo() : corner(0), cx(0), cxs(0), cy(0), cys(0), o(0, 0, 0), s(0, 0, 0), grid(8), orient(0) {}
    int size() const    { return s.x*s.y*s.z; }
    int us(int d) const { return s[d]*grid; }
    bool operator==(const selinfo &sel) const { return o==sel.o && s==sel.s && grid==sel.grid && orient==sel.orient; }
    bool validate()
    {
        if(grid <= 0 || grid >= worldsize) return false;
        if(o.x >= worldsize || o.y >= worldsize || o.z >= worldsize) return false;
        if(o.x < 0) { s.x -= (grid - 1 - o.x)/grid; o.x = 0; } 
        if(o.y < 0) { s.y -= (grid - 1 - o.y)/grid; o.y = 0; } 
        if(o.z < 0) { s.z -= (grid - 1 - o.z)/grid; o.z = 0; } 
        s.x = clamp(s.x, 0, (worldsize - o.x)/grid);
        s.y = clamp(s.y, 0, (worldsize - o.y)/grid);
        s.z = clamp(s.z, 0, (worldsize - o.z)/grid);
        return s.x > 0 && s.y > 0 && s.z > 0;
    }
};
