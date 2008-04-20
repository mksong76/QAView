/**
 * Directory emulation code.
 * vim:sw=2
 */
#include "qzdir.h"
#include "qfileinfo.h"
#include "qzfs.h"
#include "zipfs.h"
#include "zipfsi.h"
#include "qunicode.h"
#include <qdir.h>
#include <qfileinfo.h>

QZDir::QZDir()
{
  m_path = "";
  m_type = QZF_NONE;
}


QZDir::QZDir(QString npath)
{
  m_path = npath;
  m_type = QZF_UNCHK;
}

void
QZDir::closeZ()
{
  if (m_type==QZF_NONE || m_type==QZF_UNCHK) {
    m_type = QZF_UNCHK;
    return;
  }
  if (m_type&QZF_ZIP) {
    m_dir.z->release();
    m_dir.z = NULL;
  } else {
    delete m_dir.q;
    m_dir.q = NULL;
  }
  m_type = QZF_UNCHK;
}

int
QZDir::updateZ()
{
  if (m_type!=QZF_UNCHK) return m_type;

  int       my_type;
  QCString  m_zname, m_sub;
  ZEntry    *my_entry;

  my_type = getNames(m_path, m_zname, m_sub, &my_entry);

  if (my_type&QZF_DIR) {
    /* this is in zip file */
    if (my_type&QZF_ZIP) {
      my_entry->grab();
      m_dir.z = (ZDirEntry*)my_entry;
    } else {
      m_dir.q = new QDir(m_path);
    }
    m_type = my_type;
  } else {
    m_type = QZF_NONE;
  }
  return m_type;
}

void
QZDir::setPath(QString npath)
{
  closeZ();
  m_path = npath;
}

bool
QZDir::cd(QString str)
{
  if (updateZ()==QZF_NONE) return false;

  if (str.left(2)=="..") {
    if (!this->cdUp()) return false;
    if (str.length()>3 && str.at(2)=='/')
      return this->cd(str.mid(3));
    return true;
  }

  if (m_type&QZF_ZIP) {
    QCString spath;
    toMB(spath, str, "euc-kr");

    ZEntry *entry = m_dir.z->findByName(spath);
    if (entry==NULL || !entry->isDir()){
      return false;
    }

    m_dir.z = (ZDirEntry*)entry;
    return true;
  } else {
    QString c_path;
    c_path = m_dir.q->path();
    c_path += "/" + str;

    ZEntry *entry;
    QCString zname, sname;
    int my_type = getNames(c_path, zname, sname, &entry);
    if (!(my_type&QZF_DIR))
      return false;
    if (my_type==QZF_DIR)
      return m_dir.q->cd(str);
    m_type = my_type;
    entry->grab();
    m_dir.z = (ZDirEntry*)entry;
    return true;
  }
}

bool
QZDir::cdUp()
{
  if (updateZ()==QZF_NONE) return false;
  if (m_type&QZF_ZIP) {
    ZDirEntry *entry = m_dir.z->getParent();
    if (entry) {
      m_dir.z = entry;
      return true;
    } else {
      m_dir.z->release();
      m_type = QZF_DIR;

      QString zname;
      zname = QString::fromUtf8(m_dir.z->zipFileName());

      int idx = zname.findRev('/');
      if (idx<0)
        m_dir.q = new QDir(".");
      else {
        if (idx==0)
          m_dir.q = new QDir("/");
        else
          m_dir.q = new QDir(zname.left(idx));
      }

      return true;
    }
  } else {
    return m_dir.q->cdUp();
  }
}

QString
QZDir::path()
{
  if (updateZ()==QZF_NONE) return m_path;

  if (m_type&QZF_ZIP) {
    char buffer[1024];

    if (m_dir.z->getFullName(buffer, 1024)<0)
      return m_path;
    if (buffer[0]=='/' && buffer[1]==0)
      buffer[0] = 0;

    QString zname, sname;
    zname = QString::fromUtf8(m_dir.z->zipFileName());
    toWC(sname, buffer, "euc-kr");

    //printf("getpath:%s %s\n", (const char*)zname.utf8(), (const char*)sname.utf8());

    zname += sname;
    m_path = zname;
    return zname;
  } else {
    return m_dir.q->path();
  }
}

QString
QZDir::absPath()
{
  if (updateZ()==QZF_NONE) return m_path;

  if (m_type&QZF_ZIP) {
    char buffer[1024];

    if (m_dir.z->getFullName(buffer, 1024)<0)
      return m_path;
    if (buffer[0]=='/' && buffer[1]==0)
      buffer[0] = 0;

    QString zname, sname;
    zname = QString::fromUtf8(m_dir.z->zipFileName());
    toWC(sname, buffer, "euc-kr");

    //printf("getpath:%s %s\n", (const char*)zname, (const char*)sname);
    // converting to absolute path
    QDir zdir(zname);
    zname = zdir.absPath();

    zname += sname;
    m_path = zname;
    return zname;
  } else {
    return m_dir.q->absPath();
  }
}

bool
QZDir::isDir()
{
  return (updateZ()&QZF_DIR)!=0;
}

bool
QZDir::isFile()
{
  return (updateZ()&QZF_FILE)!=0;
}

bool
QZDir::exist()
{
  return updateZ()!=QZF_NONE;
}


QStringList
QZDir::getList(QString mask, int filter, int sort_policy)
{
  if (updateZ()==QZF_NONE)
    return QStringList();

  if (m_type&QZF_ZIP) {
    ZEntry *ptr = m_dir.z->getFirst();
    QStringList to_return;
    QString str;
    int   en_type;
    to_return.append(".");
    to_return.append("..");
    while (ptr) {
      if (filter&(ptr->isDir()?QZFL_DIR:QZFL_FILE)) {
        toWC(str, QCString(ptr->name()), "euc-kr");
        to_return.append(str);
      }
      ptr = ptr->m_next;
    }
    return to_return;
  } else {
    int qspec = 0;
    if (filter&QZFL_DIR)
      qspec |= QDir::Dirs;
    if (filter&QZFL_FILE)
      qspec |= QDir::Files;
    return m_dir.q->entryList(mask, qspec);
  }
}

QString
QZDir::getDir(const QString &path, QString *filename)
{
  int idx;
  idx = path.findRev('/');
  if (idx<0) {
    if (NULL!=filename) *filename = path;
    return ".";
  }
  if (idx==0) {
    if (NULL!=filename) *filename = path.mid(1);
    return "/";
  }
  if (NULL!=filename) *filename = path.mid(idx+1);
  return path.left(idx);
}

QString
QZDir::getBasename(const QString &path)
{
  QString fname;
  QZDir::getDir(path, &fname);
  return fname;
}

bool
QZDir::isDir(QString sub)
{
  return getType(sub)==QZF_DIR;
}

bool
QZDir::isFile(QString sub)
{
  return getType(sub)==QZF_FILE;
}

int
QZDir::getType(QString sub)
{
  if (updateZ()==QZF_NONE) return QZF_NONE;

  if (sub=="." || sub=="..") return QZF_DIR;

  if (m_type&QZF_ZIP) {
    QCString    str;
    toMB(str, sub, "euc-kr");
    ZEntry *entry = m_dir.z->findByName(str);
    if (!entry) return QZF_NONE;
    if (entry->isDir())
      return QZF_DIR;
    else
      return QZF_FILE;
  } else {
    QFileInfo   finfo(m_dir.q->absPath()+"/"+sub);
    if (finfo.isFile()) {
      QString ext = finfo.extension(false);
      //printf("EXT:%s\n", (const char*)ext);
      if (ext.lower()=="zip") return QZF_DIR;

      return QZF_FILE;
    }
    if (finfo.isDir())
      return QZF_DIR;
    return QZF_NONE;
  }
}

