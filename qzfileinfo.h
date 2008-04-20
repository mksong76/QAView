#ifndef __QZ_FILEINFO_H__
#define __QZ_FILEINFO_H__

class ZEntry;
class QFileInfo;

class   QZFileInfo {
  private:
    union {
      ZEntry    *z;
      QFileInfo *q;
    }       m_dir;
    int     m_type;
  public:

    QZFileInfo(QString str);
    QZFileInfo(ZEntry *entry);
    ~QZFileInfo();

    int getType();
    unsigned int getSize();
    int isFile();
    int isDir();
    QString getName();
    QString getFullName();
};

#endif
