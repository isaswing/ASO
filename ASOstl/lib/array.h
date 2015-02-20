//------------------------------------------------------------------------------
#pragma once
#ifndef ArrayH
#define ArrayH
//------------------------------------------------------------------------------
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sortBase.h"
#define ARRAY_DEFINE_OPERATION T &operator[](int i){assert(i>=0&&i<len);return Obj(i);}
//------------------------------------------------------------------------------
#ifndef NULL
#define NULL 0
#endif //NULL
//------------------------------------------------------------------------------
#pragma pack(push,1)
//------------------------------------------------------------------------------
template<class T>class TArrayBase
{
public:
    T *img;
    int len;
    int Use_MAlloc;

    TArrayBase();
    TArrayBase(int num,int init=0);
    ~TArrayBase();

    void Close();
    T &Obj(int i);
    void Alloc(int num);
    void InitZero();
    void SetLength(int NewLen)
    {
        assert(img);
        assert(len>=NewLen);
        assert(Use_MAlloc);
        img = (T*)realloc(img,NewLen*sizeof(T));
        len = NewLen;
    }

    TArrayBase<T> &operator=(TArrayBase<T> &o);

    int Size();

    bool Grow(int Extend);
    bool Small(int NewLen)
    {
        assert(Use_MAlloc);
        T *tmp = (T *)(realloc(img,NewLen*sizeof(T)));
        if(tmp==NULL)
            return false;
        len = NewLen;
        return true;
    }

    void Assign(TArrayBase<T> &o,int l=-1)
    {
        if( (l<0) || (l>o.len) )
            l = o.len;

        Alloc(l);

        memcpy(img,o.img,sizeof(T)*l);
    }

    void Assign(T *o,int l)
    {
        Alloc(l);

        memcpy(img,o,sizeof(T)*l);
    }

    T *BSearch(const void *key,TSortCmp cmp,int n=-1)
    {
        if(n<=0)
            n = len;

        return (T*)(bsearch(key,img,n,sizeof(img[0]),cmp));
    }

    void Sort(TSortCmp cmp)
    {
        qsort(img,len,sizeof(img[0]),cmp);
    }

    ARRAY_DEFINE_OPERATION
};
//------------------------------------------------------------------------------
template<class T>inline TArrayBase<T>::TArrayBase()
{
    img = NULL;
    len = 0;

    Use_MAlloc = 0;
}
//------------------------------------------------------------------------------
template<class T>inline TArrayBase<T>::TArrayBase(int num,int init)
{
    img = NULL;
    len = 0;

    Use_MAlloc = 0;

    Alloc(num);
    if(init)
    	InitZero();
}
//------------------------------------------------------------------------------
template<class T>inline TArrayBase<T>::~TArrayBase()
{
    Close();
}
//------------------------------------------------------------------------------
template<class T>inline void TArrayBase<T>::Close()
{
    if(img)
    {
        if(Use_MAlloc)
            free(img);
        else
            delete []img;
        img = NULL;
    }
    len = 0;
}
//------------------------------------------------------------------------------
template<class T>inline T &TArrayBase<T>::Obj(int i)
{
    assert(img);
    assert(i>=0&&i<len);
    return img[i];
}
//------------------------------------------------------------------------------
template<class T>inline void TArrayBase<T>::Alloc(int num)
{
    Close();
    if(num<=0)
        return;
    if(Use_MAlloc)
        img = (T*)malloc(num*sizeof(T));
    else
    	img = new T[num];
    assert(img);
    len = num;
}
//------------------------------------------------------------------------------
template<class T>inline bool TArrayBase<T>::Grow(int Extend)
{
//	assert(img);
    assert(Use_MAlloc);
    
    T *t = (T*)(realloc(img,(len+Extend)*sizeof(T)));
    if(t==NULL)
        return false;
    img  = t;
    len += Extend;
    return true;
}
//------------------------------------------------------------------------------
template<class T>inline void TArrayBase<T>::InitZero()
{
    assert(img);
    memset(img,0,sizeof(T)*len);
}
//------------------------------------------------------------------------------
template<class T>TArrayBase<T> &TArrayBase<T>::operator=(TArrayBase<T> &o)
{
    Alloc(o.len);
    for(int i=0;i<len;i++)
        img[i] = o[i];
    return *this;
}
//------------------------------------------------------------------------------
template<class T> inline int TArrayBase<T>::Size()
{
    assert(img);
    return len*sizeof(T);
}
//------------------------------------------------------------------------------
template<class T>int Largest(TArrayBase<T> &a,int num=-1)
{
    if(num<=0)
        num = a.len;

    T tmp;
    tmp = a[0];
    int index = 0;
    for(int i=1;i<num;i++)
    {
        if(a[i]>tmp)
        {
            tmp = a[i];
            index = i;
        }
    }
    return index;
}
//------------------------------------------------------------------------------
template<class T>int Largest(T *a,int num)
{
    T tmp;
    tmp = a[0];
    int index = 0;
    for(int i=1;i<num;i++)
    {
        if(a[i]>tmp)
        {
            tmp = a[i];
            index = i;
        }
    }
    return index;
}
//------------------------------------------------------------------------------
template<class T>int Smallest(TArrayBase<T> &a,int num=-1)
{
    if(num<=0)
        num = a.len;

    T tmp;
    tmp = a[0];
    int index = 0;
    for(int i=1;i<num;i++)
    {
        if(a[i]<tmp)
        {
            tmp = a[i];
            index = i;
        }
    }
    return index;
}
//------------------------------------------------------------------------------
template<class T>void MinMax(T *a,int &MinIndex,int &MaxIndex,int num)
{
    T tmpMin,tmpMax;
    tmpMin = a[0];
    tmpMax = a[0];
    MinIndex = 0;
    MaxIndex = 0;
    for(int i=1;i<num;i++)
    {
        if(a[i]<tmpMin)
        {
            tmpMin   = a[i];
            MinIndex = i;
        }
        else if(a[i]>tmpMax)
        {
            tmpMax   = a[i];
            MaxIndex = i;
        }
    }
}
//------------------------------------------------------------------------------
template<class T>inline void MinMax(TArrayBase<T> &a,int &MinIndex,int &MaxIndex,int num=-1)
{
    if(num<=0)
        num = a.len;

    assert(a.img);
    assert(a.len>=num);

    MinMax(a.img,MinIndex,MaxIndex,num);
}
//------------------------------------------------------------------------------
template<class T>inline void Normalize(TArrayBase<T> &a, T l, T h, int num=-1)
{
    if (num<=0)
        num = a.len;

    int MinIndex,MaxIndex;
    MinMax(a,MinIndex,MaxIndex,num);
    T vMin, vLen;
    vMin = a[MinIndex];
    vLen = a[MaxIndex] - vMin;
    for(int i=0;i<num;i++)
        a[i] = (a[i]-vMin)*h/vLen + l;
}
//------------------------------------------------------------------------------
template<class T>inline void Normalize(T *a, T l, T h, int num)
{
    assert(num>0);
    
    TArrayBase<T> tmp;
    tmp.img = a;
    tmp.len = num;
    
    Normalize(tmp,l,h,tmp.len);
    
    tmp.img = NULL;
    tmp.len = 0;
}
//------------------------------------------------------------------------------
template<class T>void Total(TArrayBase<T> &a,T &ret,int num=-1)
{
    if(num<=0)
        num = a.len;

    ret = a[0];
    for(int i=1;i<num;i++)
    	ret += a[i];
}
//------------------------------------------------------------------------------
template<class T>double EntropyE(TArrayBase<T> &a,int num=-1)         //base e entropy
{
    if(num<=0)
        num = a.len;

    T sum;

    Total(a,sum,num);

    double e = 0;
    double m = 1.0/sum;

    for(int i=0;i<num;i++)
    {
        double t = a[i]*m;

        if(t>1e-30)
            e -= t*log(t);
    }

    return e;
}
//------------------------------------------------------------------------------
template<class T> inline double NormEntropy(TArrayBase<T> &a,int num=-1)
{
    if(num<=0)
        num = a.len;

    return EntropyE(a,num)/log(num);
}
//------------------------------------------------------------------------------
template<class T> inline double Entropy(TArrayBase<T> &a,int num=-1)
{
    return EntropyE(a,num)*1.4426950408889634073599246810019;          //log2(e)
}
//------------------------------------------------------------------------------
template<class T>inline void Mean(TArrayBase<T> &a,T &ret,int num=-1)
{
    if(num<=0)
        num = a.len;

    Total(a,ret,num);
    ret /= num;
}
//------------------------------------------------------------------------------
template<class T>
class TDynArrayBase:public TArrayBase<T>
{
public:
#ifdef __GNUC__
    using TArrayBase<T>::img;
    using TArrayBase<T>::len;
    using TArrayBase<T>::Use_MAlloc;
    using TArrayBase<T>::Grow;
#endif
	
public:
    int Number;

    TDynArrayBase()
    {
        Number = 0;
    }

    void operator += (T &o)
    {
        New() = o;
    }

    T &New(int MaxCap=10)
    {
        if(Use_MAlloc)
        {
            if(Number>=len)
                Grow(Number-len+MaxCap);
        }
        else
        {
            assert(Number<len);
        }

        Number ++;
        return img[Number-1];
    }

    T &Next(int MaxCap=10)
    {
        if(Use_MAlloc)
        {
            if(Number>=len)
                Grow(Number-len+MaxCap);
        }
        else
        {
            assert(Number<len);
        }

        return img[Number];
    }

    void Expand(int num)
    {
        Grow(num);    
    }

    void Close()
    {
        TArrayBase<T>::Close();
        Number = 0;
    }

    void operator=(TDynArrayBase<T> &o);

    int Size()
    {
        return Number*sizeof(T);
    }

    void Alloc(int _len)
    {
        Number = 0;
        TArrayBase<T>::Alloc(_len);
    }

    void FastInsert(int pos,int n=1)
    {
        if(Use_MAlloc)
        {
            if((Number+n)>=len)
                Grow(Number+n-len+1);
        }
        else
        {
            assert((Number+n)<=len);
        }
        memmove(img+pos+n,img+pos,(Number-pos)*sizeof(T));
        Number += n;
    }

    void TrueFastInsert(int pos,int n=1)
    {
        if (Use_MAlloc)
        {
            if ((Number+n)>=len)
                Grow(Number+n-len+1);
        }
        else
        {
            assert((Number+n)<=len);
        }

        for(int i=Number-1;i>=pos;i--)
            img[i+n] = img[i];
        Number += n;
    }

    void FastDel(int pos,int n=1)
    {
        assert(Number>=n);
        memmove(img+pos,img+pos+n,(Number-pos-n)*sizeof(T));
        Number -= n;
    }

    void TrueFastDel(int pos,int n=1)
    {
        assert(Number>=n);

        if(pos==(Number-n))
        {
            Number -= n;
            return;
        }

        for(int i=0;i<Number-pos-n;i++)
        {
            img[pos+i] = img[pos+i+n];
        }

        Number -= n;
    }

    void Append(int n,int minCap=10)
    {
        if(Use_MAlloc)
        {
            if((Number+n)>=len)
                Grow(Number+n-len+minCap);
        }
        else
        {
            assert((Number+n)<=len);
        }
            
        Number += n;
    }

    void Assign(T *o,int l,int more=0)
    {
        Alloc(l+more);

        memcpy(img,o,sizeof(T)*l);
        Number = l;
    }

    void Assign(TDynArrayBase<T> &o,int more=0)
    {
        Alloc(o.len+more);
        
        memcpy(img,o.img,sizeof(T)*o.len);
        Number = o.Number;
    }

    T *BSearch(const void *key,TSortCmp cmp,int n=-1)
    {
        if(n<=0)
            n = Number;

        return (T*)(bsearch(key,img,n,sizeof(img[0]),cmp));
    }

    T *Search(const void *key,TSortCmp cmp,int n=-1)
    {
        if(n<=0)
            n = Number;

        for(int i=0;i<n;i++)
        {
            if(cmp(key,img+i)==0)
                return img+i;
        }

        return NULL;
    }

    void FastAppend(T &Val, bool canRepeat=true)
    {
        if(Number==0 || canRepeat==true)
        {
            New() = Val;
            return;
        }

        bool find = false; 

        for(int i=0;i<Number;i++)
        {
            if (img[i] == Val)
            {
                find = true;
                break;
            }
        }
            
        if (find==false)
            New() = Val;
    }

    int BInsert(T &Val,TSortCmp cmp,bool canRepeat=true)
    {
        if(Number==0)
        {
            New() = Val;
            return Number-1;
        }

        int ret = cmp(&Val,img+(Number-1));

        if(ret>=0)
        {
            if( (ret!=0) || canRepeat )
                New() = Val;

            return Number-1;
        }

        int low = 0;
        int high = Number;

        while(low<high)
        {
            int mid = ((low+high)>>1);

            ret = cmp(&Val,img+mid);

            if(ret==0)
            {
                if(canRepeat)
                {
                    FastInsert(mid);
                    img[mid] = Val;
                }

                return mid;
            }

            if(ret<0)
                high = mid;
            else
                low = mid+1;
        }

        if(cmp(&Val,img+low)>0)
            low ++;

        FastInsert(low);
        img[low] = Val;

        return low;
    }

    void Sort(TSortCmp cmp)
    {
        qsort(img,Number,sizeof(img[0]),cmp);
    }

    void Splice(int pos, int n, T *o, int l)
    {
        if (n>l)
        {
            int i = 0;
                
            while(i<l)
            {
                img[pos+i] = o[i];
                i ++;
            }

            TrueFastDel(pos+i,n-i);
        }
        else
        {
            if (n<l)
            {
                if (Use_MAlloc)
                {
                    if ((Number+l-n)>=len)
                        Grow(Number+l-n);
                }
                else
                {
                    assert((Number+l-n)<=len);
                }
            }

            int i = 0;

            while(i<n)
            {
                img[pos+i] = o[i];
                i ++;
            }

            TrueFastInsert(pos+i,l-n);

            while(i<l)
            {
                img[pos+i] = o[i];
                i ++;
            }
        }
    }
};
//------------------------------------------------------------------------------
template<class T>void TDynArrayBase<T>::operator=(TDynArrayBase<T> &o)
{
    if (o.Number==0)
    {
        Close();
        return;
    }

    Alloc(o.Number);
    for(int i=0;i<o.Number;i++)
        img[i] = o[i];
    Number = o.Number;
}
//------------------------------------------------------------------------------
template<class T>
class TAlcArrayBase:public TDynArrayBase<T>
{
public:
#ifdef __GNUC__
    using TArrayBase<T>::Use_MAlloc;
#endif
	
public:
    TAlcArrayBase()
    {
        Use_MAlloc = 1;
    }
};    
//------------------------------------------------------------------------------
template<class T>
class TDynPointerArray:public TAlcArrayBase< T*>
{
public:
#ifdef __GNUC__
    using TArrayBase< T*>::Obj;
    using TArrayBase< T*>::InitZero;
    using TDynArrayBase< T*>::Append;
    using TAlcArrayBase< T*>::Number;
    using TAlcArrayBase< T*>::img;
    using TAlcArrayBase< T*>::len;
#endif
	
public:
    ~TDynPointerArray()
    {
        Close();
    }

    void Close()
    {
        for(int i=0;i<Number;i++)
        {
            if(img[i])
                delete img[i];
        }

        TDynArrayBase<T*>::Close();
    }

    void Alloc(int num)
    {
        Close();
        TDynArrayBase<T*>::Alloc(num);
		InitZero();
    }

    void FastDelPointer(int pos,int n=1)
    {
        assert(Number>=n);
        for(int i=0;i<n;++i)
        {
            if (img[pos+i])
                delete img[pos+i];
        }
        memmove(img+pos,img+pos+n,(Number-pos-n)*sizeof(T*));
        Number -= n;
    }
};
//------------------------------------------------------------------------------
typedef TArrayBase<unsigned char>    TGrayArray;
typedef TDynArrayBase<unsigned char> TDynGrayArray;
typedef TAlcArrayBase<unsigned char> TAlcGrayArray;
//------------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------------
#endif // Exclusive Include
//------------------------------------------------------------------------------
