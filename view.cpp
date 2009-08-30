#include <qpixmap.h>
#include <qwmatrix.h>
#include <qpainter.h>
#include <stdio.h>
#include <unistd.h>
#include "view.h"
#include "document.h"
#include "aconfig.h"
#include "aview.h"

View::View(QWidget *parent, const char *name, WFlags f) :
    QWidget(parent, name, f), m_bar(this), m_timer(this)
{
    m_main = NULL;
    m_control = (AView*)parent;

    m_csize = size();
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(updateBar()));
    loadConfig();
}

void
View::loadConfig()
{
    setRotation(g_cfg->getInt(ACFG_Rotation), false);
    m_fg = g_cfg->getColor(ACFG_TextColor);
    m_bg = g_cfg->getColor(ACFG_Background);
    m_margin = g_cfg->getInt(ACFG_Margin);
    m_barHeight = g_cfg->getInt(ACFG_BarHeight);

    m_bar.loadConfig();

    m_buffer.resize(0,0);
    updateLayout(true);
}

void
View::resizeEvent(QResizeEvent *ev)
{
    QSize c_size = size();
    if (c_size == m_csize) return;
    m_csize = c_size;
    updateMatrixes(true);
}

QPixmap
View::getFrame(Content *c, QRect &r)
{
    QPixmap tmp;
    QPainter dc;

    tmp.resize(r.size());
    dc.begin(&tmp);
    c->paint(dc);
    dc.end();

    tmp = tmp.xForm(m_rmat);
    r = m_rmat.map(r);
    return tmp;
}

void
View::rebuildBuffer()
{
    QPainter dc;

    printf("View::rebuildBuffer() ENTER\n");

    m_buffer.resize(m_csize);

    if (m_main) {
        QRect r;
        QPixmap tmp;

        dc.begin(&m_buffer);

        dc.fillRect(m_buffer.rect(), m_bg);

        r = m_mrect;
        tmp = getFrame(m_main, r);
        dc.drawPixmap(r.topLeft(), tmp);

        r = m_brect;
        tmp = getFrame(&m_bar, r);
        dc.drawPixmap(r.topLeft(), tmp);

        dc.end();
    } else {
        QPixmap tmp;
        printf("View::rebuildBuffer() draw menu\n");
        tmp.resize(m_vsize);
        QRect mrect = tmp.rect();
        dc.begin(&tmp);
        dc.setBackgroundColor(m_bg);
        dc.setPen(m_fg);
        dc.setFont(QFont("fixed", 16));
        dc.eraseRect(mrect);
        QString my_str;
        my_str.sprintf(tr(
              "QAView by amateras           \n"
              "-----------------------------\n"
              "[  OK  ] - Full Screen / Menu\n"
              "[CANCEL] - Close Doc   / Quit\n"
              "[  UP  ] - Open File         \n"
              "[ LEFT ] - Rotate Left       \n"
              "[ RIGHT] - Rotate Right      \n"
              "-----------------------------\n"
              " Rotation : %3d              \n"
              "-----------------------------\n"), m_rotation);
        dc.drawText(mrect, AlignCenter, my_str);
        dc.end();
        tmp = tmp.xForm(m_rmat);
        mrect = m_rmat.map(mrect);
        dc.begin(&m_buffer);
        dc.drawPixmap(mrect.topLeft(), tmp);
        dc.end();
    }
    printf("View::rebuildBuffer() LEAVE\n");
}

void
View::paintEvent(QPaintEvent *ev)
{
    QRect urect = ev->rect();
    QRect mrect;
    QPainter dc(this);

    printf("View::paintEvent() ENTER\n");

    // clear background.
    dc.fillRect(urect, m_bg);

    // If buffer is not ready, we have nothing to do.
    if (m_buffer.isNull()) {
        rebuildBuffer();
        if (m_buffer.isNull()) {
            printf("View::paintEvent() buffer is still empty\n");
            return;
        }
    }

    mrect = urect.intersect(m_buffer.rect());
    if (mrect.isEmpty()) {
        printf("View::paintEvent() nothing to draw\n");
        return;
    }

    dc.drawPixmap(mrect.x(),mrect.y(),m_buffer, mrect.x(), mrect.y(),
            mrect.width(), mrect.height());
    printf("View::paintEvent() LEAVE\n");
}

void
View::setRotation(int angle, bool update)
{
    // angle adjustment.
    while (angle>=360) angle -= 360;
    while (angle<0) angle += 360;

    // we allow only 90 degree unit.
    if ((angle%90)!=0) {
        printf("View::setRotation(%d) invalid angle\n", angle);
        return;
    }

    // Same? then skip.
    if (m_rotation==angle) return;
    m_rotation = angle;

    updateMatrixes(update);
}

void
View::updateMatrixes(bool update)
{
    // update matrix.
    m_vmat.reset();
    m_vmat.rotate(-m_rotation);
    m_vmat = QPixmap::trueMatrix(m_vmat, m_csize.width(), m_csize.height());
    m_rmat = m_vmat.invert();

    // update m_vsize.
    QRect   trect;
    trect.setRect(0,0,m_csize.width(),m_csize.height());
    trect = m_vmat.map(trect);
    m_vsize = trect.size();

    m_buffer.resize(0,0);
    updateLayout(update);
}

void
View::rotateRight()
{
    setRotation(m_rotation+90);
}

void
View::rotateLeft()
{
    setRotation(m_rotation-90);
}

void
View::setMain(Document *d)
{
    printf("View::setMain(0x%08x)\n", d);
    if (m_main==d) return;
    m_main = d;
    if (m_main)
        m_timer.start(500);
    else
        m_timer.stop();
    m_buffer.resize(0,0);
    updateLayout(true);
}

void
View::updateLayout(bool update)
{
    QRect r;
    printf("View::updateLayout()\n");
    if (m_main) {
        printf("View::updateLayout() vsize=%dx%d\n",
                m_vsize.width(), m_vsize.height());
        m_mrect.setRect(m_margin,m_margin,
                m_vsize.width()-m_margin*2,
                m_vsize.height()-m_margin*2-m_barHeight);
        m_main->resize(m_mrect.size());
        m_brect.setRect(0, m_vsize.height()-m_barHeight,
                m_vsize.width(), m_barHeight);
        m_bar.resize(m_brect.size());
    }
    m_buffer.resize(0,0);
    if (update) repaint(false);
}

int
View::getRotation()
{
    return m_rotation;
}
void
View::updateAll()
{
    rebuildBuffer();
    repaint(false);
}

void
View::updateMain(int hint)
{
    // full update is needed.
    if (m_main==NULL || m_buffer.isNull()) {
        updateAll();
        return;
    }

    // progress in partial update.
    QRect r = m_mrect;
    QPixmap f = getFrame(m_main, r);
    QPainter dc;

    dc.begin(this);
    dc.drawPixmap(r.topLeft(), f);
    dc.end();

    dc.begin(&m_buffer);
    dc.drawPixmap(r.topLeft(), f);
    dc.end();
}

void
View::updateBar()
{
    printf("View::updateBar()\n");
    if (m_main==NULL || m_buffer.isNull()) {
        updateAll();
        return;
    }

    // progress in partial update.
    QRect r = m_brect;
    QPixmap f = getFrame(&m_bar, r);
    QPainter dc;

    dc.begin(&m_buffer);
    dc.drawPixmap(r.topLeft(), f);
    dc.end();

    dc.begin(this);
    dc.drawPixmap(r.topLeft(), f);
    dc.end();
}

Document *
View::getMain()
{
    return m_main;
}

void
View::mousePressEvent(QMouseEvent *ev)
{
    m_control->viewMousePressEvent(ev);
}

void
View::mouseReleaseEvent(QMouseEvent *ev)
{
    m_control->viewMouseReleaseEvent(ev);
}

void
View::mouseMoveEvent(QMouseEvent *ev)
{
    m_control->viewMouseMoveEvent(ev);
}
