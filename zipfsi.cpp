#include "zipfsi.h"
#include "zipfs.h"
#include <string.h>

ZipFD       *zip_fd_list = NULL;

ZipFD *
zfd_find(const char *name)
{
    ZipFD   *ptr = zip_fd_list;
    while( ptr ) {
        if( strcmp(ptr->zip_name ,name)==0 ) {
            break;
        }
        ptr = ptr->next;
    }
    return ptr;
}

ZipFD *
zfd_findByRoot(ZRootEntry *root)
{
    ZipFD   *ptr = zip_fd_list;
    while( ptr ) {
        if( ptr->root==root ) {
            break;
        }
        ptr = ptr->next;
    }
    return ptr;
}

void
zfd_add(ZipFD *fd)
{
    fd->next = zip_fd_list;
    zip_fd_list = fd;
}

void
zfd_remove(ZipFD *fd)
{
    ZipFD   *prev = NULL, *ptr = zip_fd_list;
    while( ptr ) {
        if( ptr==fd ) {
            if( prev )
                prev->next = ptr->next;
            else
                zip_fd_list = ptr->next;
            return;
        }
        prev = ptr;
        ptr = ptr->next;
    }
    return;
}

void
zfd_gc()
{
    ZipFD   *ptr = zip_fd_list, *prev=NULL, *to_del;

    while( ptr ) {
        if( ptr->ref_count==0 ) {
            delete ptr->root;
            delete ptr->zip_name;

            to_del = ptr;
            ptr = ptr->next;
            if( prev ) {
                prev->next = ptr;
            } else {
                zip_fd_list = ptr;
            }

            delete to_del;
            continue;
        }
        prev = ptr;
        ptr = ptr->next;
    }
}
