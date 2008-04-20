#ifndef __USER_EVENT_H__
#define __USER_EVENT_H__

#include <qevent.h>
#include <qpixmap.h>

enum {
    QAVIEW_IMAGE_READY      = QEvent::User
};

class UEImageReady : public QEvent
{
    public:
        UEImageReady(const QString &file, const QPixmap &px,
                const QSize &splits) :
            QEvent((QEvent::Type)QAVIEW_IMAGE_READY) {
            m_filename = file;
            m_image = px;
            m_splits = splits;
        }
        QString file() { return m_filename; }
        QPixmap image() { return m_image; }
        QSize   splits() { return m_splits; }
    private:
        QString m_filename;
        QPixmap m_image;
        QSize   m_splits;
};

#endif
