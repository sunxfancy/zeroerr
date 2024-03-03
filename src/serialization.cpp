#include "zeroerr/internal/serialization.h"

namespace zeroerr
{
    
IRObject* IRObject::alloc(unsigned size)
{
    IRObject* list = new IRObject[size];
    list[0].i = size;
    return list+1;
}

char* IRObject::alloc_str(unsigned size)
{
    char* s = new char[size+1];
    s[size] = 0;
    return s;
}


} // namespace zeroerr
