/**
 * Directory list emulation for specified.
 * vim:sw=2
 */
#ifndef __QZ_DIR_H__
#define __QZ_DIR_H__

#include <qstring.h>
#include <qstringlist.h>

class   ZDirEntry;
class   ZFileEntry;
class   QDir;

/* QZDir filter simbole.
   all symboles must be ored.  */
enum {
  QZFL_NONE=0, QZFL_FILE=1, QZFL_DIR=2, QZFL_DOTS=4,
  QZFL_ALL = QZFL_DIR|QZFL_FILE|QZFL_DOTS
};

enum {
  QZFS_NAME, 
  QZFS_EXT,
  QZFS_SIZE,
  QZFS_DIR,
  QZFS_REVERSE = 0x10
};

class   QZDir {
  private:
    QString     m_path;

    int         m_type;

    union {
      ZDirEntry         *z;
      QDir              *q;
    } m_dir;

  protected:
    virtual void closeZ();
    virtual int updateZ();

  public:

    QZDir();
    QZDir(QString path);
    void setPath(QString path);
    QStringList getList(QString mask, int filter, int sort_policy);

    bool cd(QString str);
    bool cdUp();
    QString path();
    QString absPath();

    /* get information about directory entry */
    int getType(QString sub);
    bool isFile(QString sub);
    bool isDir(QString sub);
    bool isFile();
    bool isDir();

    bool exist();

    static QString getDir(const QString &path, QString *filename = NULL);
    static QString getBasename(const QString &path);
};

#endif  /* __QZ_DIR_H__ */
