#include "qunicode.h"
#include "unicode.h"
#include <stdio.h>
#include <stdlib.h>


int
toMB(QCString &buf, QString st, const char *enc_name)
{
  char          buff[5];
  int           idx, result, enc;

  enc = Unicode::getEncoding(enc_name, NEED_WC_TO_MB);
  if (enc<0 ) return -1;

  //printf("SRC:%s\n", (const char*)st);

  buf = "";
  for (idx=0 ; idx<st.length() ; idx++) {
    result = Unicode::wctomb(buff, st.at(idx).unicode(), 4, enc);
    if (result<0) {
      continue;
    } else {
      buff[result] = 0;
      buf.append(buff);
    }
  }
  return buf.length();
}


QString
toWC(const char *st, int st_len, const char *enc_name)
{
  QString st_buf;
  toWC(st_buf, st, st_len, enc_name);
  return st_buf;
}

QString
toWC(const char *st, const char *enc_name)
{
  QString st_buf;
  toWC(st_buf, st, -1, enc_name);
  return st_buf;
}

int
toWC(QString &buf, const char *st, const char *enc_name)
{
  return toWC(buf, st, -1, enc_name);
}

int
toWC(QString &buf, const char *st, int st_len, const char *enc_name)
{
  int   enc;
  enc = Unicode::getEncoding(enc_name, NEED_MB_TO_WC);
  //printf("ENCODING:%s,%d\n", enc_name, enc);
  if (enc<0 ) return -1;

  buf = "";

  const char    *st_src = st;
  int           idx=0, result;
  wchar_t       wch;

  if (st_len<0) st_len = strlen(st);

  while (idx<st_len) {
    result = Unicode::mbtowc(&wch, st_src+idx, st_len-idx, enc);
    if (result<0) {
      idx++;
    } else {
      idx += result;
      buf.append(QChar((ushort)wch));
    }
  }

  return buf.length();
}
