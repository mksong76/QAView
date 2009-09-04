/**
 * File Selection dialog.
 *
 * vim:sw=2:et
 */
#include "fileselect.h"

#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include "qzdir.h"
#include "aview.h"
#include <stdio.h>
#include <qpe/resource.h>


FileSelect::FileSelect( QWidget* parent,  const char* name, WFlags fl )
    : FileSelectForm( parent, name, fl )
{
    if ( !name ) setName( "FileSelect" );

    connect(m_btnOpen, SIGNAL(pressed()), this, SLOT(openFile()));
    connect(m_btnReload, SIGNAL(pressed()), this, SLOT(openDirectory()));
    connect(m_btnClose, SIGNAL(pressed()), this, SIGNAL(endOpenFile()));
    connect(m_btnParent, SIGNAL(pressed()), this, SLOT(cdUp()));
    connect(m_tPathName, SIGNAL(activated(int)), this, SLOT(openDirectory()));

    connect(m_wFileList, SIGNAL(doubleClicked(QListViewItem*)),
        this, SLOT(itemClicked(QListViewItem*)));
    connect(m_wFileList, SIGNAL(returnPressed(QListViewItem*)),
        this, SLOT(itemClicked(QListViewItem*)));

    m_btnClose->setIconSet(Resource::loadPixmap("qaview/fileclose"));
    m_btnReload->setIconSet(Resource::loadPixmap("qaview/reload"));
    m_btnOpen->setIconSet(Resource::loadPixmap("qaview/fileopen"));
    m_btnParent->setIconSet(Resource::loadPixmap("qaview/up"));
    m_btnReload->setFixedWidth(m_btnReload->sizeHint().height());
    m_btnClose->setFixedWidth(m_btnClose->sizeHint().height());
    m_btnOpen->setFixedWidth(m_btnOpen->sizeHint().height());
    m_btnParent->setFixedWidth(m_btnParent->sizeHint().height());

    m_dir = new QZDir(".");

    m_pDir = Resource::loadPixmap("slfolder");
    m_pFile = Resource::loadPixmap("slUnknown14");

    refreshDirectory();

    setFocusPolicy(QWidget::StrongFocus);
    m_wFileList->setFocusProxy(this);

    connect(m_wParser, SIGNAL(activated(int)), this, SLOT(selectParser(int)));
    selectParser(0);
}

/*
 *  Destroys the object and frees any allocated resources
 */
FileSelect::~FileSelect()
{
    // no need to delete child widgets, Qt does it all for us
  delete m_dir;
}

void
FileSelect::keyPressEvent(QKeyEvent *ev)
{
  //printf("FileSelect::keyPressEvent(0x%04x)\n", ev->key());
  QListViewItem *cptr = m_wFileList->currentItem();

  switch (ev->key()) {
    case Qt::Key_Escape:
      m_key = ev->key();
      m_stamp.setToNow();
      break;
    case Qt::Key_Left:
    case Qt::Key_Backspace:
      this->cdUp();
      break;
    case Qt::Key_Down:
    case Qt::Key_Up:
      if (cptr) {
        if (ev->key()==Qt::Key_Down) {
          cptr = cptr->itemBelow();
        } else {
          cptr = cptr->itemAbove();
        }

        if (cptr) {
          m_wFileList->setCurrentItem(cptr);
          m_wFileList->ensureItemVisible(cptr);
        }
      }
      break;
    case Qt::Key_Right:
    case Qt::Key_Return:
      if (cptr) {
        this->itemClicked(cptr);
      }
      break;
    default :
      ev->ignore();
  }
}

void
FileSelect::cdUp()
{
  QString   pdir = m_dir->absPath();
  if (m_dir->cdUp()) {
    QString   nfile = pdir.mid(m_dir->absPath().length()+1);
    refreshDirectory();
    setFile(nfile);
  }
}

void
FileSelect::keyReleaseEvent(QKeyEvent *ev)
{
  //printf("FileSelect::keyPressEvent(0x%04x)\n", ev->key());
  int holding=0;

  if (m_key==ev->key())
      holding = m_stamp.passed();
  m_key = 0;

  switch (ev->key()) {
    case Qt::Key_Escape:
      if (holding<=500)
          emit endOpenFile();
      else
        cdUp();
      break;
    default :
      ev->ignore();
  }
}

FILE_TYPE
FileSelect::getFileType()
{
  switch (m_wParser->currentItem()) {
    case 0:
      return FT_AUTO;
    case 1:
      return FT_TEXT;
    case 2:
      return FT_IMAGE;
  }
}

FILE_ENCODING
FileSelect::getFileEncoding()
{
  if (m_wParser->currentItem()==1) {
    switch (m_wEncoding->currentItem()) {
      case 0:
        return FE_AUTO;
      case 1:
        return FE_EUC_KR;
      case 2:
        return FE_JOHAB;
      case 3:
        return FE_UTF8;
      case 4:
        return FE_UNICODE_LE;
      case 5:
        return FE_UNICODE_BE;
    }
  } else {
    return FE_AUTO;
  }
}

void
FileSelect::itemClicked(QListViewItem *item)
{
  //printf("FileSelect::itemClicked(0x%p)\n", item);
  if (item) {
    QString str = item->text(0);
    if (m_dir->cd(str)) {
      //printf("cd(%s) success!!\n", (const char*)str);
      refreshDirectory();
    } else {
      QString path;
      path = m_dir->absPath();
      path += "/" + str;
      emit openFile(path, getFileType(), getFileEncoding());
      emit endOpenFile();
    }
  }
}

void
FileSelect::openDirectory()
{
  QString new_dir = m_tPathName->currentText();
  QZDir  *n_path = new QZDir(new_dir);

  if (n_path->exist()) {
    delete m_dir;
    m_dir = n_path;
    refreshDirectory();
    m_wFileList->setFocus();
  } else {
    m_tPathName->setEditText(m_dir->absPath());
  }
}

void
FileSelect::openFile()
{
  QListViewItem *ptr = m_wFileList->selectedItem();
  if (ptr) this->itemClicked(ptr);
}

void
FileSelect::refreshDirectory()
{
  m_tPathName->setEditText(m_dir->absPath());

  m_wFileList->clear();
  QStringList   list = m_dir->getList("*", QZFL_ALL, 0);
  QListViewItem *item;
  int idx;
  for (idx=0 ; idx<list.count() ; idx++) {
    item = new QListViewItem( m_wFileList, list[idx]);
    item->setPixmap(0, m_dir->isDir(list[idx]) ? m_pDir : m_pFile);
    m_wFileList->insertItem(item);
  }
}

QString
FileSelect::getDirectory()
{
  return m_dir->absPath();
}

void
FileSelect::setDirectory(const QString &str)
{
  QZDir     *new_one = new QZDir();
  QString   n_path = str;
  QString   o_path;
  QString   n_name = "";
  int       idx;

  /* finding new path and name */
  while (1) {
    new_one->setPath(n_path);
    if (!new_one->exist()) {
      idx = n_path.findRev('/');
      if (idx<0) break;
      n_name = n_path.mid(idx+1);
      n_path = n_path.left(idx);
    } else {
      break;
    }
  }

  /* set to new path and update file list */
  if (m_dir->absPath()!=n_path) {
    if (m_dir) delete m_dir;
    m_dir = new_one;

    m_tPathName->setEditText(n_path);
    refreshDirectory();
  } else {
    delete new_one;
  }
  setFile(n_name);
}

void
FileSelect::setFile(const QString &name)
{
  QListViewItem *ptr = m_wFileList->firstChild();
  while (ptr) {
    if (ptr->text(0)==name) {
      m_wFileList->setSelected(ptr, true);
      m_wFileList->ensureItemVisible(ptr);
      return;
    }
    ptr = ptr->nextSibling();
  }
}

void
FileSelect::selectParser(int idx)
{
  switch (idx) {
    case 0: /* AUTO */
      m_wEncoding->clear();
      break;
    case 1: /* TEXT */
      m_wEncoding->clear();
      m_wEncoding->insertItem("AUTO");
      m_wEncoding->insertItem("EUC-KR");
      m_wEncoding->insertItem("JOHAB");
      //m_wEncoding->insertItem("Unicode LE");
      //m_wEncoding->insertItem("Unicode BE");
      break;
    case 2: /* IMAGE */
      m_wEncoding->clear();
      m_wEncoding->insertItem("AUTO");
  }
}
