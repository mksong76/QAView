#ifndef __ZIP_FILE_PORTING_H__
#define __ZIP_FILE_PORTING_H__

typedef signed int      ZF_INT32;
typedef unsigned int    ZF_UINT32;

typedef signed short    ZF_INT16;
typedef unsigned short  ZF_UINT16;

typedef signed char     ZF_INT8;
typedef unsigned char   ZF_UINT8;

typedef signed char     ZF_CHAR8;
typedef unsigned char   ZF_UCHAR8;

typedef unsigned char   ZF_BOOL;
#define ZF_FALSE        0
#define ZF_TRUE         1

#define ZF_SEEK_SET     SEEK_SET
#define ZF_SEEK_CUR     SEEK_CUR
#define ZF_SEEK_END     SEEK_END

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
/**
 * Data structure for file.
 */
typedef int             ZF_FILE;
typedef struct stat     ZF_Stat;
#define ZF_RDONLY       O_RDONLY
#define ZF_WRONLY       O_WRONLY
#define ZF_RDWR         O_RDWR
#define ZF_CREAT        O_CREAT
#define ZF_EXCL         O_EXCL
#define ZF_TRUNC        O_TRUNC
#define zf_open(name, flags)    open(name, flags)
#define zf_seek(x,y,z)          lseek(x,y,z)
#define zf_read(x,y,z)          read(x,y,z)
#define zf_write(x,y,z)         write(x,y,z)
#define zf_fstat(x,y)           fstat(x,y)
#define zf_close(fd)            close(fd)

#define zf_error(x)             printf x

#define zf_malloc(x)            malloc(x)
#define zf_calloc(x,y)          calloc(x,y)
#define zf_free(x)              free(x)
#define zf_strdup(x)            strdup(x)
#define zf_realloc(x,y)         realloc(x, y)

#endif
