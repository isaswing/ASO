#ifndef TimerIntervalH
#define TimerIntervalH
//---------------------------------------------------------------------------
#include <windows.h>
#include <mmsystem.h>
//---------------------------------------------------------------------------
class TTimeInterval
{
protected:
    unsigned __int64 Total;
    DWORD            Time1;
    DWORD            Time2;
public:
    TTimeInterval()
    {
        Init();
    }

    void Init()
    {
        Total = 0;
    }

    void Start()
    {
        Time1 = timeGetTime();
    }

    void FullStart()
    {
        Total = 0;
        Time1 = timeGetTime();
    }

    void Pause()
    {
        Time2  = timeGetTime();
        Total += Time2-Time1;
    }

    float GetTime()
    {
        long double t = Total;
        t *= 1e-3;
        return (float)t;
    }
};
//---------------------------------------------------------------------------
#endif//TimerIntervalH
