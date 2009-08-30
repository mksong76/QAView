#ifndef __STATUS_BAR_H__
#define __STATUS_BAR_H__

#include <qpainter.h>
#include <qsize.h>
#include "content.h"

class View;

class StatusBar : public Content {
    private:
        void drawBattery(QRect r, QPainter &dc);
        void drawClock(QRect r, QPainter &dc);
        void drawProgress(QRect r, QPainter &dc);
    public:
        StatusBar(View *p);
        void loadConfig();
        void paint(QPainter &p);
        void resize(const QSize &s);

    protected:
        QSize       m_vsize;
        MyBattery   m_battery;
        QColor      m_fg, m_bg, m_barText, m_barFG, m_barBG;
};

#endif
