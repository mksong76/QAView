/**
 * Text Parser.
 * @file parser.h
 * @author MoonKyu Song (lanterrt@hotmail.com)
 * vim:sw=2:sws=2:et:ai:smartindent
 */
#ifndef __TPARSER_H__
#define __TPARSER_H__

#include "para.h"

struct LineInfo {
  struct LineInfo *next;
  unsigned long   cnt;
  unsigned long   offsets[1];
};

#define SK_KEYWORD  0x01
#define SK_REGULAR  0x02

struct SearchResult {
  int   para;
  short offset, length;

  SearchResult(int p=0, short o=0, short l=0) {
    para = p; offset = o; length = l; };
  SearchResult(const SearchResult &p) {
    para = p.para;
    offset = p.offset;
    length = p.length;
  }

  operator bool() { return length!=0; }
  SearchResult &operator = (const SearchResult &p) {
    para = p.para;
    offset = p.offset;
    length = p.length;
  }
};

class OffsetDB {
    LineInfo      *m_list, *m_last;
    unsigned long **m_ldb;
    int           m_lines;
  public:
    OffsetDB();
    ~OffsetDB();
    void beginAdd();
    void addOffset(unsigned long offset);
    void endAdd(unsigned long offset);
    unsigned long operator[](int para);
    int length(int para);

    int   count();
    int   indexOf(unsigned long offset);
};

class QZFile;

class TParser {
  public:
    virtual Paragraph getParagraph(int idx) = 0;
    virtual unsigned long getOffset(int idx) = 0;
    virtual unsigned long getLength(int idx) = 0;
    virtual int getParaCount() = 0;

    /**
     * search text.
     * @param str key string for search ( single word or some regular.. )
     * @param flags search flags ( it will specify the type of <str> )
     * @param para search beginning paragraph.
     * @param offset search beginning offset.
     */
    virtual SearchResult search(const QString &str, int flags,
                                              int para=0, short offset=0) = 0;

    /**
     * get suitable parser for specified file.
     * @param fd file.
     * @return parser for accessing file.
     */
    static TParser *getParser(QZFile *fd);

    /**
     * Get ANSI parser.
     * @param[in] fd file.
     * @param[in] encoding_id If it is zero, it select one automatically.
     */
    static TParser *getANSIParser(QZFile *fd, int encoding_id);
};

class SimpleParser : public TParser {
  protected:
    OffsetDB    m_offdb;
    virtual Paragraph getParagraphI(unsigned long offset, int length) = 0;

  public:
    SimpleParser();

    /**
     * get specified paragraph.
     * @param idx index for paragraph. ( 0 ~ N-1 )
     * @return paragraph in document.
     */
    Paragraph getParagraph(int idx);
    unsigned long getOffset(int idx);
    unsigned long getLength(int idx);

    int getParaID(unsigned long offset);

    /**
     * get number of paragraphs of this document.
     * @return number of paragraphs.
     */
    int getParaCount();

    /**
     * search keyword in paragraph.
     * @param str keyword for searching.
     * @param after paragraph index for beginning of search. ( inclusive )
     * @return -1 on failure. otherwise it will return paragraph index.
     */
    virtual SearchResult search(const QString &str, int flags=0,
                                                  int para=0, short offset=0);
};


#endif
