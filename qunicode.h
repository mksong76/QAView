#ifndef __QUNICODE_H__
#define __QUNICODE_H__

#include <qstring.h>
#include <qcstring.h>

int toMB(QCString &buf, QString st, const char *enc=NULL);
int toWC(QString &buf, const char *st, int st_len=-1, const char *enc=NULL);
int toWC(QString &buf, const char *st, const char *enc=NULL);
QString  toWC(const char *st, const char *enc=NULL);
QString  toWC(const char *st, int length=-1, const char *enc=NULL);

#endif
