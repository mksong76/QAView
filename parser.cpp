/**
 * Text Parser.
 * vim:sw=2:sts=2:et
 */
#include "parser.h"
#include "para.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "qzfile.h"
#include "filetype.h"

OffsetDB::OffsetDB()
{
  m_ldb = NULL;
  m_list = m_last = NULL;
  m_lines = 0;
}

OffsetDB::~OffsetDB()
{
  LineInfo  *to_del;

  while (m_list) {
    to_del = m_list;
    m_list = m_list->next;

    free(to_del);
  }
  if (m_ldb)
    free(m_ldb);
}

void
OffsetDB::beginAdd()
{
  LineInfo  *to_del;

  while (m_list) {
    to_del = m_list;
    m_list = m_list->next;

    free(to_del);
  }
  if (m_ldb)
    free(m_ldb);
  m_ldb = NULL;
  m_list = NULL;
  m_last = NULL;
  m_lines = 0;
}

#define   MAX_OFFSET_COUNT  100

void
OffsetDB::addOffset(unsigned long offset)
{
  if (m_last==NULL || m_last->cnt==MAX_OFFSET_COUNT) {
    LineInfo *n_line;
    n_line = (LineInfo*)malloc(sizeof(LineInfo)+
        sizeof(unsigned long)*(MAX_OFFSET_COUNT-1));
    n_line->cnt = 0;
    n_line->next = NULL;

    if (m_last) {
      m_last->next = n_line;
    } else {
      m_list = n_line;
    }
    m_last = n_line;
  }
  m_last->offsets[m_last->cnt++] = offset;
  m_lines++;
}

void
OffsetDB::endAdd(unsigned long offset)
{
  addOffset(offset);

  int db_count;
  db_count = (m_lines+MAX_OFFSET_COUNT-1)/MAX_OFFSET_COUNT;
  m_ldb = (unsigned long **)malloc(sizeof(unsigned long*)*db_count);

  LineInfo  *ptr = m_list;
  db_count = 0;
  while (ptr) {
    m_ldb[db_count++] = ptr->offsets;
    ptr = ptr->next;
  }

  m_lines--;
}

unsigned long
OffsetDB::operator[] (int para)
{
  if (m_lines>para && para>=0) {
    return m_ldb[para/MAX_OFFSET_COUNT][para%MAX_OFFSET_COUNT];
  }
  if (m_lines>0)
    return m_ldb[0][0];
  return 0;
}

int
OffsetDB::count()
{
  return m_lines;
}

int
OffsetDB::length(int para)
{
  if (m_lines>para && para>=0) {
    return (int)(m_ldb[(para+1)/MAX_OFFSET_COUNT][(para+1)%MAX_OFFSET_COUNT]
        - m_ldb[para/MAX_OFFSET_COUNT][para%MAX_OFFSET_COUNT]);
  }
  if (m_lines>0)
    return (int)(m_ldb[0][1]-m_ldb[0][0]);
  return 0;
}

int
OffsetDB::indexOf(unsigned long offset)
{
  LineInfo  *ptr = m_list;
  int       idx = 0, idx_base = 0;
  while (ptr) {
    if (ptr->offsets[0]<=offset &&
        ptr->offsets[ptr->cnt-1]>=offset) {
      for (idx=0 ; idx<ptr->cnt && ptr->offsets[idx]<offset ;idx++);
      if (idx>0 && (idx>=ptr->cnt || ptr->offsets[idx]!=offset)) idx--;
      return idx_base+idx;
    } else {
      idx_base += ptr->cnt;
      ptr = ptr->next;
    }
  }
  return m_lines-1;
}

SimpleParser::SimpleParser()
{
}

Paragraph
SimpleParser::getParagraph(int idx)
{
  //printf("GetParagraphI(%d,%d)\n", m_offdb[idx], m_offdb.length(idx));
  return getParagraphI(m_offdb[idx], m_offdb.length(idx));
}

unsigned long
SimpleParser::getOffset(int idx)
{
  return m_offdb[idx];
}

unsigned long
SimpleParser::getLength(int idx)
{
  return m_offdb.length(idx);
}

int
SimpleParser::getParaID(unsigned long offset)
{
  return m_offdb.indexOf(offset);
}

int
SimpleParser::getParaCount()
{
  return m_offdb.count();
}

/**
 * search keyword in paragraph.
 */
SearchResult
SimpleParser::search(const QString &str, int flags, int para, short offset)
{
  return SearchResult();
}

#include "hcode.h"
#include "ansiparser.h"

/**
 * Detecting format of file.
 */
TParser *
TParser::getParser(QZFile *fd)
{
  QString       f_name;

  /* file name check */
  /*
  f_name = fd->name();
  if (f_name.right(3)==".dz") {
    return new DictParser(fd);
  }
  */

  return getANSIParser(fd, FE_AUTO);
}

TParser *
TParser::getANSIParser(QZFile *fd, int encoding_id)
{
  const char    *encoding = "euc-kr";

  switch (encoding_id) {
    default:
    case FE_AUTO:
      {
        int   ks_count, kssm_count, eng_count;
        char  buffer[2048];
        int   readed;

        readed = fd->read(buffer, sizeof(buffer));
        isKS(buffer, readed, &ks_count, &kssm_count, &eng_count);
        if ((ks_count+kssm_count)>=eng_count) {
          if (kssm_count>ks_count)
            encoding = "johab";
          else
            encoding = "euc-kr";
        }
      }
      break;
    case FE_EUC_KR:
      encoding = "euc-kr";
      break;
    case FE_JOHAB:
      encoding = "johab";
      break;
    case FE_UTF8:
      encoding = "utf8";
      break;
  }
  //printf("ENCODING:%s\n", encoding);
  return new AnsiParser(fd, encoding);
}


