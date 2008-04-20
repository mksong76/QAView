#ifndef __TEXT_VIEW_H__
#define __TEXT_VIEW_H__
/**
 * Text viewer class.
 * vim:ts=8:sw=2
 */

#include <qwidget.h>
#include "formatter.h"
#include "timestamp.h"
#include "baseview.h"
#include <qwmatrix.h>
#include <qpixmap.h>
#include <qtimer.h>

class   TParser;
class   AskDialog;

/**
 * Text Viewing Class.
 * Widget for text area. It will use Formatter for text draw.
 */
class   TextView : public BaseView
{
  Q_OBJECT
  public slots:
    void nextStep();    /* for autoscroll */

  private:
    void paintEvent(QPaintEvent *ev);
    void resizeEvent(QResizeEvent *ev);

    void cleanLineDB();
    void updateSize();
    void drawProgressBar(QRect &area, QPainter &dc);
    void drawBattery(QRect &area, QPainter &dc);
    void setParserAndLocation(TParser *ps, int para=0, int offset=0);
    void getParagraph(int &para, int &offset);
    void setRotation(int deg);
    void drawClock(QRect &area, QPainter &dc);
    void setParagraph(int paragraph, int idx);

  public:
    void setSlideShowMode(int mode);
    void loadConfig();
    void setParagraph(int para);
    int getParaCount();
    bool getDocumentLocation(int &para, int &offset);
    bool hasDocument();
    bool setDocument(QString filename, int parser_id, int encoding_id,
            int para, int offset);
    void resetDocument();

    bool canSearch();
    void search(QString key, int flags);

    void nextPage();
    void prevPage();
    void rotateRight();
    void rotateLeft();

    TextView(QWidget *parent=NULL, const char *name=NULL, WFlags f=0);
  private:
    Formatter   m_fmt;
    TParser     *m_parser;

    QFont       m_font;
    QColor      m_fg;
    QColor      m_colors[9];

    int         m_margin;

    int         m_pb, m_pbHeight, m_pbLength;
    QFont       m_pbFont;
    QColor      m_pbColor1, m_pbColor2, m_pbText;

    int         m_rotation;
    int         m_pWidth, m_pHeight;
    QWMatrix    m_matrix;
    QPixmap     m_orgBuffer, m_rotBuffer;
    bool        m_rebuild;

    QTimer      m_timer;
    int         m_scrollDelay, m_scrollHeight;

};

#endif
