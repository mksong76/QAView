/**
 * Text viewer.
 * This will show text in QAview component.
 * vim:sw=2:et
 */
#include "textview.h"
#include <qpainter.h>
#include <qrect.h>
#include <qfont.h>
#include "qunicode.h"
#include "parser.h"
#include "formatter.h"
#include "aview.h"
#include "aconfig.h"
#include "qzfile.h"
#include <stdio.h>
#include <qwmatrix.h>
#include <qlineedit.h>
#include <qpe/power.h>


TextView::TextView(QWidget *parent, const char *name, WFlags f) :
    BaseView(parent, name, f)
{
  m_font = QFont("dinaru", 16, QFont::Normal, FALSE);
  m_fmt.setFont(m_font, false, false);
  m_parser = NULL;

  setBackgroundColor(QColor(0,0,0));
  m_colors[8] = QColor(0,0,0);
  m_colors[0] = QColor(0xff,0xff,0xff);

  m_margin = 4;

  m_pb = 10;
  m_pbHeight = 10;
  m_pbFont = QFont("helvetica", 8, QFont::Normal, FALSE);
  m_pbColor1.setRgb(0x22,0x55, 0x22);
  m_pbColor2.setRgb(0x22,0x22, 0x22);
  m_pbText.setRgb(0xff,0xff,0xff);

  m_rotation = 0;
  m_rebuild = true;
  m_pWidth = 0;
  m_pHeight = 0;

  m_scrollHeight = 1;
  m_scrollDelay = 3000;

  connect(&m_timer, SIGNAL(timeout()), this, SLOT(nextStep()));

  loadConfig();
}

#include <qdatetime.h>
#include <time.h>

/**
 * draw progress bar.
 * @param area [in] area for clock.
 * @param dc [in] drawing handle.
 */
void
TextView::drawClock(QRect &area, QPainter &dc)
{
  QString   to_show;
  QTime     tm;
  QDateTime tm2;

  tm = QTime::currentTime();
  to_show.sprintf("%2d:%02d", tm.hour(), tm.minute());

  dc.fillRect(area, this->backgroundColor());
  dc.setFont(m_pbFont);
  dc.setPen(m_pbText);
  dc.drawText(area, AlignCenter, to_show);
}

#include "battery.h"


/**
 * draw battery bar.
 * @param area [in] area for battery.
 * @param dc [in] drawing handle.
 */
void
TextView::drawBattery(QRect &area, QPainter &dc)
{
  QRect             bat_area;
  static MyBattery  power;
  //PowerStatus   power;

  dc.fillRect(area, this->backgroundColor());
  dc.setPen(m_pbText);
  dc.drawRect(area);

  bat_area = area;
  bat_area.rLeft() += 2;
  bat_area.rTop() += 2;
  bat_area.rBottom() -= 2;
  bat_area.rRight() -= 2;

  /*
  power = power_manager.readStatus();
  */
  power.updateStatus();

  if (power.acStatus() == PowerStatus::Online) {
    dc.drawText(area, AlignCenter, "A C");
    return;
  }

  int   all_width = bat_area.width()-1;
  all_width = all_width*power.batteryPercentRemaining()/100;

  /*
  printf("Status:%d Accu:%d Remain:%d RemainTime:%d\n",
      power.batteryStatus(),
      power.batteryPercentAccurate(),
      power.batteryPercentRemaining(), power.batteryTimeRemaining() );
      */

  bat_area.rRight() = bat_area.rLeft()+all_width;
  dc.fillRect(bat_area, this->m_pbText);
}

/**
 * draw progress bar.
 * @param area [in] area for scroll bar.
 * @param dc [in] drawing handle.
 */
void
TextView::drawProgressBar(QRect &area, QPainter &dc)
{
  unsigned long   m_offset;
  int     m_width, x_para, x_offset;
  QRect   p_rect;

  m_fmt.getLastParagraph(x_para, x_offset);
  m_offset = m_parser->getOffset(x_para);
  Paragraph para = m_parser->getParagraph(x_para);
  if (x_offset>0 && para.m_str->length()>0)
    m_offset += x_offset*m_parser->getLength(x_para)/para.m_str->length();

  QString to_show;
  to_show.sprintf("%d  /  %d", m_offset, m_pbLength);

  p_rect = area;

  if (m_pbLength>0)
    p_rect.rRight() = p_rect.left()+p_rect.width()*m_offset/m_pbLength;
  else
    p_rect.rRight() = p_rect.left()+p_rect.width();
  dc.fillRect(p_rect, m_pbColor1);
  p_rect.rLeft() = p_rect.right();
  p_rect.rRight() = area.right();
  dc.fillRect(p_rect, m_pbColor2);

  p_rect.rLeft() = area.left();
  p_rect.rRight() = area.right();
  dc.setFont(m_pbFont);
  dc.setPen(m_pbText);
  dc.drawText(p_rect, AlignCenter, to_show);
}

void
TextView::paintEvent(QPaintEvent *ev)
{
  QRect     urect = ev->rect();
  QRect     mrect = this->rect();
  int       dx, dy;

  /*
  printf("TextView::paintEvent(%d,%d) (%dx%d)\n",
      urect.left(), urect.top(),
      urect.width(), urect.height());
      */

  if (!m_parser) {
    QPainter  dc(this);

    dc.setPen(m_fg);
    dc.setFont(QFont("fixed", 16));

    dc.eraseRect(urect);
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
    return;
  }

  QPainter  dc;

  if (m_rotation==0 || m_rebuild) {
    if (m_rotation!=0) {
      mrect = QRect(0, 0, m_pWidth, m_pHeight);
      urect = mrect;
      m_orgBuffer.resize(mrect.size());
      dc.begin(&m_orgBuffer);
    } else {
      dc.begin(this);
    }
    dc.setBackgroundColor(this->backgroundColor());
    dc.setPen(m_fg);

    /* erase area which text draw.. */
    if (urect.bottom()>mrect.bottom()-m_pb)
      dc.eraseRect(urect.left(), urect.top(),
          urect.width(), (mrect.bottom()-m_pb)-urect.top()+1);
    else
      dc.eraseRect(urect);


    dx = urect.left()-mrect.left()-m_margin;
    dy = urect.top()-mrect.top()-m_margin;

    dc.setClipRect(urect);
    m_fmt.drawLines(&dc, dx, dy, urect.left(), urect.top(),
        urect.width(), urect.height());

    dc.setClipRect(urect);

    /* draw progress bar */
    if (m_parser && m_pb>0 && urect.bottom()>mrect.bottom()-m_pb) {
      QRect   p_rect;

      p_rect = mrect;
      p_rect.rTop() = p_rect.rBottom()-m_pb+1;
      p_rect.rLeft() += 30;
      p_rect.rRight() -= 30;
      drawProgressBar(p_rect, dc);

      p_rect = mrect;
      p_rect.rTop() = p_rect.rBottom()-m_pb+1;
      p_rect.rLeft() = p_rect.rRight() - 30;
      drawClock(p_rect, dc);

      p_rect = mrect;
      p_rect.rTop() = p_rect.rBottom()-m_pb+1;
      p_rect.rRight() = p_rect.rLeft() + 30;
      drawBattery(p_rect, dc);
    }
    dc.end();
  }

  if (m_rotation!=0) {
    if (m_rebuild) {
      m_rotBuffer = m_orgBuffer.xForm(m_matrix);
      m_rebuild = false;
    }
    urect = ev->rect();
    bitBlt(this, urect.left(), urect.top(), &m_rotBuffer,
        urect.left(), urect.top(), urect.width(), urect.height());
  }
}

/**
 * Update formatter size.
 * It will change size of formatter area according to size of this window
 * and rotation value.
 */
void
TextView::updateSize()
{
  int       n_width, n_height;
  if (m_rotation==90 || m_rotation==270) {
    n_width = height();
    n_height = width();
  } else {
    n_width = width();
    n_height = height();
  }
  if (n_width==m_pWidth && n_height==m_pHeight) return;
  m_pWidth = n_width;
  m_pHeight = n_height;

  m_fmt.setSize(m_pWidth-m_margin*2, m_pHeight-m_pb-m_margin*2);
  m_rebuild = true;
}

/**
 * Change rotation value.
 * @param degree new rotation value. multiple of 90.
 */
void
TextView::setRotation(int degree)
{
  while (degree<0) degree+= 360;
  while (degree>=360) degree-= 360;

  //printf("TextView::rotate(%d)\n", degree);

  switch (degree) {
    case 0:
      m_rotation = 0;
      m_orgBuffer.resize(0,0);
      m_rotBuffer.resize(0,0);
      m_rebuild = false;
      updateSize();
      m_fmt.updateLines();
      repaint(false);
      break;
    case 90: case 180: case 270:
      m_rotation = degree;
      m_matrix.reset();
      m_matrix.rotate(degree);
      m_rebuild = true;
      updateSize();
      m_fmt.updateLines();
      repaint(false);
      break;
  }
  /* store rotation value. */
  g_cfg->getInt(ACFG_Rotation) = m_rotation;
  g_cfg->write(ACFG_Rotation);
}

void
TextView::rotateRight()
{
  setRotation(m_rotation+90);
}

void
TextView::rotateLeft()
{
  setRotation(m_rotation-90);
}

void
TextView::resizeEvent(QResizeEvent *ev)
{
  //printf("TextView::resizeEvent(%d,%d)\n", width(), height());
  updateSize();
  m_fmt.updateLines();
  if (ev->oldSize().width()!=width())
    repaint(false);
}

/**
 * set scroll mode.
 * @param mode [in] 0:turn off, 1:turn on, 2:toggle.
 */
void
TextView::setSlideShowMode(int mode)
{
  bool new_mode;
  switch (mode) {
    case 0:
    case 1:
      new_mode = mode;
      break;
    case 2:
      new_mode = !m_timer.isActive();
      break;
    default:
      return;
  }

  if (new_mode!=m_timer.isActive()) {
    if (new_mode) {
      m_timer.start(m_scrollDelay);
    } else {
      m_timer.stop();
    }
  }
}

void
TextView::nextPage()
{
  if (m_fmt.nextPage()) {
    m_rebuild = true;
    repaint(false);
  } else {
    emit loadNext();
  }
}

void
TextView::nextStep()
{
  int   move, valid_height;
  if (m_fmt.nextLine(m_scrollHeight, &move, &valid_height)) {
    int       xx = m_margin, yy = m_margin;
    int       ww = m_pWidth-m_margin*2, hh = m_pHeight-m_margin*2;

    if (m_rotation!=0) {
      m_rebuild = true;
      repaint(false);
    } else {
      bitBlt(this, xx, yy, this, xx, yy+move, ww, valid_height);
      repaint(0, yy+valid_height,
          m_pWidth, m_pHeight-yy-valid_height,
          false);
    }
  } else
    setSlideShowMode(0);
}

void
TextView::prevPage()
{
  if (m_fmt.prevPage()) {
    m_rebuild = true;
    repaint(false);
  } else {
    printf("load previous file\n");
    emit loadPrev();
  }
}

void
TextView::cleanLineDB()
{
  m_fmt.cleanLines();
}

void
TextView::setParagraph(int para)
{
  this->setParagraph(para, 0);
}

void
TextView::search(QString key, int flag)
{
  if (m_parser) {
    SearchResult sr;
    sr = m_parser->search(key, flag);
    if (sr) {
      setParagraph(sr.para);
    }
  }
}

void
TextView::setParagraph(int para, int offset)
{
  if (m_parser) {
    m_rebuild = true;
    m_fmt.setParagraph(para, offset);
    m_fmt.updateLines();
    repaint(false);
  }
}

int
TextView::getParaCount()
{
  if (m_parser) {
    return m_parser->getParaCount();
  } else {
    return 0;
  }
}

void
TextView::getParagraph(int &para, int &offset)
{
  if (m_parser) {
    m_fmt.getParagraph(para, offset);
    return;
  }
  para = 0;
  offset = 0;
  return;
}

/**
 * Check wether it is showing document.
 * @return true if it has document.
 */
bool
TextView::hasDocument()
{
  return NULL!=m_parser;
}

/**
 * Try to change document.
 */
bool
TextView::setDocument(QString filename,
    int parser_id, int encoding_id, int para, int offset)
{
  TParser   *n_ps;
  QZFile    *n_fd;
  n_fd = new QZFile(filename);
  if (!n_fd->exists()) {
    printf("TextView::setDocument(%s) NOT EXIST\n",
        (const char*)filename.utf8());
    delete n_fd;
    return false;
  }
  if (!n_fd->open()){
    printf("TextView::setDocument(%s) FAILS to open\n",
        (const char*)filename.utf8());
    delete n_fd;
    return false;
  }

  switch (parser_id) {
    case 0:
    default:
      n_ps = TParser::getParser(n_fd);
      break;
    case 1:
      n_ps = TParser::getANSIParser(n_fd, encoding_id);
      break;
  }
  if (NULL==n_ps) {
    printf("TextView::setDocument(%s) FAILS to get Parser\n",
        (const char*)filename.utf8());
    delete n_fd;
    return false;
  }
  setParserAndLocation(n_ps, para, offset);
  return true;
}

void
TextView::resetDocument()
{
  setParserAndLocation(NULL, 0, 0);
  m_orgBuffer.resize(1,1);
  m_rotBuffer.resize(1,1);
  m_rebuild = true;
}

/**
 * Get current document location.
 * @param[out] filename Current file name.
 * @param[out] para Paragraph number.
 * @param[out] offset File offset.
 * @return It shall return true if it has document and set other parameters.
 *      Otherwise it shall return false and other parameters does not be
 *      changed.
 */
bool
TextView::getDocumentLocation(int &para, int &offset)
{
  if (m_parser) {
    m_fmt.getParagraph(para, offset);
    return true;
  }
  return false;
}

void
TextView::setParserAndLocation(TParser *ps, int para, int offset)
{
  if (m_parser) delete m_parser;
  m_parser = ps;
  m_fmt.setParser(m_parser);
  if (ps) {
    printf("TextView::setParserAndLocation(%d,%d)\n", para, offset);
    if (para>=ps->getParaCount()) {
      para = ps->getParaCount()-1;
      offset = 0;
    }
    m_pbLength = ps->getOffset(ps->getParaCount()-1)+
                      ps->getLength(ps->getParaCount()-1);

    printf("TextView::setParserAndLocation:m_fmt.setParagraph(%d,%d)\n",
        para, offset);
    m_fmt.setParagraph(para, offset);
    m_fmt.updateLines();
  } else {
    m_fmt.cleanLines();
  }
  m_rebuild = true;
  repaint();
}

static int color_key[] = {
  ACFG_TextColor,
  ACFG_HighLight1,
  ACFG_HighLight2,
  ACFG_HighLight3,
  ACFG_HighLight4,
  ACFG_HighLight5,
  ACFG_HighLight6,
  ACFG_HighLight7,
  ACFG_Background
};

void
TextView::loadConfig()
{
  //printf("TextView::loadConfig\n");
  m_font = QFont(
      g_cfg->getString(ACFG_FontName),
      g_cfg->getInt(ACFG_FontSize),
      g_cfg->getBool(ACFG_FontBold) ? QFont::Bold : QFont::Normal, FALSE);

  m_fmt.setFont(m_font, g_cfg->getBool(ACFG_FontBold),
      g_cfg->getBool(ACFG_FakeBold));
  m_fmt.setLineMargin(g_cfg->getInt(ACFG_LineMargin));
  m_margin = g_cfg->getInt(ACFG_Margin);

  updateSize();
  m_fmt.updateLines();

  setBackgroundColor(g_cfg->getColor(ACFG_Background));
  m_fg = g_cfg->getColor(ACFG_TextColor);
  for (int idx=0 ; idx<9 ; idx++)
    m_colors[idx] = g_cfg->getColor(color_key[idx]);
  m_fmt.setColors(m_colors);

  m_pbColor1 = g_cfg->getColor(ACFG_BarFG);
  m_pbColor2 = g_cfg->getColor(ACFG_BarBG);
  m_pbText = g_cfg->getColor(ACFG_BarText);

  m_pbHeight = g_cfg->getInt(ACFG_BarHeight);
  m_pb = g_cfg->getBool(ACFG_BarVisible) ? m_pbHeight : 0;

  setRotation(g_cfg->getInt(ACFG_Rotation));

  m_scrollHeight = g_cfg->getInt(ACFG_ScrollHeight);
  m_scrollDelay = g_cfg->getInt(ACFG_ScrollDelay);

  repaint(false);
}

bool
TextView::canSearch()
{
  return true;
}

