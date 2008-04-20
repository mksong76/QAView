/**
 * ANSI text parser.
 * vim:ts=8:sw=2
 */
#include "ansiparser.h"
#include "qzfile.h"
#include "unicode.h"
#include <malloc.h>

#define BUFFER_SIZE 1024
static int
makeLineDB(OffsetDB &db, QZFile *fd)
{
  char  buf[BUFFER_SIZE];
  int   idx, readed;
  unsigned long offset;
  bool new_line;

  db.beginAdd();
  new_line = true;
  fd->seek(0,SEEK_SET);
  while(1) {
    offset = fd->tell();
    readed = fd->read(buf, BUFFER_SIZE);
    for (idx=0 ; idx<readed ; idx++) {
      if (new_line) {
        db.addOffset(offset+idx);
        new_line = false;
      }
      if (buf[idx]=='\n')
        new_line = true;
    }
    if (readed<BUFFER_SIZE)
      break;
  }
  db.endAdd(fd->tell());
  return 0;
}

AnsiParser::AnsiParser(QZFile *fd, const char *enc)
{
  m_fd = fd;
  if (enc)
    m_enc = strdup(enc);
  else
    m_enc = NULL;

  makeLineDB(m_offdb, fd);
}

bool
AnsiParser::setEncoding(const char *enc)
{
  if (m_enc)
    free(m_enc);
  if (enc)
    m_enc = strdup(enc);
  else
    m_enc = NULL;
  return true;
}

Paragraph
AnsiParser::getParagraphI(unsigned long offset, int length)
{
  m_fd->seek(offset, SEEK_SET);

  char *buffer;
  buffer = new char[length+1];

  int result = m_fd->read(buffer, length);
  if (result<length)
    goto fail;

  /* remove new line or carrige return */
  while (result>0 && (buffer[result-1]=='\n' || buffer[result-1]=='\r'))
    result--;
  buffer[result] = 0;
  if (result<1) goto fail;

  {
    Paragraph mine(result);
    int     idx, enc, mb_size, w_idx;
    wchar_t wch;

    enc = Unicode::getEncoding(m_enc, NEED_MB_TO_WC);
    if (enc<0) enc = Unicode::getEncoding(NULL);

    w_idx = 0;
    for (idx=0 ; idx<result ;) {
      mb_size = Unicode::mbtowc(&wch, &buffer[idx], result-idx, enc);
      if (mb_size<0) {
        idx++;
        continue;
      }
      idx += mb_size;
      mine.m_str->append(QChar((unsigned short)wch));
      //mine.m_attr[w_idx].attr.col = w_idx%8;
      w_idx++;
    }
    delete buffer;
    return mine;
  }
fail:
  delete buffer;
  return Paragraph();
}

AnsiParser::~AnsiParser()
{
  delete m_fd;
}

