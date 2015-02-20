//---------------------------------------------------------------------------

#ifndef FileIOH
#define FileIOH
//---------------------------------------------------------------------------
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
//---------------------------------------------------------------------------
class FileIO
{
public:
    FILE* f;

    FileIO()
    {
        f = NULL;
    }

    ~FileIO()
    {
        Close();
    }

    bool Open(const char *fileName, const char* mode = "wb")
    {
        f = fopen(fileName, mode);

        if (f == NULL)
            return false;

        return true;
    }

    void Close()
    {
        if (f != NULL)
        {
            fclose(f);
            f = NULL;
        }
    }

    long size()
    {
        if (f != NULL)
        {
        	struct stat tmp;
        	fstat(fileno(f),&tmp);
        	return tmp.st_size;
        }

        return -1;
    }

    bool Read(void* buffer, int size, int length)
    {
        if (f != NULL)
        {
            if (fread(buffer, size, length, f) == length)
            {
                return true;
            }
        }

        return false;
    }

    bool ReadLine(char *buffer)
    {
        if (f != NULL)
        {
                if (fgets(buffer, INT_MAX, f) != NULL)
                {
                        return true;
                }
        }

        return false;
    }

    bool Write(void* buffer, int size, int length)
    {
        if (f != NULL)
        {
            if (fwrite(buffer, size, length, f) == length)
            {
                return true;
            }
        }

        return false;
    }

    bool Print(const char *buffer, ...)
    {
        va_list ap;
        va_start(ap, buffer);
        va_end(ap);
        if (vfprintf(f, buffer, ap) == -1)
        {
                return false;
        }
        return true;
    }

	void Flush()
	{
		assert(f);
		fflush(f);
	}

};
//---------------------------------------------------------------------------
#endif
