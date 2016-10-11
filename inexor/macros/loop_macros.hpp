#pragma once

namespace inexor {
namespace server {

    /// TODO: remove these duplicated macros from the server 
    /// code in another refactoring branch and create pull request
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
    /// from tools.hpp
    #define loopv(v)    for(int i = 0; i<(v).length(); i++)
    #define loopvj(v)   for(int j = 0; j<(v).length(); j++)
    #define loopvk(v)   for(int k = 0; k<(v).length(); k++)
    #define loopvrev(v) for(int i = (v).length()-1; i>=0; i--)

};
};
