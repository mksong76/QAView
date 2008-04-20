#ifndef __AF_FILE_H__
#define __AF_FILE_H__

#include "zf_port.h"

typedef struct _AF_FileInternal *AF_FILE;

typedef struct _AF_FileInternal{
    ZF_INT32    (*read)(AF_FILE in_fd, ZF_UCHAR8 *out_buf, ZF_UINT32 in_size);
    ZF_INT32    (*write)(AF_FILE in_fd, const ZF_UCHAR8 *in_buf,
            ZF_UINT32 in_length);
    ZF_INT32    (*seek)(AF_FILE in_fd, ZF_INT32 in_offset, ZF_INT32 in_whence);
    ZF_INT32    (*close)(AF_FILE in_fd);
    AF_FILE     (*dup)(AF_FILE in_fd);
    ZF_INT32    (*size)(AF_FILE in_fd);
} AF_File;

#define AF_SEEK_SET     0
#define AF_SEEK_CUR     1
#define AF_SEEK_END     2

/*
#define af_seek(in_fd, in_offset, in_whence) \
    ((in_fd)->seek((in_fd), (in_offset), (in_whence)))
#define af_read(in_fd, out_buf, in_size) \
    ((in_fd)->read((in_fd), (out_buf), (in_size)))
#define af_write(in_fd, in_buf, in_length) \
    ((in_fd)->write((in_fd), (in_buf), (in_length)))
#define af_close(in_fd) \
    ((in_fd)->close((in_fd)))
#define af_dup(in_fd) \
    ((in_fd)->dup((in_fd)))
#define af_size(in_fd) \
    ((in_fd)->size((in_fd)))
    */

#ifdef  __cplusplus
extern "C" {
#endif
    ZF_INT32    af_seek(AF_FILE in_fd, ZF_INT32 in_offset, ZF_INT32 in_whence);
    ZF_INT32    af_read(AF_FILE in_fd, ZF_UCHAR8 *out_buf, ZF_UINT32 in_size);
    ZF_INT32    af_write(AF_FILE in_fd, const ZF_UCHAR8 *in_buf,
            ZF_UINT32 in_length);
    ZF_INT32    af_close(AF_FILE in_fd);
    AF_FILE     af_dup(AF_FILE in_fd);
    ZF_INT32    af_size(AF_FILE in_fd);
    AF_FILE     af_openSys(const ZF_CHAR8 *in_fname);
#ifdef  __cplusplus
}
#endif

#endif
