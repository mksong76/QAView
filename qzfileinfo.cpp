#include "qzfileinfo.h"
#include <qfileinfo.h>
#include "zipfs.h"
#include "qzfs.h"


QZFileInfo::QZFileInfo(QString str)
{
  QCString  zname, sname;
  ZEntry *entry=NULL;

  m_type = getNames(str, zname, sname, &entry);
  if (m_type==QZF_NONE) return;

  if (m_type&QZF_ZIP) {
    m_dir.z = entry;
    m_dir.z->grab();
  } else {
    m_dir.q = new QFileInfo(str);
  }
}

QZFileInfo::~QZFileInfo()
{
  if (m_type==QZF_NONE) return;

  if (m_type&QZF_ZIP) {
    m_dir.z->release();
  } else {
    delete m_dir.q;
  }
}

QZFileInfo::QZFileInfo(ZEntry *entry)
{
  if (!entry) m_type = QZF_NONE;

  m_type = (entry->isDir() ? QZF_DIR : QZF_FILE)|QZF_ZIP;
  m_dir.z = entry;
}

bool
QZFileInfo::isDir()
{
  return (m_type&QZF_DIR)!=0;
}

bool
QZFileInfo::isFile()
{
  return (m_type&QZF_FILE)!=0;
}

int
QZFileInfo::getType()
{
  return m_type;
}

unsigned int
QZFileInfo::getSize()
{
  if ((m_type&QZF_FILE)==0) return 0;

  if (m_type&QZF_ZIP) {
    return (unsigned int)((ZFileEntry*)m_dir.z)->m_size;
  } else {
    return m_dir.size();
  }
}
