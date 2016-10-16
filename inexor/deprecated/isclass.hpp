#pragma once

/// TODO: what is this?
template<class T> struct isclass
{
    template<class C> static char test(void (C::*)(void));
    template<class C> static int test(...);
    enum 
    {
        yes = sizeof(test<T>(0)) == 1 ? 1 : 0,
        no = yes^1
    };
};
