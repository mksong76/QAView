#ifndef __VIEW_H__
#define __VIEW_H__

#include <qwidget.h>
#include <qpixmap.h>
#include <qwmatrix.h>
#include <qtimer.h>
#include "view.h"
#include "document.h"
#include "statusbar.h"

#define HINT_PAGE_DOWN      1
#define HINT_PAGE_UP        2

class AView;

class View : public QWidget
{
    Q_OBJECT
    private:
        void paintEvent(QPaintEvent *ev);
        void resizeEvent(QResizeEvent *ev);
        void updateMatrixes(bool update);
        void updateLayout(bool update);
        QPixmap getFrame(Content *c, QRect &r);

        void mousePressEvent(QMouseEvent *ev);
        void mouseReleaseEvent(QMouseEvent *ev);
        void mouseMoveEvent(QMouseEvent *ev);
    public:
        View(QWidget *parent=NULL, const char *name=NULL,
                WFlags f=0);

        void setRotation(int rot, bool update=true);
        int getRotation();
        void setMain(Document *d);
        Document *getMain();

    public slots:
        void rotateLeft();
        void rotateRight();
        void rebuildBuffer();
        void updateMain(int hint);
        void updateAll();
        void updateBar();
        void loadConfig();

    private:
        AView       *m_control;
        Document    *m_main;
        StatusBar   m_bar;
        QRect       m_mrect, m_brect;

        QTimer      m_timer;

        QPixmap     m_buffer;
        QColor      m_bg, m_fg;
        QSize       m_csize, m_vsize;
        int         m_margin, m_barHeight;

        // m_vmat: from real to virtual, m_rmat: from virtual to real.
        // m_rmat could be used for transforming original buffer image.
        int         m_rotation;
        QWMatrix    m_vmat, m_rmat;
};

#endif
