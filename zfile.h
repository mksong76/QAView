#ifndef __ZFILE_H__
#define __ZFILE_H__

#include "afile.h"
#include "af_file.h"
#include "zipfs.h"

class   ZFile : public AFile
{
    protected :
        AF_FILE     m_fd;
        ZFileEntry  *m_fe;

    private :
        ZFile();

    public:
        ~ZFile();
        static ZFile *open(const char *file_name, const char *sub_name);
        static bool isDir(const char *file_name, const char *sub_name);
        static bool isFile(const char *file_name, const char *sub_name);

        virtual int read(char *buffer, int length);
        virtual int seek(int offset, int whence);
        virtual int tell();
        virtual void close() { };
        virtual int size();
};

#ifdef  __cplusplus
extern "C" {
#endif
    void    ZFileSystemGC();
#ifdef  __cplusplus
};
#endif

#endif
