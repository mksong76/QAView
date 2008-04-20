#ifndef __QZ_FS_H__
#define __QZ_FS_H__

#include <qstring.h>

// what is this file
enum { QZF_NONE=0, QZF_ZIP=1, QZF_DZ=2,
    QZF_FILE=0x10, QZF_DIR=0x20, QZF_UNCHK=0x80 };
#define QZF_TYPEMASK    0x0f

class ZEntry;
/*
 * Get zip file name and path
 * @param full full path for search
 * @param zname, sname for ziped file
 * @return QZF_ZFILE when the path is in the zip file
 *      QZF_QFILE when the path is normal file.
 *      QZF_NONE when the specified file does not exist.
 */
int getNames(const QString &full, QCString &zname, QCString &sname,
                                                      ZEntry **buf=NULL);

#endif
