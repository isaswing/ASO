//------------------------------------------------------------------------------
#ifndef cFileIOFlagsH
#define cFileIOFlagsH
//------------------------------------------------------------------------------
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
//------------------------------------------------------------------------------
#pragma pack(push,1)
//------------------------------------------------------------------------------
enum TCFileIOFlags
{
    amRead      = O_RDONLY,
    amWrite     = O_WRONLY,
    amReadWrite = O_RDWR,

    cmReadUsr      = S_IRUSR,
    cmWriteUsr     = S_IWUSR,
    cmReadWriteUsr = cmReadUsr|cmWriteUsr,
    cmExecUsr	   = S_IXUSR,
    cmReadGrp	   = S_IRGRP,
    cmWriteGrp	   = S_IWGRP,
    cmReadWriteGrp = cmReadGrp|cmWriteGrp,
    cmExecGrp      = S_IXGRP,
    cmReadOth      = S_IROTH,
    cmWriteOth     = S_IWOTH,
    cmReadWriteOth = cmReadOth|cmWriteOth,
    cmExecOth      = S_IXOTH,

    omOpen       = 0,
    omCreate     = O_CREAT,
    omCreateNew  = O_CREAT|O_TRUNC,
    omOpenAlways = O_CREAT,
    omAppend     = O_APPEND,

    mmBegin      = SEEK_SET,
    mmCurrent    = SEEK_CUR,
    mmEnd        = SEEK_END
};
//------------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------------
#endif // Exclusive Include
//------------------------------------------------------------------------------
