#ifndef __IMAGE_VIEW_H__
#define __IMAGE_VIEW_H__
/**
 * Image viewer.
 * vim:ts=8:sw=2:et
 */
#include <qwmatrix.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qimage.h>
#include "baseview.h"
#include "monitor.h"

/* ImageView Fit Type */
typedef enum {
  IVFM_NONE,
  IVFM_STATIC,
  IVFM_WIDTH,
  IVFM_HEIGHT,
  IVFM_BOTH
} IV_FIT_METHOD;

typedef enum {
  IVSM_NONE,
  IVSM_STATIC,
  IVSM_PORTRAIT,
  IVSM_LANDSCAPE
} IV_SPLIT_METHOD;

typedef enum {
  IVPM_DOWN_RIGHT,      // down -> right.
  IVPM_DOWN_LEFT,       // down -> left.
  IVPM_RIGHT_DOWN,      // right -> down.
  IVPM_LEFT_DOWN        // left -> down.
} IV_PAGING_MODE;

typedef enum {
  IVLC_LOAD,
  IVLC_STOP,
  IVLC_NONE
} IV_LOADER_CMD;

typedef enum {
  IVLS_LOADING,
  IVLS_WAITING,
  IVLS_STOPPING
} IV_LOADER_STATUS;


class ImageItem;
class FileLister;

class ImageView : public BaseView
{
  Q_OBJECT
  private:
    bool event(QEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void resizeEvent(QResizeEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

    void resetResized();
    void setImage(const QPixmap &img, const QSize &splits);

    bool scrollDown();
    bool scrollRight();
    bool scrollUp();
    bool scrollLeft();
    bool rightSlice();
    bool downSlice();
    bool leftSlice();
    bool upSlice();

    void handleImageReady(const QString &fname, const QPixmap &pixmap,
        const QSize &splits);
  public:
    ImageView(QWidget *parent=NULL, const char *name=NULL, WFlags f=0);
    void setSlideShowMode(int mode);
    void loadConfig();
    void setParagraph(int para);
    int getParaCount();
    bool getDocumentLocation(int &para, int &offset);
    bool hasDocument();
    bool setDocument(QString filename, int parser_id, int encoding_id,
            int para, int offset);
    void resetDocument();
    void setFileLister(FileLister *flist);

    bool canSearch();
    void search(QString key, int flags);

    void nextPage();
    void prevPage();
    void rotateRight();
    void rotateLeft();

    void resetPage();

    QPixmap loadImage(const QString &path, QSize &splits);
    QSize getProperMatrixAndSize(QWMatrix &mat, QPixmap &img);

    void backgroundLoaderLoop();
    bool checkAndStartLoader(const QString &filename);
    bool stopBackgroundLoader();
  private:
    QPixmap m_image;
    int     m_imgVSplits, m_imgHSplits;
    int     m_imgWidth, m_imgHeight;
    QString m_filename;

    int     m_xslice, m_yslice;
    int     m_xoffset, m_yoffset;

    QSize   m_deskSize;

    /* IVSM_STATIC */
    int     m_vSplit, m_hSplit;

    /* IVFM_STATIC */
    double  m_scaleFactor;
    bool    m_noScaleUp;

    IV_FIT_METHOD   m_fitMethod;
    IV_SPLIT_METHOD m_splitMethod;
    IV_PAGING_MODE  m_scrollMode;
    IV_PAGING_MODE  m_pageMode;

    FileLister  *m_flist;

    Monitor         m_bgLock;
    ImageItem       *m_loadQueue;
    int             m_queueSize;
    IV_LOADER_CMD       m_cmd;
    QString             m_cmdFile;
    IV_LOADER_STATUS    m_status;
};


#endif
