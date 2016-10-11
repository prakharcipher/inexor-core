#pragma once

namespace inexor {
namespace server {

    /// 
    template<typename T>
    T clamp(const T &val, const T &min, const T &max)
    {
        return std::max(min, std::min(max, val));
    }

};
};
