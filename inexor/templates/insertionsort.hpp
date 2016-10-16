#pragma once

/// template implementation of insertionsort
template<class T, class F>
static inline void insertionsort(T *start, T *end, F fun)
{
    for(T *i = start+1; i < end; i++)
    {
        if(fun(*i, i[-1]))
        {
            T tmp = *i;
            *i = i[-1];
            T *j = i-1;
            for(; j > start && fun(tmp, j[-1]); --j)
                *j = j[-1];
            *j = tmp;
        }
    }

}

template<class T, class F>
static inline void insertionsort(T *buf, int n, F fun)
{
    insertionsort(buf, buf+n, fun);
}

template<class T>
static inline void insertionsort(T *buf, int n)
{
    insertionsort(buf, buf+n, sortless());
}
