#ifndef __ZIP_FILE_SYSTEM_INTERNAL_H__
#define __ZIP_FILE_SYSTEM_INTERNAL_H__

#include "zipfs.h"

typedef struct ZipFD {
    ZRootEntry  *root;
    const char  *zip_name;
    int         ref_count;
    struct ZipFD    *next;
} ZipFD;

#ifdef  __cplusplus
extern "C" {
#endif

    ZipFD *zfd_find(const char *name);
    ZipFD *zfd_findByRoot(ZRootEntry *root);
    void zfd_add(ZipFD *fd);
    void zfd_remove(ZipFD *fd);
    void zfd_gc();

#ifdef  __cplusplus
};
#endif

#endif
