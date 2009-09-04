#include "textdoc.h"
#include "aconfig.h"
#include "view.h"
#include "qzfile.h"
#include "parser.h"

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

TextDocument::TextDocument(View *p) : Document(p)
{
}

void
TextDocument::loadConfig()
{
    m_font = QFont(g_cfg->getString(ACFG_FontName),
            g_cfg->getInt(ACFG_FontSize),
            g_cfg->getBool(ACFG_FontBold) ? QFont::Bold : QFont::Normal, FALSE);

    for (int idx=0 ; idx<9 ; idx++)
        m_colors[idx] = g_cfg->getColor(color_key[idx]);
    m_fmt.setColors(m_colors);

    m_fmt.setFont(m_font, g_cfg->getBool(ACFG_FontBold),
            g_cfg->getBool(ACFG_FakeBold));
    m_fmt.setLineMargin(g_cfg->getInt(ACFG_LineMargin));
    m_fmt.updateLines();

    m_parser = NULL;
}

void
TextDocument::paint(QPainter &p)
{
    printf("TextDocument::paint() %dx%d\n",
            m_vsize.width(), m_vsize.height());
    m_fmt.drawLines(&p, 0, 0, 0, 0, m_vsize.width(), m_vsize.height());
}

void
TextDocument::resize(const QSize &s)
{
    printf("TextDocument::resize(%d,%d)\n", s.width(), s.height());
    if (m_vsize==s) return;
    m_vsize = s;
    m_fmt.setSize(s.width(), s.height());
    m_fmt.updateLines();
}

bool
TextDocument::nextPage()
{
    return m_fmt.nextPage();
}

bool
TextDocument::prevPage()
{
    return m_fmt.prevPage();
}

void
TextDocument::setParagraph(int p, int o)
{
    if (m_parser) {
        m_fmt.setParagraph(p, o);
        m_fmt.updateLines();
    }
}

int
TextDocument::getParaCount()
{
    if (m_parser) {
        return m_parser->getParaCount();
    } else {
        return 0;
    }
}

bool
TextDocument::hasDocument()
{
    return m_parser!=NULL;
}

void
TextDocument::resetDocument()
{
    setParserAndLocation(NULL, 0, 0);
}

bool
TextDocument::setDocument(QString filename, FILE_TYPE type_id,
        FILE_ENCODING encoding_id, int para, int offset)
{
    TParser   *n_ps;
    QZFile    *n_fd;
    n_fd = new QZFile(filename);
    if (!n_fd->exists()) {
        printf("TextDocument::setDocument(%s) NOT EXIST\n",
                (const char*)filename.utf8());
        delete n_fd;
        return false;
    }
    if (!n_fd->open()){
        printf("TextDocument::setDocument(%s) FAILS to open\n",
                (const char*)filename.utf8());
        delete n_fd;
        return false;
    }

    switch (encoding_id) {
        case FE_EUC_KR:
        case FE_JOHAB:
        case FE_UTF8:
            n_ps = TParser::getANSIParser(n_fd, encoding_id);
            break;
        default:
            n_ps = TParser::getParser(n_fd);
            break;
    }
    if (NULL==n_ps) {
        printf("TextDocument::setDocument(%s) FAILS to get Parser\n",
                (const char*)filename.utf8());
        delete n_fd;
        return false;
    }
    setParserAndLocation(n_ps, para, offset);
    return true;
}

bool
TextDocument::getDocumentLocation(int &para, int &offset)
{
    if (m_parser) {
        m_fmt.getParagraph(para, offset);
        return true;
    }
    return false;
}

void
TextDocument::setParserAndLocation(TParser *ps, int para, int offset)
{
    if (m_parser) delete m_parser;
    m_parser = ps;
    m_fmt.setParser(m_parser);
    if (ps) {
        printf("TextDocument::setParserAndLocation(%d,%d)\n", para, offset);
        if (para>=ps->getParaCount()) {
            para = ps->getParaCount()-1;
            offset = 0;
        }
        m_totalLength = ps->getOffset(ps->getParaCount()-1)+
                          ps->getLength(ps->getParaCount()-1);
        printf("TextDocument::setParserAndLocation:m_fmt.setParagraph(%d,%d)\n",
                para, offset);
        m_fmt.setParagraph(para, offset);
        m_fmt.updateLines();
    } else {
        m_fmt.cleanLines();
    }
}

bool
TextDocument::getProgress(unsigned long &offset, unsigned long &all)
{
    int     lp, lo;
    if (m_parser==NULL) {
        return false;
    }
    all = m_totalLength;
    m_fmt.getLastParagraph(lp, lo);
    offset = m_parser->getOffset(lp)+lo;
    return true;
}
