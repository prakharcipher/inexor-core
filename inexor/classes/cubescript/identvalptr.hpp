#pragma once

/// 
union identvalptr
{
    SharedVar<int>   *i;   // ID_VAR
    SharedVar<float> *f;   // ID_FVAR
    SharedVar<char*> *s;   // ID_SVAR
};
