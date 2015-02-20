//------------------------------------------------------------------------------
#ifndef TimerIntervalH
#define TimerIntervalH
//------------------------------------------------------------------------------
#include <time.h>
#include <stdint.h>
//------------------------------------------------------------------------------
#pragma pack(push,1)
//------------------------------------------------------------------------------
class TTimeInterval
{
protected:
    uint64_t Total;
    uint64_t Time1;
    uint64_t Time2;

    uint64_t Clock()
    {
        timespec tm;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&tm);
        return 1e9*tm.tv_sec + tm.tv_nsec;
    }

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
        Time1 = Clock();
    }

    void FullStart()
    {
        Total = 0;
        Time1 = Clock();
    }

    void Pause()
    {
        Time2  = Clock();
        Total += Time2-Time1;
    }

    float GetTime()
    {
        long double t = Total;
        t *= 1e-9;
        return t;
    }
};
//------------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------------
#endif // Exclusive Include
//------------------------------------------------------------------------------
