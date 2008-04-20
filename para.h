#ifndef __PARAGRAPH_H__
#define __PARAGRAPH_H__

#include <qstring.h>

typedef union Attr{
  struct {
    unsigned char ccol  :1;
    unsigned short col  :12;
    unsigned char bold  :1;
    unsigned char under :1;
    unsigned char link  :1;
  } attr;
  unsigned short  flag;

  Attr() { this->flag = 0; }
  Attr(const Attr &o) { this->flag = o.flag; }
  bool operator == (const Attr &o) { return this->flag == o.flag; }
  const Attr &operator = (const Attr &o) { this->flag = o.flag ; return *this; }
} Attr;

struct  Link {
  int     sidx, eidx;
  QString str;

  struct Link *next;

  Link(int s, int e, QString qs) { sidx=s; eidx=e; str=qs; next=NULL; }
  Link *findLink(int off);
  ~Link() { delete next; }
};

struct  ParaData
{
  int     m_ref;
  QString m_str;
  Attr    *m_attr;
  int     m_len;
  int     m_ind;
  int     m_lines;

  Link    *m_link;

  ParaData(int len);
  ~ParaData();
};

struct  Paragraph {
    ParaData  *m_para;

    QString   *m_str;
    Attr      *m_attr;

  private:
    void grab();
    void release();

  public:
    Paragraph();
    Paragraph(int length);
    ~Paragraph();
    Paragraph(const Paragraph&p);

    void addLink(int s, int e, QString st);
    Link *findLink(int s);
    bool empty();
    bool operator()();

    Paragraph &operator = (const Paragraph &p);
    //Paragraph &operator = (Paragraph p);
};

#endif
