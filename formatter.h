/**
 * Text Formatter.
 * It will reformatting paragraph with UI information ( Font, Width, Height )
 * vim:sw=2:et
 */
#ifndef __FORMATTER_H__
#define __FORMATTER_H__

#include "para.h"
#include <qmap.h>

class   TParser;
class   QPainter;
class   QFontMetrics;
class   QFont;
class   QColor;
struct  FLine;

class Formatter
{
  private:
    int             m_width, m_height, m_lmargin, m_tabwidth, m_iheight;

    bool            m_fakebold;

    struct FontM {
      QFont         *f;
      QFontMetrics  *m;
      bool          b;
    } m_metrics, m_bmetrics;

    int         m_pstart, m_poffset, m_pend;
    TParser     *m_ps;
    QColor      *m_colors;

    QMap<int,Paragraph> m_cache;

    FLine       *m_head, *m_ptr, *m_last;

  private:
    FLine       *getFLines(int idx);

  public:
    Formatter();
    ~Formatter();

    void updateLines();
    void cleanLines();
    void setParser(TParser *ps);
    void setLineMargin(int margin);
    void setSize(int width, int height);
    void setFont(QFont f, bool isBold, bool can_fake_bold);
    void setColors(QColor *colors);
    void setParagraph(int paragraph, int idx);
    void getParagraph(int &para, int &offset);
    void getLastParagraph(int &para, int &offset);
    void drawLines(QPainter *dc,
          int fx, int fy, int dx, int dy, int dw, int dh);

    bool nextPage();
    bool prevPage();
    bool nextLine(int cnt=1, int *move=NULL, int *valid_height=NULL);
};

#endif
