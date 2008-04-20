#include "af_file.h"


typedef struct {
    AF_File base;
    ZF_FILE     fd;
    ZF_CHAR8    *fname;
} AF_ZFile;

#define GET_FILE(x) ((AF_ZFile*)(x))
#define GET_FD(x)   (GET_FILE(x)->fd)

static ZF_INT32
_sys_read(AF_FILE in_fd, ZF_UCHAR8 *out_buf, ZF_UINT32 in_size)
{
    return zf_read(GET_FD(in_fd), out_buf, in_size);
}

static ZF_INT32
_sys_write(AF_FILE in_fd, const ZF_UCHAR8 *in_buf, ZF_UINT32 in_length)
{
    return zf_write(GET_FD(in_fd), in_buf, in_length);
}

static AF_FILE
_sys_dup(AF_FILE in_fd)
{
    AF_ZFile    *new_one = NULL;
    ZF_FILE     fd = -1;
    fd = zf_open(GET_FILE(in_fd)->fname, ZF_RDONLY);
    if (fd<0) return NULL;

    new_one = (AF_ZFile*)zf_malloc(sizeof(AF_ZFile));
    if (NULL==new_one) goto failure;

    new_one->fd = fd;
    new_one->fname = zf_strdup(GET_FILE(in_fd)->fname);
    if (NULL==new_one->fname) goto failure;

    memcpy(&new_one->base, in_fd, sizeof(new_one->base));

    return (AF_FILE)new_one;
failure:
    if (NULL!=new_one) {
        if (new_one->fname!=NULL) {
            zf_free(new_one->fname);
        }
        zf_free(new_one);
    }
    if (fd>=0) {
        zf_close(fd);
    }
}

static ZF_INT32
_sys_seek(AF_FILE in_fd, ZF_INT32 in_offset, ZF_INT32 in_whence)
{
    switch (in_whence) {
        case AF_SEEK_SET:
            in_whence = ZF_SEEK_SET;
            break;
        case AF_SEEK_CUR:
            in_whence = ZF_SEEK_CUR;
            break;
        case AF_SEEK_END:
            in_whence = ZF_SEEK_END;
            break;
        default:
            return -1;
    }
    return zf_seek(GET_FD(in_fd), in_offset, in_whence);
}

static ZF_INT32
_sys_close(AF_FILE in_fd)
{
    if (in_fd==NULL)
        return -1;
    zf_free(GET_FILE(in_fd)->fname);
    zf_close(GET_FD(in_fd));
    zf_free(in_fd);
    return 0;
}


static ZF_INT32
_sys_size(AF_FILE in_fd)
{
    ZF_Stat     st;
    if (zf_fstat(GET_FD(in_fd), &st)<0)
        return -1;
    return st.st_size;
}

static AF_File _AF_ZFileAPI = {
    &_sys_read,
    &_sys_write,
    &_sys_seek,
    &_sys_close,
    &_sys_dup,
    &_sys_size
};

AF_FILE
af_openSys(const ZF_CHAR8 *in_fname) {
    AF_ZFile    *new_one;
    ZF_FILE     fd;

    fd = zf_open(in_fname, ZF_RDONLY);
    if (fd<0) return NULL;
    new_one = (AF_ZFile*)zf_malloc(sizeof(AF_ZFile));
    new_one->fd = fd;
    new_one->fname = strdup(in_fname);
    memcpy(&new_one->base, &_AF_ZFileAPI, sizeof(_AF_ZFileAPI));
    return (AF_FILE)new_one;
}

ZF_INT32
af_seek(AF_FILE in_fd, ZF_INT32 in_offset, ZF_INT32 in_whence)
{
    return in_fd->seek(in_fd, in_offset, in_whence);
}

ZF_INT32
af_read(AF_FILE in_fd, ZF_UCHAR8 *out_buf, ZF_UINT32 in_size)
{
    return in_fd->read(in_fd, out_buf, in_size);
}

ZF_INT32
af_write(AF_FILE in_fd, const ZF_UCHAR8 *in_buf, ZF_UINT32 in_length)
{
    return in_fd->write(in_fd, in_buf, in_length);
}

ZF_INT32
af_close(AF_FILE in_fd)
{
    return in_fd->close(in_fd);
}

AF_FILE
af_dup(AF_FILE in_fd)
{
    return in_fd->dup(in_fd);
}

ZF_INT32
af_size(AF_FILE in_fd)
{
    return in_fd->size(in_fd);
}
