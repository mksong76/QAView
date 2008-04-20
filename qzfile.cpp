/**
 * File abstraction.
 * @file qzfile.cpp
 * vim:et:sw=4:ts=8:sts=4:et
 */
#include "qzfile.h"
#include "zlib.h"
#include <qfile.h>
#include <unistd.h>
#include "zfile.h"
#include "zipfs.h"
#include "zipfsi.h"
#include "qunicode.h"
#include "qzfs.h"


/**
 * Constructors
 */
QZFile::QZFile()
{
    m_fd.q = NULL;
    m_subPath = "";
    m_fullPath = "";
    m_type = QZF_NONE;
}

QZFile::QZFile(const QString &name)
{
    m_fd.q = NULL;
    m_type = QZF_UNCHK;
    m_fullPath = name;
    m_subPath = "";
}

QZFile::~QZFile()
{
    this->close();
}

/**
 * File name manifulation.
 */
void
QZFile::setName(const QString &name)
{
    if( m_fullPath != name ) {
        if( m_fd.q!=NULL )
            this->close();
        m_fullPath = name;
        m_type = QZF_UNCHK;
    }
}

QString
QZFile::name()
{
    return m_fullPath;
}

void
QZFile::updateName()
{
    if (m_type!=QZF_UNCHK) return;
    m_type = getNames(m_fullPath, m_zipPath, m_subPath);
#ifdef      ZF_DEBUG
    printf("FILE TYPE:[%s] %d\n", (const char*)m_fullPath, m_type);
    printf("FILE NAMES:%s,%s\n", (const char*)m_zipPath,
            (const char*)m_subPath);
#endif
}

/**
 * File access.
 */
bool
QZFile::open()
{
    if( m_fd.q!=NULL ) return TRUE;

    QString zip_name;
    int     idx;

    updateName();

    if (m_type==QZF_NONE) return false;

    switch (m_type&QZF_TYPEMASK) {
      case QZF_NONE:
          m_fd.q = fopen(m_fullPath.utf8(), "rb");
          break;
      case QZF_ZIP:
          m_fd.z = ZFile::open(m_zipPath, m_subPath);
          break;
      default:
          return 0;
    }

    if( m_fd.q==NULL )
        m_type = QZF_NONE;
    return m_fd.q!=NULL;
}

void
QZFile::close()
{
    if( m_fd.q!=NULL ) {
        if( m_type&QZF_TYPEMASK )
            delete m_fd.z;
        else
            fclose(m_fd.q);
        m_fd.z = NULL;
        m_subPath = "";
    }
}

bool
QZFile::exists()
{
    updateName();

    if (m_type==QZF_NONE)
        return false;
    return (m_type&(QZF_FILE|QZF_DIR))!=0;
}

int
QZFile::read(char *buffer, int length)
{
    if (m_fd.q==NULL) return -1;
    if (m_type&QZF_TYPEMASK) {
        return m_fd.z->read(buffer, length);
    } else {
        return fread(buffer, 1, length, m_fd.q);
    }
}

int
QZFile::seek(int offset, int whence)
{
    if (m_fd.q==NULL) return 0;
    if (m_type&QZF_TYPEMASK) {
        return m_fd.z->seek(offset, whence);
    } else {
        return fseek(m_fd.q, offset, whence);
    }
}

int
QZFile::tell()
{
    if (m_type&QZF_FILE) {
        if (m_type&QZF_TYPEMASK) {
            return m_fd.z->tell();
        } else {
            return ftell(m_fd.q);
        }
    }
    return 0;
}

#include <sys/stat.h>

int
QZFile::size()
{
  if (m_type&QZF_FILE) {
      if (m_type&QZF_TYPEMASK) {
          return m_fd.z->size();
      } else {
          struct stat   myst;
          fstat(fileno(m_fd.q), &myst);
          return myst.st_size;
      }
  }
  return 0;
}
