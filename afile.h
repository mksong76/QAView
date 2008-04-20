#ifndef __A_FILE_H__
#define __A_FILE_H__


class   AFile
{
  public:
    virtual int read(char *buffer, int length) = 0;
    virtual int seek(int offset, int whence) = 0;
    virtual int tell() = 0;
    virtual void close() = 0;
    virtual int size() = 0;
};


#endif
