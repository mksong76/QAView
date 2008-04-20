/**
 * Image viewer.
 * vim:ts=8:sw=2:et
 */
#include <qwidget.h>
#include "imageview.h"
#include "qzfile.h"
#include <malloc.h>
#include <qpainter.h>
#include <qapplication.h>
#include "userevent.h"
#include "filelister.h"
#include "aconfig.h"


#ifndef IV_MAX_QUEUE_SIZE
#define IV_MAX_QUEUE_SIZE   2
#endif

class ImageItem
{
  public:
    typedef enum {
      LOADING,
      LOADED,
      WAITING,
      FAILED
    } Status;

    ImageItem::ImageItem(const QString &file);

    QString     m_filename;
    QPixmap     m_image;
    QSize       m_splits;
    Status      m_status;
    ImageItem   *m_next;
};

ImageItem::ImageItem(const QString &file)
{
  m_filename = file;
  m_next = NULL;
  m_status = LOADING;
}

static void *
_backgroundLoaderLoop(void *ptr)
{
  ImageView *iv = (ImageView*)ptr;
  iv->backgroundLoaderLoop();
  return NULL;
}

ImageView::ImageView(QWidget *parent, const char *name, WFlags f) :
  BaseView(parent, name, f)
{
  m_deskSize = QApplication::desktop()->size();

  m_fitMethod = IVFM_WIDTH;
  m_splitMethod = IVSM_PORTRAIT;
  m_noScaleUp = true;
  m_scrollMode = IVPM_LEFT_DOWN;
  m_pageMode = IVPM_LEFT_DOWN;

  m_loadQueue = NULL;
  m_queueSize = 0;
  m_cmd = IVLC_NONE;
  m_cmdFile = "";
  m_status = IVLS_WAITING;

  pthread_attr_t    attr;
  pthread_t         tid;

  pthread_attr_init(&attr);
  if (pthread_create(&tid, &attr, _backgroundLoaderLoop, this)!=0) {
    printf("ImageView::ImageView:fail to start background thread!\n");
  }
}

void
ImageView::setSlideShowMode(int mode)
{
}

void
ImageView::loadConfig()
{
  m_fitMethod = (IV_FIT_METHOD)g_cfg->getInt(ACFG_ScalingMethod);
  m_splitMethod = (IV_SPLIT_METHOD)g_cfg->getInt(ACFG_SlicingMethod);
  m_noScaleUp = !g_cfg->getBool(ACFG_ScaleUp);
  m_scrollMode = (IV_PAGING_MODE)g_cfg->getInt(ACFG_ScrollPolicy);
  m_pageMode = (IV_PAGING_MODE)g_cfg->getInt(ACFG_PagingPolicy);
  m_hSplit = g_cfg->getInt(ACFG_HSlicing);
  m_vSplit = g_cfg->getInt(ACFG_VSlicing);
  m_scaleFactor = ((double)g_cfg->getInt(ACFG_ScaleFactor))/100.0f;

  resetResized();
  repaint(false);
}

void
ImageView::setParagraph(int paragraph)
{
}

int
ImageView::getParaCount()
{
  if (hasDocument())
    return 1;
  return 0;
}

bool
ImageView::getDocumentLocation(int &para, int &offset)
{
  if (hasDocument()) {
    para = 0;
    offset = 0;
    return true;
  }
  return false;
}

bool
ImageView::hasDocument()
{
  return !m_filename.isNull();
}

QPixmap
ImageView::loadImage(const QString &path, QSize &splits)
{
  QZFile    *fd = NULL;
  unsigned char *buffer = NULL;
  int size, readed, ret;
  QPixmap   img, resized;
  QWMatrix  mat;

  fd = new QZFile(path);
  if (!fd->open()) {
    printf("Fail to open(%s)\n", (const char*)path);
    delete fd;
    return (QPixmap)NULL;
  }
  size = fd->size();
  buffer = (unsigned char*)malloc(size);
  if (buffer==NULL) {
    printf("Fail to malloc(%d)\n", size);
    delete fd;
    return (QPixmap)NULL;
  }
  readed = 0;
  while (readed<size) {
    ret = fd->read((char*)(buffer+readed), size-readed);
    if (ret<1) {
      printf("Fail to read(%d) readed=%d size=%d err=%d\n",
          size-readed, readed, size, ret);
      goto failure;
    }
    readed += ret;
  }
  if (!img.loadFromData(buffer, size)) {
    printf("Fail to loadImge(%08x, %d)\n",
        buffer, size);
    goto failure;
  }
  free(buffer);

  splits = getProperMatrixAndSize(mat, img);
  resized = img.xForm(mat);
  if (resized.isNull()) {
    splits = QSize(1,1);
    return img;
  } else {
    /*
    resized.detach();
    img.resize(0,0);
    */
    return resized;
  }
failure:
  if (buffer)
    free(buffer);
  if (fd)
    delete fd;
  return (QPixmap)NULL;
}


void
ImageView::setImage(const QPixmap &img, const QSize &splits)
{
  m_image = img;
  m_imgHSplits = splits.width();
  m_imgVSplits = splits.height();
  m_imgWidth = (m_image.width()+m_imgHSplits-1)/m_imgHSplits;
  m_imgHeight = (m_image.height()+m_imgVSplits-1)/m_imgVSplits;;
  printf("ImageView::setImage img=%dx%d splits=%dx%d\n",
      img.width(), img.height(), splits.width(), splits.height());
  resetPage();
}

bool
ImageView::setDocument(QString filename, int parser_id, int encoding_id,
        int para, int offset)
{
  QPixmap   img;
  QSize     splits;
  printf("ImageView::setDocument(%s)\n", (const char*)filename.utf8());
  m_flist->setFile(filename);
  if (checkAndStartLoader(filename)) {
    m_filename = filename;
    return true;
  }
  img = loadImage(filename, splits);
  if (img.isNull()) return false;
  setImage(img, splits);
  m_filename = filename;
  repaint(false);
  return true;
}

void
ImageView::resetPage()
{
  switch (m_pageMode) {
    case IVPM_DOWN_RIGHT:
    case IVPM_RIGHT_DOWN:
      m_xslice = 0;
      m_yslice = 0;
      break;
    case IVPM_DOWN_LEFT:
    case IVPM_LEFT_DOWN:
      m_xslice = m_imgHSplits-1;
      m_yslice = 0;
      break;
  }
  switch (m_scrollMode) {
    case IVPM_DOWN_RIGHT:
    case IVPM_RIGHT_DOWN:
      m_xoffset = 0;
      m_yoffset = 0;
      break;
    case IVPM_DOWN_LEFT:
    case IVPM_LEFT_DOWN:
      m_xoffset = m_imgWidth-m_deskSize.width();
      m_yoffset = 0;
      break;
  }
}

void
ImageView::resetDocument()
{
  m_image.resize(0,0);
  m_filename = (QString)NULL;
  stopBackgroundLoader();
}

bool
ImageView::canSearch()
{
  return false;
}

void
ImageView::search(QString key, int flags)
{
  /* do nothing because it can not search anything in the image */
  return;
}

bool
ImageView::scrollDown()
{
  if (m_yoffset+m_deskSize.height() < m_imgHeight) {
    m_yoffset += m_deskSize.height()*2/3;
    if (m_yoffset+m_deskSize.height()>m_imgHeight) {
      m_yoffset = m_imgHeight - m_deskSize.height();
    }
    repaint(false);
    return true;
  }
  m_yoffset = 0;
  return false;
}

bool
ImageView::scrollRight()
{
  if (m_xoffset+m_deskSize.width() < m_imgWidth) {
    m_xoffset += m_deskSize.width()*2/3;
    if (m_xoffset+m_deskSize.width()>m_imgWidth) {
      m_xoffset = m_imgWidth - m_deskSize.width();
    }
    repaint(false);
    return true;
  }
  m_xoffset = 0;
  return false;
}

bool
ImageView::scrollUp()
{
  if (m_yoffset>0) {
    m_yoffset -= m_deskSize.height()*2/3;
    repaint(false);
    return true;
  }
  m_yoffset = m_imgHeight-m_deskSize.height();
  return false;
}

bool
ImageView::scrollLeft()
{
  if (m_xoffset>0) {
    m_xoffset -= m_deskSize.width()*2/3;
    repaint(false);
    return true;
  }
  m_xoffset = m_imgWidth-m_deskSize.width();
  return false;
}

bool
ImageView::rightSlice()
{
  if (m_xslice<m_imgHSplits-1) {
    m_xslice++;
    repaint(false);
    return true;
  }
  m_xslice = 0;
  return false;
}

bool
ImageView::downSlice()
{
  if (m_yslice<m_imgVSplits-1) {
    m_yslice++;
    repaint(false);
    return true;
  }
  m_yslice = 0;
  return false;
}

bool
ImageView::leftSlice()
{
  if (m_xslice>0) {
    m_xslice--;
    repaint(false);
    return true;
  }
  m_xslice = m_imgHSplits-1;
  return false;
}

bool
ImageView::upSlice()
{
  if (m_yslice>0) {
    m_yslice--;
    repaint(false);
    return true;
  }
  m_yslice = m_imgVSplits-1;
  return false;
}

void
ImageView::nextPage()
{
  switch (m_scrollMode) {
    case IVPM_DOWN_RIGHT:
      if (scrollDown() || scrollRight()) return;
      break;
    case IVPM_DOWN_LEFT:
      if (scrollDown() || scrollLeft()) return;
      break;
    case IVPM_RIGHT_DOWN:
      if (scrollRight() || scrollDown()) return;
      break;
    case IVPM_LEFT_DOWN:
      if (scrollLeft() || scrollDown()) return;
      break;
  }
  switch (m_pageMode) {
    case IVPM_RIGHT_DOWN:
      if (rightSlice() || downSlice()) return;
      break;
    case IVPM_LEFT_DOWN:
      if (leftSlice() || downSlice()) return;
      break;
    case IVPM_DOWN_RIGHT:
      if (downSlice() || rightSlice()) return;
      break;
    case IVPM_DOWN_LEFT:
      if (downSlice() || leftSlice()) return;
      break;
  }
  emit loadNext();
}

void
ImageView::prevPage()
{
  switch (m_scrollMode) {
    case IVPM_DOWN_RIGHT:
      if (scrollLeft() || scrollUp()) return;
      break;
    case IVPM_DOWN_LEFT:
      if (scrollRight() || scrollUp()) return;
      break;
    case IVPM_RIGHT_DOWN:
      if (scrollUp() || scrollLeft()) return;
      break;
    case IVPM_LEFT_DOWN:
      if (scrollUp() || scrollRight()) return;
      break;
  }
  switch (m_pageMode) {
    case IVPM_RIGHT_DOWN:
      if (upSlice() || leftSlice()) return;
      break;
    case IVPM_LEFT_DOWN:
      if (upSlice() || rightSlice()) return;
      break;
    case IVPM_DOWN_RIGHT:
      if (leftSlice() || upSlice()) return;
      break;
    case IVPM_DOWN_LEFT:
      if (rightSlice() || upSlice()) return;
      break;
  }

  emit loadPrev();
}

void
ImageView::rotateRight()
{
}

void
ImageView::rotateLeft()
{
}


void
ImageView::paintEvent(QPaintEvent *ev)
{
  QPainter  dc(this);
  dc.fillRect(ev->rect(), this->backgroundColor());
  if (m_image.isNull()) return;

  if (m_xoffset<0) m_xoffset = 0;
  if (m_yoffset<0) m_yoffset = 0;

  dc.drawPixmap(0, 0, m_image, m_xslice*m_imgWidth+m_xoffset,
      m_yslice*m_imgHeight+m_yoffset, m_imgWidth, m_imgHeight);
}


void
ImageView::resizeEvent(QResizeEvent *ev)
{
  QSize new_desk = QApplication::desktop()->size();
  if (m_deskSize!=new_desk) {
    m_deskSize = new_desk;
    resetResized();
  }
  repaint(false);
}

void
ImageView::resetResized()
{
  if (m_filename.isNull()) return;

  QSize     splits;
  QPixmap   img = loadImage(m_filename, splits);
  if (!img.isNull()) {
    setImage(img, splits);
  }
  if (!stopBackgroundLoader()) {
    checkAndStartLoader(m_filename);
  }
}

bool
ImageView::event(QEvent *ev)
{
  if (ev->type()==(QEvent::Type)QAVIEW_IMAGE_READY) {
    UEImageReady    *ue = (UEImageReady*)ev;
    handleImageReady(ue->file(), ue->image(), ue->splits());
    return true;
  }
  return QWidget::event(ev);
}

QSize
ImageView::getProperMatrixAndSize(QWMatrix &mat, QPixmap &img)
{
  int       width, height;
  int       vsplit, hsplit;
  double    scale = 1.0f;

  width = img.width();
  height = img.height();

  switch (m_splitMethod) {
    case IVSM_NONE:
      vsplit = 1;
      hsplit = 1;
      break;
    case IVSM_STATIC:
      vsplit = m_vSplit;
      hsplit = m_hSplit;
      break;
    case IVSM_PORTRAIT:
      hsplit = 1;
      while (width/hsplit>height) hsplit++;
      vsplit = 1;
      break;
    case IVSM_LANDSCAPE:
      vsplit = 1;
      while (height/vsplit>width) hsplit++;
      hsplit = 1;
      break;
  }
  if (hsplit>1)
    width = (width+hsplit-1)/hsplit;
  if (vsplit>1)
    height = (height+vsplit-1)/vsplit;


  switch (m_fitMethod) {
    case IVFM_NONE:
      scale = 1.0f;
      break;
    case IVFM_STATIC:
      scale = m_scaleFactor;
      break;
    case IVFM_WIDTH:
      scale = (double)m_deskSize.width()/width;
      break;
    case IVFM_HEIGHT:
      scale = (double)m_deskSize.height()/height;
      break;
    case IVFM_BOTH:
      scale = (double)m_deskSize.height()/height;
      if ((double)m_deskSize.width()/width<scale)
        scale = (double)m_deskSize.width()/width;
      break;
  }
  if (m_noScaleUp && m_fitMethod!=IVFM_STATIC && scale>1.0f) {
    scale = 1.0f;
  }
  mat.reset();
  mat.scale(scale, scale);

  return QSize(hsplit, vsplit);
}

void
ImageView::backgroundLoaderLoop()
{
  QString   fname;
  m_bgLock.lock();
  while (1) {
    if (m_cmd==IVLC_LOAD) {
      m_cmd = IVLC_NONE;
      m_status = IVLS_LOADING;
      fname = m_cmdFile;
      m_bgLock.unlock();

      printf("ImageView::backgroundLoaderLoop : loading %s\n",
          (const char*)fname.utf8());

      QPixmap   px;
      QSize     splits;
      px = loadImage(fname, splits);
      m_bgLock.lock();
      QApplication::postEvent(this, new UEImageReady(fname, px, splits));
    } else if (m_cmd==IVLC_NONE){
      m_status = IVLS_WAITING;
      m_bgLock.wait();
    } else if (m_cmd==IVLC_STOP) {
      m_status = IVLS_STOPPING;
      m_bgLock.unlock();
      break;
    }
  }
}

void
ImageView::handleImageReady(const QString &fname, const QPixmap &image,
    const QSize &splits)
{
  ImageItem **ptr = &m_loadQueue;

  printf("ImageView::handleImageReady(%s,%s)\n",
      (const char*)fname.utf8(),
      image.isNull()?"NULL":"VALID");

  while (*ptr) {
    if ((*ptr)->m_filename==fname) {
      break;
    }
    ptr = &(*ptr)->m_next;
  }
  if (NULL==*ptr) {
    checkAndStartLoader(m_filename);
    return;
  }

  if ((*ptr)->m_status==ImageItem::WAITING) {
    ImageItem *todel = *ptr;
    *ptr = todel->m_next;
    m_queueSize -= 1;

    if (image.isNull()) {
      emit loadNext();
      return;
    } else {
      setImage(image, splits);
      repaint(false);
    }
    delete todel;
  } else {
    if (image.isNull()) {
      (*ptr)->m_status = ImageItem::FAILED;
    } else {
      (*ptr)->m_image = image;
      (*ptr)->m_splits = splits;
      (*ptr)->m_status = ImageItem::LOADED;
    }
  }
  checkAndStartLoader(m_filename);
}

bool
ImageView::stopBackgroundLoader()
{
  bool to_return = false;

  m_bgLock.lock();
  m_cmd = IVLC_NONE;
  m_bgLock.unlock();

  ImageItem *ptr, *todel;
  ptr = m_loadQueue;
  while (ptr) {
    todel = ptr;
    ptr = ptr->m_next;
    if (todel->m_status==ImageItem::LOADING)
      to_return = true;
    delete todel;
  }
  m_queueSize = 0;
  m_loadQueue = NULL;
  return to_return;
}

bool
ImageView::checkAndStartLoader(const QString &filename)
{
  int cidx;

  cidx = m_flist->findPath(filename);
  if (cidx<0) return false;

  ImageItem *found[IV_MAX_QUEUE_SIZE+1];
  memset(found, 0, sizeof(found));

  ImageItem *ptr, *todel;

  int   qidx;
  bool  hasLoading = false;
  ptr = m_loadQueue;
  while (ptr) {
    todel = ptr;
    ptr = ptr->m_next;

    if (todel->m_status==ImageItem::LOADING) {
      hasLoading = true;
    }

    qidx = m_flist->findPath(todel->m_filename);
    if (qidx>=cidx && qidx<=cidx+IV_MAX_QUEUE_SIZE) {
      found[qidx-cidx] = todel;
    } else {
      printf("ImageView::check drop %s\n",
          (const char*)todel->m_filename.utf8());
      delete todel;
    }
  }

  m_loadQueue = NULL;
  m_queueSize = 0;
  ImageItem **last = &m_loadQueue;

  bool to_return = false;

  if (found[0]) {
    switch (found[0]->m_status) {
      case ImageItem::LOADED:
        setImage(found[0]->m_image, found[0]->m_splits);
        delete found[0];
        repaint(false);
        printf("ImageView::check : FOUND LOADED (%s)\n",
            (const char*)filename.utf8());
        to_return = true;
        break;
      case ImageItem::LOADING:
        m_image = (QPixmap)NULL;
        found[0]->m_status = ImageItem::WAITING;
        *last = found[0];
        last = &(*last)->m_next;
        m_queueSize += 1;
        repaint(false);
        printf("ImageView::check : FOUND LOADING (%s)\n",
            (const char*)filename.utf8());
        to_return = true;
        break;
      default:
      case ImageItem::FAILED:
        delete found[0];
        to_return = false;
        break;
    }
  }

  /* restore loading queue */
  int   idx;
  for (idx=1 ; idx<IV_MAX_QUEUE_SIZE+1 ; idx++) {
    if (found[idx]) {
      *last = found[idx];
      last = &(*last)->m_next;
      m_queueSize += 1;
    } else {
      if (!hasLoading) {
        QString filename = m_flist->getFile(cidx+idx);
        if (!filename.isNull()) {
          *last =  new ImageItem(filename);
          last = &(*last)->m_next;
          hasLoading = true;
          m_queueSize += 1;

          m_bgLock.lock();
          m_cmd = IVLC_LOAD;
          m_cmdFile = filename;
          m_bgLock.signal();
          m_bgLock.unlock();
        }
      }
    }
  }
  *last = NULL;
  return to_return;
}

void
ImageView::setFileLister(FileLister *flist)
{
  m_flist = flist;
}
