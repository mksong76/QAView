/**
 * ANSI text parser.
 * vim:ts=8:sw=2
 */
#ifndef __ANSI_PARSER_H__
#define __ANSI_PARSER_H__

#include "parser.h"

class QZFile;

class AnsiParser : public SimpleParser
{
    QZFile  *m_fd;
    char    *m_enc;
  protected:
    virtual Paragraph getParagraphI(unsigned long offset, int length);

  public:
    ~AnsiParser();
    AnsiParser(QZFile *fd, const char *enc=NULL);
    bool setEncoding(const char *enc);
};

#endif
