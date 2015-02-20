//---------------------------------------------------------------------------
#ifndef StrUtilsH
#define StrUtilsH
//---------------------------------------------------------------------------
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "array.h"
//---------------------------------------------------------------------------
class TSmallStr
{
public:
	unsigned BuffLen;
	unsigned Len;
	char	*Buff;
	
public:
	TSmallStr()
	{
		BuffLen = 0;
		Len		= 0;
		Buff	= NULL;
	}
	
	TSmallStr(const TSmallStr &src)
	{
		BuffLen	= 0;
		Len		= 0;
		Buff	= NULL;

		*this = src;
	}
	
	TSmallStr(const char *src)
	{
		BuffLen = 0;
		Len		= 0;
		Buff	= NULL;

		*this = src;
	}
	
	~TSmallStr()
	{
		Close();
	}
	
	void Close()
	{
		if (Buff)
			free(Buff);
		
		BuffLen	= 0;
		Len		= 0;
		Buff	= NULL;
	}
	
	void Alloc(int len)
	{
		len = ((len+255)&(~255));
		
		if(len==0)
        {
            Close();
            return;
        }

        if(BuffLen==(unsigned)len)
        {
            Buff[0] = 0;
            Len     = 0;
            return;
        }

        Close();

        Buff    = (char*)(malloc(len*sizeof(char)));
        Len     = 0;
        BuffLen = len;

        Buff[0] = 0;
	}
	
	void Realloc(int len)
    {
        if(BuffLen>=(unsigned)len)
            return;

        len = ((len+255)&(~255));

        Buff    = (char*)(realloc(Buff,len*sizeof(char)));
        BuffLen = len;
    }

	char* operator()()
	{
		return Buff;
	}
	
	char& operator[](int inx)
	{
		assert(inx>=0&&inx<=(int)Len);
		return Buff[inx];
	}
	
	TSmallStr& operator=(const TSmallStr &src)
	{
		SetFast(src.Buff,src.Len);
		return *this;
	}
	
	TSmallStr& operator=(const char *src)
	{
		SetFast(src,strlen(src));
		return *this;
	}
	
	void operator+=(TSmallStr &src)
	{
		AddFast(src.Buff,src.Len);
	}
	
	void operator+=(const char *src)
	{
		AddFast(src,strlen(src));
	}
	
	void Expand(int moreLen)
	{
		Realloc(Len+moreLen);
	}

    void SetLen(int newLen)
    {
        assert(newLen>=0);
        Realloc(newLen+1);

        Buff[newLen] = 0;
        Len = newLen;
    }

    void SetFast(const char *src,int len,int MoreBuff=0)
    {
        if(src==NULL)
        {
            Close();
            return;
        }

        Alloc(len+1+MoreBuff);
        memcpy(Buff,src,len*sizeof(char));
        Len = len;

        Buff[len] = 0;
    }

	void Set(const char *hdr,int MoreBuff=0)
	{
		SetFast(hdr,strlen(hdr),MoreBuff);
	}

	void Set(TSmallStr &hdr,int MoreBuff=0)
	{
		SetFast(hdr.Buff,hdr.Len,MoreBuff);
	}
	
	void AddFast(const char *src,int len)
    {
        if(src==NULL)
            return;

        Realloc(Len+len+1);
        memcpy(Buff+Len,src,(len)*sizeof(char));
        Len += len;

        Buff[Len] = 0;
    }

    void AddFast(const char *src)
    {
        if (src==NULL)
            return;
        AddFast(src,strlen(src));
    }
	
	void AddFast(int c)
    {
        Expand(2);
        Buff[Len]   = (char)c;
        Buff[Len+1] = 0;
        Len         ++;
    }
	
	void ReCountLen()
    {
        Len = strlen(Buff);
    }
	
	void Format(const char *fmt,...)
	{
		Realloc(4096);
		
		va_list ap;
		va_start(ap,fmt);
		Len = vsprintf(Buff,fmt,ap);
		va_end(ap);
		
		assert(Len<BuffLen);
	}
	
	void Append(const char *fmt,...)
	{
		Realloc(Len+4096);
		
		va_list ap;
        va_start(ap, fmt);
        Len += vsprintf(Buff+Len,fmt,ap);
        va_end(ap);

        assert(Len<BuffLen);
	}

	void DelExt()
	{
		for(int i=Len-1;i>=0;i--)
		{
			if (Buff[i]=='.')
			{
				Len		= i;
				Buff[i] = 0;
				break;
			}
			
			if ( (Buff[i]=='\\') || (Buff[i]=='/') )
				break;
		}
	}

	void ChangeExt(const char *NewExt)
	{
		DelExt();
		*this += NewExt;
	}
};
//---------------------------------------------------------------------------
inline int StrSplit(const char *str, const char *pattern,...)
{
	int cnt = 0;

	va_list ap;
	va_start(ap,pattern);

	while(*str)
	{
		const char *p    = strstr(str,pattern);
		TSmallStr  *part = (TSmallStr*)va_arg(ap,int);

		if (p==NULL)
		{
			part->Set(p);
			cnt ++;
		}
		else
		{
			part->SetFast(str,p-str);
			str += part->Len + 1;
			cnt ++;
		}
	}

	va_end(ap);

	return cnt;
}
//---------------------------------------------------------------------------
inline void StrSplit(const char *str, const char *delim, TDynArrayBase<TSmallStr> &buff)
{
	const char *hd = str;
	const char *tl = strstr(hd,delim);

	while(hd&&*hd)
	{
		if (tl==NULL)
		{
			TSmallStr &Str = buff.New();
			Str.Set(hd);
			break;
		}

		TSmallStr &Str = buff.New();
		Str.SetFast(hd,tl-hd);

		hd = tl + 1;
		tl = strstr(hd,delim);
	}
}
//---------------------------------------------------------------------------
#endif // Exclusive Include
