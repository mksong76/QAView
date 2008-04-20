#include "qzfs.h"
#include <qfile.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "qunicode.h"
#include "zipfs.h"

/**
 * Get real name for file.
 */
int
getNames(const QString &full, QCString &zname, QCString &sname, ZEntry **buf)
{
    QString zip_name, suname;
    int prev_type = QZF_NONE;

    if (QFile::exists(full)) {
        QFileInfo       qfi(full);
        if (qfi.isDir())
          return QZF_DIR;
        if (qfi.isFile()) {
          prev_type = QZF_FILE;
        } else {
          return QZF_NONE;
        }
    }
    //printf("File[%s] does not exist!\n", (const char*)full);

    /* finding zip file name */
    int idx;
    idx = full.length();
    zip_name = full;
    while(idx>=0) {
        if (QFile::exists(zip_name))
            break;

        idx = full.findRev('/', idx);
        if (idx<0)
            return prev_type;
        zip_name = full.left(idx);
        idx--;
    }
    if (toMB(sname, full.mid(idx+1), "euc-kr")<0)
      sname = full.mid(idx+1);
    QFileInfo   zdir(zip_name);
    zip_name = zdir.absFilePath();

    ZRootEntry *root = ZDirEntry::getEntry(zip_name.utf8());
    if (root==NULL) {
      if (zdir.extension(false)=="dz") {
        zname = zip_name.utf8();
        sname = NULL;
        return QZF_DZ|QZF_FILE;
      }
      return prev_type;
    }

    ZEntry *entry = root->findByName(sname);
    if (entry==NULL) {
      return prev_type;
    }

    zname = zip_name.utf8();

    if (buf) *buf = entry;
    if (entry->isDir()) {
      return QZF_ZIP|QZF_DIR;
    } else {
      return QZF_ZIP|QZF_FILE;
    }
}

