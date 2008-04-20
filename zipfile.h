#ifndef __ZIP_FILE_H__
#define __ZIP_FILE_H__

#include "zf_port.h"
#include "af_file.h"

typedef enum ZF_EType {
    ZF_ET_FILE      = 0,
    ZF_ET_DIR       = 1,
    ZF_ET_OTHER     = 2
} ZF_EType;

typedef struct ZF_Entry {
    ZF_EType    type;
    ZF_CHAR8    *name;
    ZF_UINT32   size;
    ZF_UINT32   offset, csize;
    ZF_INT32    st_m_time;
    ZF_UINT16   how;
} ZF_Entry;

typedef ZF_BOOL (*ZF_Handler)(ZF_UCHAR8* in_client, ZF_Entry *in_entry);

#ifdef  __cplusplus
extern "C" {
#endif

    ZF_BOOL zip_parseFile(AF_FILE fd, ZF_UCHAR8 *in_client, ZF_Handler handler);
    AF_FILE zip_open(AF_FILE in_fd, ZF_UINT32 in_offset, ZF_UINT32 in_csize,
        ZF_UINT32 in_size);
    AF_FILE zlib_open(AF_FILE in_fd, ZF_UINT32 in_offset, ZF_UINT32 in_csize,
        ZF_UINT32 in_size, ZF_INT32 in_wbits);

#ifdef  __cplusplus
};
#endif

#endif
