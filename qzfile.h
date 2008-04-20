#ifndef __ZIP_FILE__
#define __ZIP_FILE__

#include <qstring.h>
#include <qcstring.h>
#include <stdio.h>

#include "afile.h"

class   QZFile : public AFile {
    private:
        union {
            FILE    *q;
            AFile   *z;
        } m_fd;

        QString     m_fullPath;
        int         m_type;
        QCString    m_zipPath, m_subPath;

        void updateName();
    public :
        QZFile();
        QZFile(const QString &name);
        ~QZFile();
        void setName(const QString &name);
        QString name();

        virtual bool exists();
        virtual bool open();
        virtual void close();
        virtual int read(char *buffer, int length);
        virtual int seek(int offset, int whence);
        virtual int tell();
        virtual int size();
};

#endif
