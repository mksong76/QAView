#include "zfile.h"
#include "zipfs.h"
#include "zlib.h"
#include "zutil.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include "zipfile.h"


ZFile::ZFile()
{
    m_fd = NULL;
    m_fe = NULL;
}

ZFile::~ZFile()
{
    af_close(m_fd);
    m_fe->getRoot()->release();
}

ZFile *
ZFile::open(const char *file_name, const char *sub_name)
{
    ZRootEntry *root = ZDirEntry::getEntry(file_name);
    if( root==NULL ) return NULL;

    ZEntry *entry = (ZFileEntry*)(root->findByName(sub_name));
    if( entry==NULL ) return NULL;
    if( entry->isDir() ) return NULL;

    ZFileEntry *f_entry = (ZFileEntry*)entry;

    AF_FILE fd;
    fd = zip_open(root->m_fd,
            f_entry->m_pos, f_entry->m_csize, f_entry->m_size);
    if (fd==NULL) return NULL;

    ZFile   *zf = new ZFile();
    if (NULL==zf) {
        af_close(fd);
        return NULL;
    }
    zf->m_fe = f_entry;
    zf->m_fd = fd;
    root->grab();

    return zf;
}

bool
ZFile::isDir(const char *file_name, const char *sub_name)
{
    ZRootEntry *root = ZDirEntry::getEntry(file_name);
    if (!root) return false;
    ZEntry *entry = root->findByName(sub_name);
    if (entry)
        return entry->isDir();
    else
        return false;
}

bool
ZFile::isFile(const char *file_name, const char *sub_name)
{
    ZRootEntry *root = ZDirEntry::getEntry(file_name);
    if (!root) return false;
    ZEntry *entry = root->findByName(sub_name);
    if (entry)
        return !entry->isDir();
    else
        return false;
}


int
ZFile::read(char *buffer, int length)
{
    return af_read(m_fd, (ZF_UCHAR8*)buffer, length);
}

int
ZFile::seek(int offset, int whence)
{
    int new_whence;
    switch(whence) {
        case SEEK_SET:
            new_whence = ZF_SEEK_SET;
            break;
        case SEEK_CUR:
            new_whence = ZF_SEEK_CUR;
            break;
        case SEEK_END:
            new_whence = ZF_SEEK_END;
            break;
        default:
            return -1;
    }

    return af_seek(m_fd, offset, new_whence);
}

int
ZFile::tell()
{
    return af_seek(m_fd, 0, ZF_SEEK_CUR);
}

int
ZFile::size()
{
  return af_size(m_fd);
}

void
ZFileSystemGC()
{
    ZDirEntry::GC();
}
