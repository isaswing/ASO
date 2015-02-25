#if defined(_WIN32) || defined(__CYGWIN32__)
#include "TimerInterval_win.h"
#else
#include "TimerInterval_linux.h"
#endif
