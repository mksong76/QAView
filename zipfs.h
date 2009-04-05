#ifndef __ZIP_FILE_SYSTEM_H__
#define __ZIP_FILE_SYSTEM_H__

#include <stdio.h>
#include "af_file.h"

class   ZDirEntry;
class   ZRootEntry;

class   ZEntry
{
    protected :
        char        *m_name;
        bool        m_isDir;
        ZDirEntry   *m_parent;

        int     getFullNameI(char *name_buffer, int max);
    public:
        ZEntry      *m_next;

        ZEntry(ZDirEntry *m_parent, const char *name, int len, bool is_dir);
        ~ZEntry();

        bool    isSameName(const char *name, int len=-1);
        bool    isDir() { return m_isDir; }
        int     getFullName(char *name_buffer=NULL, int max=2048);
        const char *name();
        virtual ZEntry *findByName(const char *name, int len=-1) = 0;
        ZDirEntry *getParent();
        ZRootEntry *getRoot();
        const char *zipFileName();

        virtual void printAll(int level) = 0;

        /**
         * Increase reference count.
         * @return Resulting reference count.
         */
        int grab();
        /**
         * @return Resulting reference count.
         */
        int release();
};

class   ZFileEntry;

class   ZDirEntry : public ZEntry
{
    private :
        ZEntry      *m_fileList, *m_last;
        int         m_entryCount;

    public:
        ZDirEntry(ZDirEntry *parent, const char *name, int len);
        ~ZDirEntry();

        ZEntry *findByName(const char *name, int len=-1);
        ZFileEntry *addFileName(const char *name, int len=-1);
        ZEntry *getFirst();
        virtual void printAll(int level);

        static ZRootEntry *getEntry(const char *f_name);
        static void GC();
};

class   ZFileEntry : public ZEntry
{
    public:
        int     m_pos, m_size, m_csize;

    public:
        ZFileEntry(ZDirEntry *parent, const char *name, int len);
        ZEntry *findByName(const char *name, int len=-1) { return NULL; }
        virtual void printAll(int level);
};

class   ZRootEntry : public ZDirEntry
{
    public:
        ZRootEntry();
        ~ZRootEntry();

    public:
        AF_FILE m_fd;
};

#endif
