#include "zipfile.h"
#include "zf_port.h"
#include "af_file.h"
#include <string.h>
#include <stdio.h>
#include "zlib.h"

/*
 * Header signatures
 */
#define SIGSIZ 4
#define LOCSIG "PK\003\004"
#define CENSIG "PK\001\002"
#define ENDSIG "PK\005\006"

/*
 * Header sizes including signatures
 */
#define LOCHDRSIZ 30
#define CENHDRSIZ 46
#define ENDHDRSIZ 22

/*
 * Header field access macros
 */
#define CH(b, n)(((unsigned char *)(b))[n])
#define SH(b, n)(CH(b, n) |(CH(b, n+1) << 8))
#undef LG
#define LG(b, n)(SH(b, n) |(SH(b, n+2) << 16))

/*
 * Macros for getting local file header(LOC) fields
 */
#define LOCFLG(b) SH(b, 6)	    /* encrypt flags */
#define LOCHOW(b) SH(b, 8)	    /* compression method */
#define LOCCRC(b) LG(b, 14)	    /* uncompressed file crc-32 value */
#define LOCSIZ(b) LG(b, 18)	    /* compressed size */
#define LOCLEN(b) LG(b, 22)	    /* uncompressed size */
#define LOCNAM(b) SH(b, 26)	    /* filename size */
#define LOCEXT(b) SH(b, 28)	    /* extra field size */

/*
 * Macros for getting central directory header(CEN) fields
 */
#define CENHOW(b) SH(b, 10);	    /* compression method */
#define CENTIM(b) LG(b, 12)	    /* file modification time(DOS format) */
#define CENSIZ(b) LG(b, 20)	    /* compressed size */
#define CENLEN(b) LG(b, 24)	    /* uncompressed size */
#define CENNAM(b) SH(b, 28)	    /* length of filename */
#define CENEXT(b) SH(b, 30)	    /* length of extra field */
#define CENCOM(b) SH(b, 32)	    /* file comment length */
#define CENOFF(b) LG(b, 42)	    /* offset of local header */

/*
 * Macros for getting end of central directory header(END) fields
 */
#define ENDSUB(b) SH(b, 8)	    /* number of entries on this disk */
#define ENDTOT(b) SH(b, 10)	    /* total number of entries */
#define ENDSIZ(b) LG(b, 12)	    /* central directory size */
#define ENDOFF(b) LG(b, 16)	    /* central directory offset */
#define ENDCOM(b) SH(b, 20)	    /* size of zip file comment */

#define MAX(x,y)        ((x)<(y)?(y):(x))
#define MIN(x,y)        ((x)<(y)?(x):(y))

#define INBUFSIZE       1020

typedef struct ZF_ZipReader {
    AF_FILE     fd;
    int         cen_offset;
    int         end_offset;
    int         cen_number;
    int         loc_offset;
} ZF_ZipReader;



static ZF_BOOL
_readZipEnd(ZF_ZipReader *io_rd, ZF_UCHAR8 *out_end_buf)
{
    ZF_UCHAR8   buf[INBUFSIZE+SIGSIZ];
    ZF_INT32    start, size, offset, last, ret;
    ZF_UCHAR8   *ptr;

    last = af_seek(io_rd->fd, 0, ZF_SEEK_END);
    start = MAX(last-0xFFFF, 0);

    size = 0;
    offset = last;
    memset(buf, 0, SIGSIZ);

    while (offset>start) {
        size = MIN(INBUFSIZE, offset-start);

        memcpy(buf+size, buf, SIGSIZ);

        offset -= size;
        af_seek(io_rd->fd, offset, ZF_SEEK_SET);
        if ((ret=af_read(io_rd->fd, buf, size))<size) {
            zf_error((__FILE__ ":Illegal size to read (ret=%d)\n", ret));
            return ZF_FALSE;
        }

        for (ptr = buf+size-1 ; ptr>=buf ; ptr--) {
            if (strncmp(ptr, ENDSIG, SIGSIZ)==0) {
                offset = ptr-buf+offset;

                af_seek(io_rd->fd, offset, ZF_SEEK_SET);
                if (af_read(io_rd->fd, out_end_buf, ENDHDRSIZ)<ENDHDRSIZ) {
                    zf_error((__FILE__ ": Illegal end size!!\n"));
                    return ZF_FALSE;
                }

                if (ENDCOM(out_end_buf)+offset+ENDHDRSIZ!=last) {
                    zf_error((__FILE__ ": Illegal end header!!\n"));
                    return ZF_FALSE;
                }
                io_rd->end_offset = offset;
                return ZF_TRUE;
            }
        }
    }
    zf_error((__FILE__":I can not find end of zip!\n"));
    return ZF_FALSE;
}

static ZF_BOOL
_findCentral(ZF_ZipReader *io_rd)
{
    ZF_UCHAR8   end_header[ENDHDRSIZ];
    ZF_INT32    cen_length;

    if(!_readZipEnd(io_rd, end_header)) return ZF_FALSE;

    io_rd->cen_offset = io_rd->end_offset-ENDSIZ(end_header);
    if (ENDOFF(end_header)>io_rd->cen_offset) {
        zf_error((__FILE__":Invalid end-of-central directory header\n"));
        return ZF_FALSE;
    }

    io_rd->loc_offset = io_rd->cen_offset-ENDOFF(end_header);
    cen_length = ENDSIZ(end_header);

    io_rd->cen_number = ENDTOT(end_header);
    if (io_rd->cen_number*CENHDRSIZ > cen_length) {
        zf_error((__FILE__":Invalid central header count![%d]\n",
                    io_rd->cen_number));
        return ZF_FALSE;
    }

    if (ENDSUB(end_header)!=io_rd->cen_number) {
        zf_error((__FILE__":Illegal drive count[%d]\n",
                    ENDSUB(end_header)));
        return ZF_FALSE;
    }
    return ZF_TRUE;
}

ZF_BOOL
zip_parseFile(AF_FILE in_fd, ZF_UCHAR8 *in_client, ZF_Handler in_handler)
{
    ZF_ZipReader    ctx;
    ZF_INT32        idx;
    ZF_UCHAR8       buf[CENHDRSIZ];
    ZF_Entry        ent;
    ZF_INT32        name_len;
    ZF_BOOL         ret = ZF_TRUE;
    ZF_CHAR8        namebuf[256];

    ctx.fd = in_fd;
    if (!_findCentral(&ctx)) {
        zf_error((__FILE__":fail to find central\n"));
        return ZF_FALSE;
    }
    if (af_seek(ctx.fd, ctx.cen_offset, ZF_SEEK_SET)<0) {
        zf_error((__FILE__":fail to seek\n"));
        return ZF_FALSE;
    }

    for (idx=0 ; idx<ctx.cen_number ; idx++) {
        if (af_read(ctx.fd, buf, CENHDRSIZ)<CENHDRSIZ) {
            zf_error((__FILE__":fail to read header\n"));
            return ZF_FALSE;
        }
        if (strncmp(buf, CENSIG, SIGSIZ)!=0) {
            zf_error((__FILE__":illegal zip header signature\n"));
            return ZF_FALSE;
        }
        name_len = CENNAM(buf);
        if (name_len<1) {
            zf_error((__FILE__":fail to get name\n"));
            return ZF_FALSE;
        }
        if (name_len<sizeof(namebuf)-1) {
            ent.name = namebuf;
        } else {
            ent.name = zf_malloc(name_len+1);
        }
        if (af_read(ctx.fd, ent.name, name_len)<name_len) {
            zf_error((__FILE__":fail to read name\n"));
            goto format_error;
        }
        ent.name[name_len] = 0;

        if (name_len>0 && ent.name[name_len-1]=='/') {
            ent.type = ZF_ET_DIR;
            ent.csize = 0;
            ent.size = 0;
            ent.offset = 0;
            ent.st_m_time = CENTIM(buf);
            ent.how = 0;

            ret = in_handler(in_client, &ent);

            if (ent.name!=namebuf) zf_free(ent.name);
            ent.name = NULL;

            if (!ret)  return ZF_FALSE;

            if (af_seek(ctx.fd, CENEXT(buf)+CENCOM(buf), ZF_SEEK_CUR)<0) {
                zf_error((__FILE__":fail to jump to next\n"));
                return ZF_FALSE;
            }
            continue;
        }

        ent.type = ZF_ET_FILE;
        ent.csize = CENSIZ(buf);
        ent.size = CENLEN(buf);
        ent.how = CENHOW(buf);
        ent.offset = CENOFF(buf)+ctx.loc_offset;
        ent.st_m_time = CENTIM(buf);

        in_handler(in_client, &ent);

        if (ent.name!=namebuf) zf_free(ent.name);
        ent.name = NULL;

        if (!ret)  return ZF_FALSE;

        if (af_seek(ctx.fd, CENEXT(buf)+CENCOM(buf), ZF_SEEK_CUR)<0) {
            zf_error((__FILE__":fail to seek on next\n"));
            goto format_error;
        }
    }
    return ZF_TRUE;
format_error:
    if (NULL!=ent.name && ent.name!=namebuf) {
        zf_free(ent.name);
    }
    return ZF_FALSE;
}

typedef struct ZF_ZStream {
    AF_File     base;
    AF_FILE     fd;
    ZF_INT32    start, csize, size, offset, s_offset;
    z_stream    zst;
    ZF_UCHAR8   in_buf[2048];
} ZF_ZStream;

#define ZS(fd)      ((ZF_ZStream*)(fd))
#define ZST(fd)     (ZS(fd)->zst)

static void *
my_zalloc(void *data, unsigned int items, unsigned int size)
{
  return zf_calloc(size, items);
}

static void
my_zfree(void *data, void *addr)
{
  zf_free(addr);
}

static ZF_INT32
_z_seek(AF_FILE in_fd, ZF_INT32 in_offset, ZF_INT32 in_whence)
{
    ZF_ZStream  *fd = ZS(in_fd);
    ZF_INT32    new_offset, to_read, result;

    switch (in_whence) {
        case ZF_SEEK_SET:
            new_offset = in_offset;
            break;
        case ZF_SEEK_CUR:
            new_offset = fd->offset+in_offset;
            break;
        case ZF_SEEK_END:
            new_offset = fd->size+in_offset;
            break;
        default:
            return -1;
    }
    /* over ranged position */
    if (new_offset>fd->size || new_offset<0)
        return -1;
    /* end of file */
    if (new_offset==fd->size) {
        fd->offset = new_offset;
        return fd->offset;
    }

    /* no change needed */
    if (new_offset==fd->offset)
        return new_offset;

    /* reset to first position if target position is less than current */
    if (new_offset<fd->offset) {
        fd->offset = 0;
        fd->s_offset = 0;

        inflateReset(&fd->zst);

        fd->zst.avail_in = 0;
        fd->zst.next_in = fd->in_buf;
        af_seek(fd->fd, fd->start, ZF_SEEK_SET);
    }

    /* seek to specified position */
    if (new_offset>fd->offset) {
        ZF_UCHAR8   out_buf[2048];
        ZF_INT32    in_size;

        while (new_offset>fd->offset) {
            /* skipping size (limited to current buffer) */
            to_read = new_offset-fd->offset;
            if (to_read>sizeof(out_buf))
                to_read = sizeof(out_buf);

            /* temporary output buffer */
            fd->zst.next_out = out_buf;
            fd->zst.avail_out = to_read;

            /* skipping */
            while (fd->zst.avail_out>0) {
                if (fd->zst.avail_in==0) {
                    in_size = sizeof(fd->in_buf);
                    if (in_size+fd->s_offset > fd->csize)
                        in_size = fd->csize-fd->s_offset;

                    if (in_size>0) {
                        /* read bytes from file */
                        result = af_read(fd->fd,
                                fd->in_buf, in_size);
                        if (result<in_size)
                            return -1;

                        fd->s_offset += in_size;
                        fd->zst.next_in = fd->in_buf;
                        fd->zst.avail_in = in_size;
                    }
                }

                result = inflate(&fd->zst, Z_SYNC_FLUSH);
                if (result==Z_DATA_ERROR)
                    return -1;
                if (result!=Z_OK)
                    break;
            }
            /* position change */
            fd->offset += to_read-fd->zst.avail_out;
        }
    }
    return fd->offset;
}

static ZF_INT32
_z_read(AF_FILE in_fd, ZF_UCHAR8 *out_buf, ZF_UINT32 in_size)
{
    ZF_ZStream  *fd = ZS(in_fd);
    int         to_read, readed, result;

    readed = 0;
    fd->zst.next_out = out_buf;
    fd->zst.avail_out = in_size;
    while (fd->zst.avail_out>0) {
        /* check input buffer */
        if (fd->zst.avail_in==0) {
            to_read = sizeof(fd->in_buf);
            if (to_read>(fd->csize-fd->s_offset))
                to_read = fd->csize - fd->s_offset;

            if (to_read>0) {
                fd->zst.next_in = fd->in_buf;
                result = af_read(fd->fd, fd->in_buf, to_read);
                if (result<0)
                    return -1;

                fd->s_offset += result;
                fd->zst.avail_in = result;
            }
        }

        /* inflate */
        result = inflate(&fd->zst, Z_SYNC_FLUSH);
        if (result==Z_DATA_ERROR)
            return -1;
        if (result!=Z_OK)
            break;
    }
    /* decompressed block size */
    readed = in_size-fd->zst.avail_out;
    fd->offset += readed;
    return readed;
}

static ZF_INT32
_z_write(AF_FILE in_fd, const ZF_UCHAR8 *in_buf, ZF_UINT32 in_length)
{
    return -1;
}

static ZF_INT32
_z_close(AF_FILE in_fd)
{
    af_close(ZS(in_fd)->fd);
    inflateEnd(&ZST(in_fd));
    zf_free(in_fd);
}

static AF_FILE
_z_dup(AF_FILE in_fd)
{
    ZF_ZStream  *file = ZS(in_fd);
    return zlib_open(file->fd, file->start, file->csize, file->size, -15);
}

static ZF_INT32
_z_size(AF_FILE in_fd)
{
    return ZS(in_fd)->size;
}

AF_File _zlibFileBase = {
    &_z_read,
    &_z_write,
    &_z_seek,
    &_z_close,
    &_z_dup,
    &_z_size
};

#ifndef BSZ
#define BSZ     16384
#endif


AF_FILE
zlib_open(AF_FILE in_fd, ZF_UINT32 in_offset, ZF_UINT32 in_csize,
        ZF_UINT32 in_size, ZF_INT32 in_wbits)
{
    ZF_INT32    ret;
    AF_FILE     new_fd;
    ZF_ZStream  *new_file=NULL;

    new_file = (ZF_ZStream*)zf_malloc(sizeof(ZF_ZStream));
    if (NULL==new_file) return NULL;

    memcpy(new_file, &_zlibFileBase, sizeof(AF_File));

    new_fd = af_dup(in_fd);
    if (new_fd==NULL) goto fail_after_malloc;
    new_file->fd = new_fd;

    new_file->start = in_offset;
    ret = af_seek(new_fd, new_file->start, AF_SEEK_SET);
    if (ret<0) goto fail_after_malloc;

    new_file->csize = in_csize;
    new_file->size = in_size;
    new_file->offset = 0;
    new_file->s_offset = 0;

    memset(&ZST(new_file), 0, sizeof(z_stream));
    inflateInit2(&ZST(new_file), in_wbits);

    return (AF_FILE)new_file;

fail_after_malloc:
    zf_free(new_file);
    return NULL;
}

AF_FILE
zip_open(AF_FILE in_fd, ZF_UINT32 in_offset, ZF_UINT32 in_csize,
        ZF_UINT32 in_size)
{
    ZF_INT32    ret;
    ZF_UCHAR8   buf[LOCHDRSIZ];

    /* no compression */
    if (in_csize==in_size) {
        return af_dup(in_fd);
    }

    /* get local header */
    ret = af_seek(in_fd, in_offset, AF_SEEK_SET);
    if (ret<0) return NULL;
    if (af_read(in_fd, buf, LOCHDRSIZ)!=LOCHDRSIZ)
        return NULL;

    return zlib_open(in_fd,
            in_offset+LOCHDRSIZ+LOCNAM(buf)+LOCEXT(buf),
            in_csize, in_size, -15);
}
