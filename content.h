#ifndef ___CONTENT_H__
#define ___CONTENT_H__

#include <qsize.h>
#include <qrect.h>
#include <qpainter.h>
#include "battery.h"

class View;

class Content {
    public:
        Content(View *p);
        virtual void loadConfig() = 0;
        virtual void paint(QPainter &p) = 0;
        virtual void resize(const QSize &s) = 0;

    protected:
        View    *m_view;
};

#endif
