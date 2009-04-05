#include "zipfs.h"
#include <stdlib.h>
#include <string.h>
#include "zipfile.h"
#include "zipfsi.h"
#include "af_file.h"

//-----------------------------------------------------------------------------
// Zip Utiltity classes.

ZEntry::ZEntry(ZDirEntry *parent, const char *name, int len, bool is_dir)
{
    m_parent = parent;
    m_name = (char*)malloc(len+1);
    memcpy(m_name, name, len);
    m_name[len] = 0;
    m_isDir = is_dir;

    m_next = NULL;
}

ZEntry::~ZEntry()
{
    if (m_next) delete m_next;
    if (m_name) delete m_name;
    //printf("DELETE ZENT:%p\n", this);
}

bool
ZEntry::isSameName(const char *name, int len)
{
    if (strncmp(name, m_name, len)==0) {
        if (m_name[len]==0)
            return true;
    }
    return false;
}

const char *
ZEntry::name()
{
    return m_name;
}

int
ZEntry::getFullName(char *name_buffer, int max)
{
    int     used = getFullNameI(name_buffer, max);

    if (used==0) {
        if (name_buffer) name_buffer[0] = '/';
        used++;
    }

    if (used==max) {
        if (name_buffer) {
            used--;
            name_buffer[used] = 0;
        }
    } else {
        name_buffer[used] = 0;
    }
    return used;
}

#ifndef MIN
#define MIN(x,y)    ((x)<(y)?(x):(y))
#endif

int
ZEntry::getFullNameI(char *name_buffer, int max)
{
    int     used=0, my_len;

    if (m_parent) {
        used += m_parent->getFullNameI(name_buffer, max);
        if (used<max && name_buffer) {
            name_buffer[used] = '/';
        }
        used += 1;
    }

    if (used<max) {
        my_len = MIN(strlen(m_name), max-used);
        if (name_buffer)
            memcpy(name_buffer+used, m_name, my_len);
        used += my_len;
    }
    return used;
}

ZDirEntry *
ZEntry::getParent()
{
    return m_parent;
}

ZRootEntry *
ZEntry::getRoot()
{
    ZEntry      *ptr=this;
    while (ptr && ptr->m_parent)
      ptr = ptr->m_parent;

    return (ZRootEntry*)ptr;
}

const char *
ZEntry::zipFileName()
{
    ZRootEntry *root = getRoot();
    ZipFD *fd = zfd_findByRoot(root);
    if (!fd) return NULL;

    return fd->zip_name;
}

ZDirEntry::ZDirEntry(ZDirEntry *parent, const char *name, int len )
    :ZEntry(parent, name, len, true)
{
    m_fileList = NULL;
    m_last = NULL;
    m_entryCount = 0;
}

ZDirEntry::~ZDirEntry()
{
    if( m_fileList )
        delete m_fileList;
    //printf("DELETE ZDIR:%p\n", this);
}

ZEntry *
ZDirEntry::findByName(const char *name, int len)
{
    int idx;
    if (len<0) len = strlen(name);

    ZEntry      *ptr;
    ZDirEntry   *cdir = this;

    if (name[0]=='/') {
        while (cdir->m_parent) cdir = cdir->m_parent;
        name++;
        len--;
    }

    ptr = cdir;
    while (len>0 && ptr) {
        cdir = (ZDirEntry*)ptr;

        for (idx=0 ; idx<len && name[idx] && name[idx]!='/' ; idx++);
        bool is_dir = idx<len && name[idx]=='/';

        ptr = cdir->m_fileList;
        while (ptr) {
            if (idx==1 && name[0]=='.') {
                ptr = cdir;
                len -= 1+is_dir;
                name += 1+is_dir;
                break;
            }
            if (idx==2 && name[0]=='.' && name[1]=='.') {
                ptr = cdir->m_parent;
                len -= idx+is_dir;
                name += idx+is_dir;
                break;
            }

            //printf("SEARCH:[%s] [:%d]\n", name, idx);
            if (ptr->isSameName(name, idx)) {
                //printf("FOUND:%s[:%d]\n", name, idx);
                if (is_dir) {
                    if (ptr->isDir()) {
                        name = name+idx+1;
                        len -= idx+1;
                        break;
                    } else {
                        return NULL;
                    }
                } else {
                    return ptr;
                }
            }
            ptr = ptr->m_next;
        }
    }
    return ptr;
}

ZFileEntry *
ZDirEntry::addFileName(const char *name, int len)
{
    int     idx;
    bool    is_dir;

    ZDirEntry *p_entry = this;
    ZEntry    *entry;

    while (1) {
        if (name[0]=='/') name += 1;
        if (len<0) len = strlen(name);

        /* find last file_name */
        for (idx=0 ; name[idx] && idx<len && name[idx]!='/'; idx++);
        is_dir = idx<len && name[idx]=='/';

        /* find directory entry */
        ZEntry *entry = p_entry->findByName(name, idx);
        if (entry==NULL) {
            if (is_dir) {
                entry = new ZDirEntry((ZDirEntry*)p_entry, name, idx);
            } else {
                entry = new ZFileEntry((ZDirEntry*)p_entry, name, idx);
            }
            entry->m_next = NULL;
            if (p_entry->m_last) {
                p_entry->m_last->m_next = entry;
                p_entry->m_last = entry;
            } else {
                p_entry->m_last = p_entry->m_fileList = entry;
            }

            if (is_dir) {
                p_entry = (ZDirEntry*)entry;
                name += idx;
                len -= idx;
                continue;
            } else {
                return (ZFileEntry*)entry;
            }
        } else {
            /* same name with different type.. */
            if (entry->isDir()!=is_dir)
                return NULL;

            if (is_dir) {
                p_entry = (ZDirEntry*)entry;
                name += idx;
                len -= idx;
                continue;
            } else {
                /* file aleady exists!! */
                return NULL;
            }
        }
    }
}

void
ZDirEntry::printAll(int level)
{
    printf("[%02d] DIR:%s\n", level, m_name);
    printf("[>>]\n");
    ZEntry  *ptr = m_fileList;
    while (ptr) {
        ptr->printAll(level+1);
        ptr = ptr->m_next;
    }
    printf("[<<]\n");
}

void
ZFileEntry::printAll(int level)
{
    printf("[%02d] FILE:%s\n", level, m_name);
}

ZFileEntry::ZFileEntry(ZDirEntry *parent, const char *name, int len)
    :ZEntry(parent, name, len, false)
{
    m_pos = 0;
    m_size = 0;
    m_csize = 0;
}

ZF_BOOL
_handleEntries(ZF_UCHAR8* in_client, ZF_Entry *in_entry)
{
    ZDirEntry *root = (ZDirEntry*)in_client;
    if (in_entry->type!=ZF_ET_FILE) return ZF_TRUE;
    ZFileEntry *new_entry = root->addFileName((const char*)in_entry->name);
    if (new_entry) {
        new_entry->m_csize = in_entry->csize;
        new_entry->m_size = in_entry->size;
        new_entry->m_pos = in_entry->offset;
        return ZF_TRUE;
    } else {
        printf ("_handleEntries:fail to addFileName(%s)\n", in_entry->name);
        return ZF_FALSE;
    }
}

ZRootEntry *
getEntryI(AF_FILE fd)
{
    if (af_seek(fd, 0, ZF_SEEK_SET)!=0) {
#ifdef      ZF_DEBUG
        printf ("getEntryI:fail to seek to 0\n");
#endif
        return NULL;
    }
    ZRootEntry *m_root = new ZRootEntry();
    if (!zip_parseFile(fd, (ZF_UCHAR8*)m_root, _handleEntries)) {
        delete m_root;
#ifdef      ZF_DEBUG
        printf ("getEntryI:fail to parse fd=%08x\n", fd);
#endif
        return NULL;
    }
#ifdef      ZF_DEBUG
    m_root->printAll(0);
#endif
    m_root->m_fd = fd;
    return m_root;
}

ZEntry *
ZDirEntry::getFirst()
{
  return m_fileList;
}

//-----------------------------------------------------------------------------
// Zip File System main classs.
// constructors.
// open zip file.
// it will check zip file( reading header information and set various
// settings.
ZRootEntry *
ZDirEntry::getEntry(const char *m_fileName)
{
    ZipFD *zfd;

    zfd = zfd_find(m_fileName);
    if (zfd) return zfd->root;

    AF_FILE fd = af_openSys((ZF_CHAR8*)m_fileName);
    ZRootEntry *root = getEntryI(fd);
    if (!root) {
#ifdef      ZF_DEBUG
        printf ("ZDirEntry::getEntryI():fail to getEntryI\n");
#endif
        af_close(fd);
        return NULL;
    }

    ZipFD   *new_fd = new ZipFD;
    new_fd->root = root;
    new_fd->zip_name = strdup(m_fileName);
    new_fd->ref_count = 0;
    new_fd->next = NULL;
    zfd_add(new_fd);

    return root;
}

int
ZEntry::grab()
{
    ZipFD *fd = zfd_findByRoot(getRoot());
    if (fd==NULL) return 0;
    fd->ref_count++;
    return fd->ref_count;
}

int
ZEntry::release()
{
    ZipFD *fd = zfd_findByRoot(getRoot());
    if (fd==NULL) return 0;
    fd->ref_count--;
    if (fd->ref_count==0) {
        zfd_gc();
        return 0;
    }
    return fd->ref_count;
}

void
ZDirEntry::GC()
{
    zfd_gc();
}

ZRootEntry::ZRootEntry() : ZDirEntry(NULL, "", 0)
{
    m_fd = NULL;
}

ZRootEntry::~ZRootEntry()
{
    if (NULL!=m_fd) {
        af_close(m_fd);
    }
    m_fd = NULL;
}
