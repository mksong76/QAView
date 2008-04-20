#ifndef __FILE_LISTER_H__
#define __FILE_LISTER_H__

#include <qstring.h>
#include "qzdir.h"

enum {
    FLT_UNKNOWN,
    FLT_IMAGE,
    FLT_TEXT
};

class FileLister {
    public :
        FileLister();

        int setFile(const QString &path);
        QString getFile(int idx);
        int count();
        int find(const QString &fname);
        int findPath(const QString &path);

        static int getType(const QString &filename);

    private:
        QZDir       m_dir;
        QStringList m_list;
        int         m_type;
};

#endif
