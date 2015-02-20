//------------------------------------------------------------------------------
#ifndef sortBaseH
#define sortBaseH
//------------------------------------------------------------------------------
#include <memory.h>
//------------------------------------------------------------------------------
#pragma pack(push,1)
//------------------------------------------------------------------------------
typedef int (*TSortCmp)(const void *r1,const void *r2);
//------------------------------------------------------------------------------
class TSortBase
{
public:
    static int IntCmpInc(const void *r1, const void *r2)
    {
        return (*(const int*)r1) - (*(const int*)r2);
    }

    static int IntCmpDec(const void *r1, const void *r2)
    {
        return (*(const int*)r2) - (*(const int*)r1);
    }

    static int UIntCmpInc(const void *r1, const void *r2)
    {
        return (*(const unsigned*)r1) - (*(const unsigned*)r2);
    }

    static int UIntCmpDec(const void *r1, const void *r2)
    {
        return (*(const unsigned*)r2) - (*(const unsigned*)r1);
    }

    static int FloatCmpInc(const void *r1, const void *r2)
    {
        return (*(const float*)r1) - (*(const float*)r2);
    }
    static int FloatCmpDec(const void *r1, const void *r2)
    {
        return (*(const float*)r2) - (*(const float*)r1);
    }

    static int DoubleCmpInc(const void *r1, const void *r2)
    {
        return (*(const double*)r1) - (*(const double*)r2);
    }
    static int DoubleCmpDec(const void *r1, const void *r2)
    {
        return (*(const double*)r2) - (*(const double*)r1);
    }

    static int StrCmpInc(const void *r1, const void *r2)
    {
        return strcmp( (*(const char**)r1), (*(const char**)r2) );
    }
};
//------------------------------------------------------------------------------
inline void XchgStorage(void *d1,void *d2,void *buff,int w)
{
    memcpy(buff,d1,w);
    memcpy(d1,d2,w);
    memcpy(d2,buff,w);
}
//------------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------------
#endif // Exclusive Include
//------------------------------------------------------------------------------

