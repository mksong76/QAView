/*
  vim:sw=2:et
 */
#include "aview.h"
#include <qtoolbar.h>
#include <qpe/resource.h>
#include <stdio.h>
#include "qzfile.h"
#include "parser.h"
#include "fileselect.h"
#include <qpe/qpeapplication.h>
#include <qlistview.h>
#include "aconfig.h"
#include "optiondlg.h"
#include "askdlg.h"
#include "textview.h"
#include "imageview.h"
#include "baseview.h"
#include <qpe/qpetoolbar.h>
#include <qpe/qpemenubar.h>
#include "filelister.h"

#define     ICON_FILE_CLOSE     "qaview/fileclose"
#define     ICON_FILE_OPEN      "qaview/fileopen"
#define     ICON_OPTIONS        "qaview/options"
#define     ICON_ROTATE_LEFT    "qaview/rotatel"
#define     ICON_ROTATE_RIGHT   "qaview/rotater"

QPixmap
IconScale(QString pix)
{
  return Resource::loadPixmap(pix);
}

AView::AView(QMainWindow *parent,const char*name, WFlags f)
    : QMainWindow (parent,name,f|WStyle_Customize|WStyle_NoBorder|
        WType_TopLevel|WStyle_NoBorderEx)
{
  setCaption(tr("AView"));

  /* composing toolbar and menu */
  m_tbMain = new QPEToolBar(this);
  m_tbMain->setHorizontalStretchable(true);
  m_tbMenu = new QPEMenuBar(m_tbMain);

  m_mnFile = new QPopupMenu(this);
  m_mnRescent = new QPopupMenu(this);
  m_mnPopup = new QPopupMenu(this);
  m_tbIcon = new QPEToolBar(this);

  m_open = new QAction("Open", IconScale(ICON_FILE_OPEN),
                            tr("&Open"), 0, this, 0);
  connect(m_open, SIGNAL(activated()), this, SLOT(openFile()));

  m_close = new QAction("Close", IconScale(ICON_FILE_CLOSE),
                            tr("&Close"), 0, this, 0);
  connect(m_close, SIGNAL(activated()), this, SLOT(closeFile()));

  m_option = new QAction("Options", IconScale(ICON_OPTIONS),
                            tr("Op&tions"), 0, this, 0);
  connect(m_option, SIGNAL(activated()), this, SLOT(showOptions()));

  m_rotateRight = new QAction("Rotate Right", IconScale(ICON_ROTATE_RIGHT),
                            tr("Rotate &Right"), 0, this, 0);
  connect(m_rotateRight, SIGNAL(activated()), this, SLOT(rotateRight()));

  m_rotateLeft = new QAction("Rotate Left", IconScale(ICON_ROTATE_LEFT),
                            tr("Rotate &Left"), 0, this, 0);
  connect(m_rotateLeft, SIGNAL(activated()), this, SLOT(rotateLeft()));

  m_open->addTo(m_mnFile);
  m_close->addTo(m_mnFile);
  m_option->addTo(m_mnFile);
  m_rotateLeft->addTo(m_mnFile);
  m_rotateRight->addTo(m_mnFile);

  m_open->addTo(m_tbIcon);
  m_option->addTo(m_tbIcon);
  m_close->addTo(m_tbIcon);

  m_tbMenu->insertItem(tr("&File"), m_mnFile);
  m_tbMenu->insertItem(tr("&Rescent"), m_mnRescent);

  connect(m_mnRescent, SIGNAL(activated(int)),
                          this, SLOT(rescentSelected(int)));
  connect(m_mnPopup, SIGNAL(activated(int)),
                          this, SLOT(rescentSelected(int)));
  setToolBarsMovable(false);
  addToolBar(m_tbMain);
  addToolBar(m_tbIcon);
  m_tbMenu->show();

  m_flist = new FileLister();

  /* composing main view!! */
  m_tview = new TextView(this, "main_view");
  m_iview = new ImageView(this, "image_view");
  m_iview->hide();
  m_view = m_tview;
  m_mode = VIEW_NONE;
  m_fullScreen = false;
  m_fullBeforeDialog = false;

  m_iview->setFileLister(m_flist);

  connect(m_tview, SIGNAL(loadNext()), this, SLOT(loadNext()));
  connect(m_tview, SIGNAL(loadPrev()), this, SLOT(loadPrev()));
  connect(m_iview, SIGNAL(loadNext()), this, SLOT(loadNext()));
  connect(m_iview, SIGNAL(loadPrev()), this, SLOT(loadPrev()));

  m_tbHeight = m_tbIcon->minimumSizeHint().height();
  if (m_tbMain->minimumSizeHint().height()>m_tbHeight)
    m_tbHeight = m_tbMain->minimumSizeHint().height();

  showMaximized();
  updateViewSize();

  m_browser = new FileSelect(this, "file_select");
  m_browser->hide();
  connect(m_browser, SIGNAL(endOpenFile()), this, SLOT(closeDialog()));
  connect(m_browser, SIGNAL(openFile(QString,int,int)),
      this, SLOT(openFile(QString,int,int)));

  m_optiondlg = new OptionDlg(this, "option_dialog");
  m_optiondlg->hide();
  connect(m_optiondlg, SIGNAL(endDialog()), this, SLOT(closeDialog()));
  connect(m_optiondlg, SIGNAL(settingChanged()),
      this, SLOT(reloadConfiguration()));

  /* Dialog for jumping to specified location */
  m_wAsk = new AskDialog(this);
  m_wAsk->hide();
  connect(m_wAsk, SIGNAL(jumpTo(int)), this, SLOT(setParagraph(int)));
  connect(m_wAsk, SIGNAL(search(QString, int)),
                                  this, SLOT(search(QString, int)));

  /* initialize values */
  m_closeReal = false;
  m_isDialog = false;
  showNormal();

  /* load configuratin */
  m_tview->loadConfig();
  m_iview->loadConfig();

  /* load configuration */
  if (g_cfg->getBool(ACFG_LoadLastFile)) {
    QString last_file = g_cfg->getLastFile();
    if (!last_file.isNull()) {
      openFile(last_file);
      m_browser->setDirectory(last_file);
    }
  } else {
    if (g_cfg->getBool(ACFG_LoadLastDir)) {
      m_browser->setDirectory(g_cfg->getLastDir());
    }
  }
  updateRescentFiles();

  setFocusPolicy(QWidget::StrongFocus);
  this->setFocus();
}

AView::~AView()
{
  /* do nothing?? */
}

void
AView::resizeEvent(QResizeEvent *ev)
{
  QSize dsize = QApplication::desktop()->size();
  QSize newsize = ev->size();
  printf("AView::resizEvent(%d,%d)\n", newsize.width(), newsize.height());
  if (m_fullScreen) {
    hide();
    showMaximized();
  }
  updateViewSize();
  QMainWindow::resizeEvent(ev);
}

void
AView::keyPressEvent(QKeyEvent *ev)
{
  if (ev->isAutoRepeat()) return;
  printf("AView::keyPressEvent(0x%04x)\n", ev->key());
  switch (ev->key()) {
    case Qt::Key_Space:
      m_view->nextPage();
      break;
    case Qt::Key_Down:
      if (!m_view->hasDocument()) {
        openFile();
      } else {
        m_keyPressed = ev->key();
        m_keyTime.setToNow();
      }
      break;
    case Qt::Key_Up:
      if (!m_view->hasDocument()) {
        openFile();
      } else {
        m_keyPressed = ev->key();
        m_keyTime.setToNow();
      }
      break;
    case Qt::Key_Right:
      m_view->rotateRight();
      break;
    case Qt::Key_Left:
      m_view->rotateLeft();
      break;
    case Qt::Key_F33:
    case Qt::Key_Return:
      m_keyPressed = Qt::Key_Return;
      m_keyTime.setToNow();
      break;
    case Qt::Key_O:
      openFile();
      break;
    case Qt::Key_T:
      showOptions();
      break;
    case Qt::Key_S:
      m_view->setSlideShowMode(2);
      return;
    case Qt::Key_Escape:
      m_keyPressed = Qt::Key_Escape;
      m_keyTime.setToNow();
      break;
    case Qt::Key_G:
      showJump();
      break;
    case Qt::Key_Slash:
      showSearch();
      break;
    default:
      ev->ignore();
      break;
  }
  m_view->setSlideShowMode(0);
}

void
AView::keyReleaseEvent(QKeyEvent *ev)
{
  if (ev->isAutoRepeat()) return;
  printf("AView::keyReleaseEvent(0x%04x)\n", ev->key());

  int holding = 0;
  int p_key = ev->key();
  if (p_key==Qt::Key_F33)
    p_key = Qt::Key_Return;

  if (m_keyPressed==p_key) {
    holding = m_keyTime.passed();
  } else {
    m_keyPressed = Qt::Key_unknown;
    ev->ignore();
    return;
  }
  m_keyPressed = Qt::Key_unknown;

  switch(p_key) {
    case Qt::Key_Escape:
      if (holding>500)
        this->closeNow();
      else
        this->closeFile();
      break;
    case Qt::Key_Return:
      if (holding>500)
        this->showPopup();
      else
        this->toggleFullScreen();
      break;
    case Qt::Key_Down:
      if (holding>500)
        m_view->setSlideShowMode(2);
      else
        m_view->nextPage();
      break;
    case Qt::Key_Up:
      if (holding>500) {
        showJump();
      } else {
        m_view->prevPage();
      }
      break;
    default:
      ev->ignore();
      break;
  }
}

void
AView::closeEvent(QCloseEvent *ev)
{
  if (m_closeReal) {
    ev->accept();
    return;
  }
  //printf("AView::closeEvent!!!\n");
  ev->ignore();
}

void
AView::openFile()
{
  openDialog(m_browser);
  if (m_view->hasDocument()) {
    m_browser->setDirectory(m_filename);
  } else {
    if (!g_cfg->getBool(ACFG_LoadLastDir)) {
      m_browser->setDirectory(g_cfg->getLastFile());
    }
  }
  m_browser->m_wFileList->setFocus();
  m_browser->repaint();
}

void
AView::closeDialog()
{
  if (m_isDialog) {
    setUpdatesEnabled(false);
    m_dialog->hide();
    setCentralWidget(NULL);
    m_isDialog = false;
    m_view->show();

    if (!m_fullBeforeDialog) {
      m_tbMain->show();
      m_tbIcon->show();
      setUpdatesEnabled(true);
      update();
    } else {
      showScreen(true);
    }
    this->setFocus();
  }
}

void
AView::openDialog(QWidget *w)
{
  m_fullBeforeDialog = m_fullScreen;

  setUpdatesEnabled(false);

  if (m_fullScreen) {
    m_fullScreen = false;
    showNormal();
  }
  m_tbMain->hide();
  m_tbIcon->hide();
  m_view->hide();
  m_isDialog = true;
  m_dialog = w;
  setCentralWidget(m_dialog);
  w->show();
  w->setFocus();

  setUpdatesEnabled(true);

  update();
}

#include <qlabel.h>
#include <qapplication.h>

void
AView::openFile(QString name)
{
  openFile(name, 0, 0);
}

void
AView::openFile(QString name, int parser_id, int encoding_id)
{
  int       para, offset, old_para, old_offset;
  QString   old_filename;

  QLabel    my_label(tr("Open file.... "), this);
  my_label.setMargin(20);
  my_label.resize(200, 60);
  my_label.move( (width()-my_label.width())/2, (height()-my_label.height())/2);
  my_label.show();
  qApp->processEvents();

  /* store last document information */
  if (!m_view->getDocumentLocation(old_para, old_offset)) {
    old_filename = (const char*)NULL;
    old_para = -1;
    old_offset = -1;
  } else {
    old_filename = m_filename;
  }

  g_cfg->getLastPosition(name, para, offset);

  BaseView  *new_view;
  int       new_mode;
  {
    int new_type = FileLister::getType(name);
    switch (new_type) {
      default:
        new_view = m_tview;
        new_mode = VIEW_TEXT;
        break;
      case FLT_IMAGE:
        new_view = m_iview;
        new_mode = VIEW_IMAGE;
        break;
      case FLT_TEXT:
        new_view = m_tview;
        new_mode = VIEW_TEXT;
        break;
    }
  }

  /* change to new file */
  /* TODO we should select appropriate viewer for the file. */
  if (new_view->setDocument(name, parser_id, encoding_id, para, offset)) {
    if (!old_filename.isNull()) {
      g_cfg->setLastPosition(old_filename, old_para, old_offset);
      updateRescentFiles();
    }
    setCaption(tr("AView - %1").arg(name));
    m_mode = new_mode;
    m_filename = name;
    showView();
  } else {
    printf("fail to setDocument(%s)\n", (const char*)name.utf8());
  }
}

void
AView::showMaximized()
{
  printf("AView::showMaximized:full=%s\n", m_fullScreen?"TRUE":"FALSE");
  if (!m_fullScreen) {
    showNormal();
  } else {
    hide();
    showFullScreen();
    resize(QPEApplication::desktop()->size());
    show();
    setFocus();
  }
}
void
AView::showNormal()
{
  printf("AView::showNormal:full=%s\n", m_fullScreen?"TRUE":"FALSE");
  QMainWindow::showNormal();
  QSize dsize = QPEApplication::desktop()->size();
  dsize.rheight()-=38;
  clearWFlags(WStyle_StaysOnTop);
  resize(dsize);
  setFocus();
}

void
AView::showScreen(bool isFull)
{
  printf("AView::showScreen(%d->%d)\n", m_fullScreen, isFull);

  if (isFull!=m_fullScreen) {
    setUpdatesEnabled(false);
    if (isFull) {
      m_fullScreen = true;
      showFullScreen();
    } else {
      m_fullScreen = false;
      showNormal();
    }
    /* when text view mode!! we must change toolbar status */
    if (centralWidget()==NULL) {
      if (m_fullScreen) {
        m_tbMain->hide();
        m_tbIcon->hide();
      } else {
        m_tbMain->show();
        m_tbIcon->show();
      }
    }
    setUpdatesEnabled(true);
    update();
  }
  updateViewSize();
  repaint();
}

void
AView::updateViewSize()
{
  if (!m_fullScreen) {
    m_view->setGeometry(0,m_tbHeight,width(), height()-m_tbHeight);
  } else {
    QSize dsize = QApplication::desktop()->size();
    m_view->setGeometry(0, 0, dsize.width(), dsize.height());
  }
}

void
AView::toggleFullScreen()
{
  showScreen(!m_fullScreen);
}

void
AView::updateRescentFiles()
{
  m_mnRescent->clear();
  QStringList   lst = g_cfg->getRescentFiles();
  QString       f_name;
  int idx=0;
  for (idx=0 ; idx<lst.count() ; idx++) {
    m_mnRescent->insertItem(lst[idx], idx, idx);
  }
}

void
AView::rescentSelected(int id)
{
  if (id>=0) {
    openFile(g_cfg->getRescentFiles()[id]);
  }
}

void
AView::showOptions()
{
  m_optiondlg->loadSettings();
  openDialog(m_optiondlg);
}


void
AView::showPopup()
{
  QRect mrect = rect();

  m_mnPopup->clear();
  m_open->addTo(m_mnPopup);
  m_close->addTo(m_mnPopup);
  m_option->addTo(m_mnPopup);
  m_rotateLeft->addTo(m_mnPopup);
  m_rotateRight->addTo(m_mnPopup);
  m_mnPopup->insertSeparator();

  int idx=0;
  QString name;
  QStringList   lst = g_cfg->getRescentFiles();
  for (idx=0 ; idx<lst.count() && idx<10 ; idx++) {
    name = lst[idx].right(20);
    m_mnPopup->insertItem(m_browser->m_pFile, name, idx);
  }

  QSize p_size = m_mnPopup->sizeHint();
  QPoint    popupPoint((mrect.width()-p_size.width())/2,
      (mrect.height()-p_size.height())/2);
  popupPoint = mapToGlobal(popupPoint);

  m_mnPopup->setActiveItem(0);

  m_mnPopup->exec(popupPoint, 0);

  /* restore to full screen mode */
  if (m_fullScreen) {
    hide();
    show();
  }
}

void
AView::rotateRight()
{
  m_view->rotateRight();
}

void
AView::rotateLeft()
{
  m_view->rotateLeft();
}

/**
 * close file.
 * it will pop document if pushed document exist.
 */
void
AView::closeFile()
{
  if (m_view->hasDocument()) {
    int     para, offset;
    if (m_view->getDocumentLocation(para, offset)) {
      g_cfg->setLastPosition(m_filename, para, offset);
    }
    m_filename = (const char*)NULL;
    m_view->resetDocument();
    setCaption(tr("AView"));
    m_mode = VIEW_NONE;
    showView();
    m_view->repaint(false);
  } else {
    closeNow();
  }
}

void
AView::closeNow()
{
  if (m_view->hasDocument()) {
    int     para, offset;
    if (m_view->getDocumentLocation(para, offset)) {
      g_cfg->setLastPosition(m_filename, para, offset);
    }
  }
  g_cfg->setLastDir(m_browser->getDirectory());
  g_cfg->sync();
  m_closeReal = true;
  close();
}

void
AView::setParagraph(int para)
{
  m_view->setParagraph(para);
}

void
AView::search(QString key, int flags)
{
  m_view->search(key, flags);
}

void
AView::showJump()
{
  if (m_view->hasDocument()) {
    int     para, offset;
    if (m_view->getDocumentLocation(para, offset)) {
      m_wAsk->jumpParagraph(para, m_view->getParaCount());
    }
  }
}

void
AView::showSearch()
{
  if (m_view->hasDocument() && m_view->canSearch()) {
    m_wAsk->searchParagraph("");
  }
}

void
AView::reloadConfiguration()
{
  m_view->loadConfig();
}

void
AView::showView()
{
  BaseView  *new_view;
  switch (m_mode) {
    default:
    case VIEW_NONE:
    case VIEW_TEXT:
      new_view = m_tview;
      break;
    case VIEW_IMAGE:
      new_view = m_iview;
      break;
  }
  if (m_view && m_view!=new_view) {
    m_view->resetDocument();
    m_view->hide();
  }
  if (m_view!=new_view) {
    m_view = new_view;
    m_view->show();
    updateViewSize();
  }
}

void
AView::loadNext()
{
  if (m_view->hasDocument()) {
    int idx;
    QString new_file;
    idx = m_flist->setFile(m_filename);
    if (idx<0) return;
    new_file = m_flist->getFile(idx+1);
    if (new_file.isNull()) return;
    openFile(new_file, 0, 0);
  }
}

void
AView::loadPrev()
{
  if (m_view->hasDocument()) {
    int idx;
    QString new_file;
    idx = m_flist->setFile(m_filename);
    if (idx<1) return;
    new_file = m_flist->getFile(idx-1);
    if (new_file.isNull()) return;
    openFile(new_file, 0, 0);
  }
}
