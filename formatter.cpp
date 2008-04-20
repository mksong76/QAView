/**
 * Text Formatter.
 * It will reformatting paragraph with UI information ( Font, Width, Height )
 * vim:sw=2:et
 */
#include "formatter.h"
#include "parser.h"
#include <qfontmetrics.h>
#include <stdio.h>
#include <qpainter.h>
#include "qunicode.h"
#include <qfont.h>
#include <stdlib.h>
#include <qmap.h>

struct FLine
{
  int           p_idx, s_idx, e_idx;
  int           width, height, ascent, left;
  Paragraph     para;
  unsigned char *offsets;
  int           lines;

  FLine         *next, *prev;

  FLine();
  ~FLine();

  Link          *getLink(int x, int y);
};

FLine::FLine()
{
  p_idx = 0;
  s_idx = 0;
  e_idx = 0;
  width = 0;
  height = 0;
  ascent = 0;
  left = 0;
  offsets = NULL;
  next = NULL;
  prev = NULL;
  lines = 0;
}

FLine::~FLine()
{
  if (offsets)
    delete offsets;
}

Formatter::Formatter()
{
  m_width = 0;
  m_height = 0;
  m_lmargin = 3;
  m_ps = NULL;
  m_head = m_ptr = m_last = NULL;
  m_pstart = 0;
  m_poffset = 0;

  m_fakebold = true;

  m_metrics.f = new QFont();
  m_metrics.m = new QFontMetrics(*m_metrics.f);
  m_metrics.b = false;

  m_bmetrics.f = new QFont();
  m_bmetrics.m = new QFontMetrics(*m_bmetrics.f);
  m_bmetrics.b = true;

  m_colors = new QColor[9];
}

Formatter::~Formatter()
{
  delete m_colors;
  delete m_bmetrics.f;
  delete m_metrics.f;
  delete m_bmetrics.m;
  delete m_metrics.m;
}

void
Formatter::cleanLines()
{
  FLine       *to_del;

  m_cache.clear();

  while (m_head) {
    to_del = m_head;
    m_head = m_head->next;

    m_cache[to_del->p_idx] = to_del->para;
    delete to_del;
  }
  m_ptr = NULL;
  m_last = NULL;
}

FLine *
Formatter::getFLines(int idx)
{
  FLine *n_head=NULL, *n_one=NULL, *n_last=NULL;
  int   s_idx=0, e_idx=0, len, p_width, ch_width, p_left, p_right, p_offset;
  int   max_ascent, max_descent;
  QChar         wch;
  FontM         *fm;
  bool          enbold;

  //printf("TRY GETFLINE:%d\n", idx);

  Paragraph para;
  if (m_cache.contains(idx))
    para = m_cache[idx];
  else
    para = m_ps->getParagraph(idx);

  len = para.m_str->length();

  /* for empty line!! special case.. It does not need any space computation. */
  if (len<1) {
    n_one = new FLine();
    n_one->para = para;
    n_one->p_idx = idx;
    n_one->height = m_bmetrics.m->ascent()+m_bmetrics.m->descent()+m_fakebold;
    //printf("GETFLINE:%d (1 lines empty)\n", idx);
    return n_one;
  }

  p_offset = 0;
  while (s_idx<len) {

    /* extra left extends for left char!! */
    wch = (*para.m_str)[s_idx];
    /* get font metrics for specified character.. */
    fm = para.m_attr[s_idx].attr.bold ? &m_bmetrics : &m_metrics;
    p_left = -fm->m->leftBearing(wch);
    if (p_left<0) p_left = 0;

    p_width = p_left;
    p_right = 0;
    e_idx = s_idx;

    while (e_idx<len) {
      wch = (*para.m_str)[e_idx];
      fm = para.m_attr[e_idx].attr.bold ? &m_bmetrics : &m_metrics;
      if (wch.unicode()=='\t') {
        ch_width = m_tabwidth - p_width%m_tabwidth;
        p_right = 0;
      } else if (wch.unicode()=='\n') {
        p_right = 0;
        break;
      } else {
        ch_width = fm->m->width(wch);
        p_right = -fm->m->rightBearing(wch)+fm->b;
        if (p_right<0) p_right = 0;
      }
      if (p_width+ch_width+p_right>m_width)
        break;
      p_width += ch_width;
      e_idx++;
    }

    n_one = new FLine();
    n_one->para = para;
    n_one->p_idx = idx;
    n_one->s_idx = s_idx;
    n_one->e_idx = e_idx;
    n_one->height = m_bmetrics.m->ascent()+m_bmetrics.m->descent()+m_fakebold;
    n_one->ascent = m_bmetrics.m->ascent();
    n_one->width = p_right+p_width;
    n_one->left = 0;

    p_offset++;

    if (n_last) {
      n_last->next = n_one;
      n_one->prev = n_last;
      n_last = n_one;
    } else {
      n_head = n_one;
      n_last = n_one;
    }

    /* to next line */
    s_idx = e_idx+(wch.unicode()=='\n');
  }
  //printf("GETFLINE:%d (%d lines)\n", idx, p_offset);
  n_last->lines |= para.m_para->m_lines&0x01;
  n_head->lines |= para.m_para->m_lines&0x02;

  return n_head;
}

void
Formatter::updateLines()
{
  if (m_ps==NULL) return;

#if 0
  printf("WIDTH:%d HEIGHT:%d\nPARAGRAPH:%d,%d\n",
      m_width, m_height, m_pstart, m_poffset);
#endif

  FLine *new_one, *new_last;

  /* if current database does not contain first line, we must clean up
     all lines and recompose .. */
  if (m_last && m_last->p_idx<m_pstart) cleanLines();

  /* prepare first paragraph if there is no line data */
  if (m_head==NULL) {
    new_one = getFLines(m_pstart);
    m_head = new_one;
  }

  /* fill previous lines if the start paragraph is ahead of current head */
  while (m_head->p_idx>m_pstart) {
    new_one = getFLines(m_head->p_idx-1);

    new_last = new_one;
    while (new_last->next) new_last = new_last->next;
    new_last->next = m_head;
    m_head->prev = new_last;

    m_head = new_one;
  }

  /* find first paragraph */
  while (m_head->p_idx<m_pstart) {
    new_one = m_head;
    m_head = m_head->next;
    delete new_one;
  }
  m_head->prev = NULL;

  /* then find the line which m_poffset indicated!! */
  m_ptr = m_head;
  while (m_ptr &&
      m_ptr->s_idx!=m_poffset &&
      m_ptr->e_idx<=m_poffset && m_ptr->p_idx==m_pstart)
    m_ptr = m_ptr->next;
  if (m_ptr==NULL || m_ptr->p_idx!=m_pstart)
    m_ptr = m_head;

  FLine *c_ptr = m_ptr, *p_ptr = NULL;
  int   c_height = 0;

  while (c_height<m_height) {
    /* break on !! */
    if (c_height+c_ptr->height>m_height) break;
    c_height += c_ptr->height + m_lmargin;

    p_ptr = c_ptr;
    c_ptr = c_ptr->next;
    if (c_ptr==NULL) {
      /* end of document!! */
      if (p_ptr->p_idx+1>=m_pend) break;

      new_one = getFLines(p_ptr->p_idx+1);
      p_ptr->next = new_one;
      new_one->prev = p_ptr;
      c_ptr = new_one;
    }
  }
  m_iheight = c_height-m_lmargin;

  m_last = p_ptr;

  /* clear after last paragraph */
  p_ptr = m_last;
  c_ptr = m_last->next;
  while (c_ptr && c_ptr->p_idx==m_last->p_idx){
    p_ptr = c_ptr;
    c_ptr = c_ptr->next;
  }

  /* cleanup last paragraphs */
  if (c_ptr) {
    p_ptr->next = NULL;
    while (c_ptr) {
      p_ptr = c_ptr;
      c_ptr = c_ptr->next;
      delete p_ptr;
    }
  }
  m_cache.clear();
}

void
Formatter::setParser(TParser *ps)
{
  if (ps!=m_ps) {
    cleanLines();
    m_cache.clear();
    m_ps = ps;
    m_pstart = 0;
    m_poffset = 0;
    m_pend = 0;
    if (m_ps) m_pend = m_ps->getParaCount();
    //printf("PARAGRAPHS:%d\n", m_pend);
  }
}

void
Formatter::setLineMargin(int margin)
{
  if (margin<0 || margin>m_height)
    return;
  m_lmargin = margin;
}

#include <qfontdatabase.h>

/**
 * change font.
 */
void
Formatter::setFont(QFont font, bool isBold, bool can_fake_bold)
{
  /* check whether we must use fake bold functionality */
  QFontDatabase fb;
  bool fakebold = isBold && can_fake_bold && (!fb.bold(font.family(), "Bold"));

  if (*m_metrics.f==font && fakebold == m_fakebold) return;

  *m_bmetrics.f = font;
  m_bmetrics.f->setWeight(QFont::Bold);
  *m_metrics.f = font;
  m_fakebold = fakebold;

  *m_metrics.m = *m_metrics.f;
  *m_bmetrics.m = *m_bmetrics.f;

  if (m_fakebold) {
    if (isBold)
      m_metrics.b = true;
    else
      m_metrics.b = false;
    m_bmetrics.b = true;
  } else {
    m_metrics.b = false;
    m_bmetrics.b = false;
  }

  m_tabwidth = m_metrics.m->width('O')*8;

  cleanLines();
}

void
Formatter::setColors(QColor *cols)
{
  for (int idx=0 ; idx<9 ; idx++ )
    m_colors[idx] = cols[idx];
}

void
Formatter::setParagraph(int paragraph, int idx)
{
  if (paragraph<0 || paragraph>=m_pend) {
    return;
  }
  if (abs(m_pstart-paragraph)>(m_height/(m_bmetrics.m->height()+m_lmargin)))
    cleanLines();
  m_pstart = paragraph;
  m_poffset = idx;
}

void
Formatter::getParagraph(int &paragraph, int &offset)
{
  paragraph = m_pstart;
  offset = m_poffset;
}

void
Formatter::getLastParagraph(int &paragraph, int &offset)
{
  if (m_last) {
    paragraph = m_last->p_idx;
    offset = m_last->e_idx;
  } else {
    paragraph = m_pstart;
    offset = m_poffset;
  }
}


void
Formatter::setSize(int width, int height)
{
  if (m_width!=width) {
    cleanLines();
  }
  m_width = width;
  m_height = height;
}

static QColor
getCustomColor(int x)
{
  QString   str;
  str.sprintf("#%03X", x);
  return QColor(str);
}

void
Formatter::drawLines(QPainter *dc,
      int fx, int fy, int dx, int dy, int dw, int dh)
{
  FLine *ptr=m_ptr;
  int   yy = 0;

  while (ptr && yy+ptr->height<=fy) {
    yy += ptr->height+m_lmargin;
    ptr = ptr->next;
  }
  if (ptr==NULL || yy>=m_height) return;

  QString   part;
  unsigned short    c_flag;
  Attr              c_attr;
  int               st_idx, ddx;
  QRect             bound;
  FontM             *fm = &m_metrics;
  QColor            my_color;

  dc->setBackgroundColor(m_colors[8]);

  while (ptr && yy<fy+dh && yy+ptr->height<m_height) {
    //printf("DRAWLINE:%d,%d\n", ptr->p_idx, ptr->s_idx);
    st_idx = ptr->s_idx;
    ddx = ptr->left;
    while (st_idx<ptr->e_idx && ddx<fx+dw) {
      part = "";
      c_attr = ptr->para.m_attr[st_idx];

      if (c_attr.attr.ccol) {
        my_color = getCustomColor(c_attr.attr.col);
      } else {
        my_color = m_colors[c_attr.attr.col];
      }

      fm = c_attr.attr.bold ? &m_bmetrics : &m_metrics;
      dc->setFont(*fm->f);
      dc->setPen(my_color);

      /* tab character processing */
      if (ptr->para.m_str->at(st_idx).unicode()=='\t') {
        ddx += m_tabwidth - (ddx%m_tabwidth);
        st_idx++;
        continue;
      }

      /* get one break */
      while (st_idx<ptr->e_idx && ptr->para.m_attr[st_idx].flag==c_attr.flag) {
        if (ptr->para.m_str->at(st_idx).unicode()=='\t')
          break;
        part.append(ptr->para.m_str->at(st_idx));
        st_idx++;
      }
#if         0
      bound.rLeft()=dx-fx+ddx;
      bound.rTop()=dy-fy+yy;
      bound.rRight()=m_width-ddx+dx-fx;
      bound.rBottom()=bound.rTop()+ptr->height;
      dc->drawText(bound, 0, part);
#else
      //printf("ASCENT:%d\n", ptr->ascent);
      dc->drawText(dx-fx+ddx, dy-fy+yy+ptr->ascent, part);
      if (fm->b) {
        dc->drawText(dx-fx+ddx+1, dy-fy+yy+ptr->ascent+1, part);
        dc->drawText(dx-fx+ddx, dy-fy+yy+ptr->ascent+1, part);
        dc->drawText(dx-fx+ddx+1, dy-fy+yy+ptr->ascent, part);
      }
#endif
      if (ptr->lines&0x01==0 && c_attr.attr.under) {
        dc->drawLine(dx-fx+ddx, dy-fy+yy+ptr->height-1,
                    dx-fx+ddx+fm->m->width(part), dy-fy+yy+ptr->height-1);
      }

      ddx += fm->m->width(part);
    }
    if (ptr->lines&0x01) {
      dc->setPen(m_colors[0]);
      dc->drawLine(
          dx-fx, dy-fy+yy+ptr->height-1,
          dx-fx+m_width, dy-fy+yy+ptr->height-1);
    }
    yy += ptr->height+m_lmargin;
    ptr = ptr->next;
  }
}

bool
Formatter::nextPage()
{
  if (m_last && (m_last->p_idx+1<m_pend || m_last->next)) {
    m_pstart = m_last->p_idx;
    m_poffset = m_last->s_idx;
    //printf("NEW POSITION:%d , %d\n", m_pstart, m_poffset);
    updateLines();
    return true;
  } else
    return false;
}

bool
Formatter::nextLine(int cnt, int *move, int *valid_height)
{
  int   idx = cnt;
  int   diffs = 0, space = 0;
  FLine *ptr = NULL, *new_one, *head;

  head = m_ptr;
  ptr = m_last;
  while (idx>0) {
    if (ptr->next==NULL) {
      if (ptr->p_idx+1<m_pend) {
        new_one = getFLines(ptr->p_idx+1);
        ptr->next = new_one;
        new_one->prev = ptr;
      } else
        break;
    }
    space = ptr->height+m_lmargin;
    ptr = ptr->next;
    idx--;
    while (space>0 && head->next) {
      space -= head->height+m_lmargin;
      diffs += head->height+m_lmargin;
      head = head->next;
    }
  }

  if (idx<cnt) {
    m_pstart = head->p_idx;
    m_poffset = head->s_idx;
    if (move) *move = diffs;
    if (valid_height) *valid_height = m_iheight-diffs;
    updateLines();
    return true;
  } else {
    return false;
  }
}

bool
Formatter::prevPage()
{
  int       p_height = 0;
  FLine     *ptr, *n_one, *n_last;

  if (m_ptr==NULL || (m_ptr->p_idx==0 && m_ptr->prev==NULL)) return false;

  ptr = m_ptr;
  while(p_height+ptr->height<m_height) {
    p_height += m_lmargin+ptr->height;
    m_ptr = ptr;
    if (ptr->prev==NULL) {
      if (ptr->p_idx>0) {
        n_one = getFLines(ptr->p_idx-1);
        n_last = n_one;
        while (n_last->next) n_last = n_last->next;

        n_last->next = m_head;
        m_head->prev = n_last;

        m_head = n_one;
      } else {
        m_ptr = ptr;
        break;
      }
    }
    ptr = ptr->prev;
  }
  m_pstart = m_ptr->p_idx;
  m_poffset = m_ptr->s_idx;
  updateLines();
  return true;
}



