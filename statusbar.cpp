#include <qpe/power.h>
#include <qpainter.h>
#include <qdatetime.h>
#include "aconfig.h"
#include "statusbar.h"
#include "document.h"
#include "view.h"

StatusBar::StatusBar(View *p) : Content(p)
{
}

void
StatusBar::loadConfig()
{
    m_fg = g_cfg->getColor(ACFG_TextColor);
    m_bg = g_cfg->getColor(ACFG_Background);
    m_barText = g_cfg->getColor(ACFG_BarText);
    m_barBG = g_cfg->getColor(ACFG_BarBG);
    m_barFG = g_cfg->getColor(ACFG_BarFG);
}

void
StatusBar::drawBattery(QRect area, QPainter &dc)
{
    QRect             bat_area;
    static MyBattery  power;
    //PowerStatus   power;

    dc.fillRect(area, m_bg);
    dc.setPen(m_barText);
    dc.drawRect(area);

    bat_area = area;
    bat_area.rLeft() += 2;
    bat_area.rTop() += 2;
    bat_area.rBottom() -= 2;
    bat_area.rRight() -= 2;

    m_battery.updateStatus();

    if (m_battery.acStatus() == PowerStatus::Online) {
        dc.drawText(area, Qt::AlignCenter, "A C");
        return;
    }

    int   all_width = bat_area.width()-1;
    all_width = all_width*m_battery.batteryPercentRemaining()/100;

    bat_area.rRight() = bat_area.rLeft()+all_width;
    dc.fillRect(bat_area, m_barText);
}

void
StatusBar::drawClock(QRect r, QPainter &dc)
{
    QString   to_show;
    QTime     tm;
    QDateTime tm2;

    tm = QTime::currentTime();
    to_show.sprintf("%2d:%02d", tm.hour(), tm.minute());

    dc.fillRect(r, m_bg);
    dc.setPen(m_barText);
    dc.drawText(r, Qt::AlignCenter, to_show);
}

void
StatusBar::drawProgress(QRect r, QPainter &dc)
{
    unsigned long offset=0, all=0;
    Document *doc;
    doc = m_view->getMain();
    if (doc) {
        if (!doc->getProgress(offset, all)) {
            offset = 0;
            all = 0;
        }
    }

    if (all!=0) {
        QRect   p = r;
        p.setWidth(offset*r.width()/all);
        dc.fillRect(p, m_barFG);
        r.setLeft(p.right());
        dc.fillRect(r, m_barBG);
    } else {
        dc.fillRect(r, m_barBG);
    }
}

void
StatusBar::paint(QPainter &dc)
{
    QRect r;
    r.setRect(0,0,30,m_vsize.height());
    drawBattery(r, dc);
    r.setRect(m_vsize.width()-30,0,30,m_vsize.height());
    drawClock(r, dc);
    r.setRect(30, 0, m_vsize.width()-60, m_vsize.height());
    drawProgress(r, dc);
}

void
StatusBar::resize(const QSize &s)
{
    m_vsize = s;
}
