#ifndef __FILE_LISTER_H__
#define __FILE_LISTER_H__

#include <qstring.h>
#include "qzdir.h"
#include "filetype.h"

class FileLister {
    public :
        FileLister();

        int setFile(const QString &path);
        QString getFile(int idx);
        int count();
        int find(const QString &fname);
        int findPath(const QString &path);

        static FILE_TYPE getType(const QString &filename, FILE_ENCODING &enc);

    private:
        QZDir       m_dir;
        QStringList m_list;
        int         m_type;
};

#endif
