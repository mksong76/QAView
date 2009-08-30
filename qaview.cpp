/**
 * @file subguide.c
 * Include subway guide main function.
 * It will create main widget and initialize database.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/qpeapplication.h>
#include "aview.h"
#include "aconfig.h"


#if     0
#include "qzfile.h"
#include "zfile.h"

int
test_seek(char **argv)
{
    QZFile       *file;
    char        buffer[4096], cmp_buf[4096];
    int         st_idx, st_len, st_ridx, st_rlen;
    int         readed;

    //file = ZFile::open(argv[1], argv[2]);
    file = new QZFile(argv[1]);
    if (!file->open()) {
    //if( !file ) {
        printf("Fail to open zip file!!\n");
        return 1;
    }

    readed = file->read(buffer, sizeof(buffer));
    printf ("READED:%d\n", readed);

    while (1) {
      printf("COMPARE:");
      fflush(stdout);
      scanf("%d%d", &st_idx, &st_len);
      if (st_len<1) {
        break;
      }
      if (st_idx+st_len>readed || st_idx<0) {
        printf("[ERR] Overrange!\n");
        continue;
      }

      file->seek(st_idx, SEEK_SET);
      st_rlen = file->read(cmp_buf, st_len);

      if (st_rlen!=st_len) {
        printf("[ERR] st_rlen(%d) != st_len(%d)\n", st_rlen, st_len);
        continue;
      }

      for (st_ridx=0; st_ridx<st_len &&
          buffer[st_idx+st_ridx]==cmp_buf[st_ridx]; st_ridx++);
      if (st_ridx<st_rlen) {
        printf("[ERR] differ from %d index\n", st_ridx);
        for ( ; st_ridx<st_len ; st_ridx++) {
          printf("[%02x:%02x] ", buffer[st_idx+st_ridx], cmp_buf[st_ridx]);
        }
        printf("\n");
      }
    }
    delete file;
    ZFileSystemGC();
}

#endif

#if     0
#include "qzfile.h"

#endif

#if     0

#include "qunicode.h"

int
test_unicode(const char *name)
{
  QString       str;
  int           result, idx;
  QChar         ch;

  result = toWC(str, QCString("ÇÑ±Û"), "euc-kr");
  if (result<0) {
    printf("ERR on converting!!\n");
    return -1;
  }
  for (idx=0; idx<str.length() ; idx++) {
    ch = str[idx];
    printf("[%04x] ", ch.unicode());
  }
  printf("\n");

  str = "pu";
  result = 2;
  for (idx=0; idx<result ; idx++) {
    ch = str[idx];
    printf("[%04x] ", ch.unicode());
  }
  printf("\n");
  return 0;
}

#endif

#if     0
#include "qzfs.h"
#include <qstring.h>
int
test_getnames(char **argv)
{
  QString org;
  QCString zname, subname;
  
  org = argv[1];
  int result = getNames(org, zname, subname);
  printf("%s -> %d %s / %s\n",
      (const char*)org,
      result,
      (const char*)zname,
      (const char*)subname);
  return 0;
}
#endif

#if   0
#include "qzdir.h"
#define TEST_FUNC(x)  test_dirget(x)
int
test_dirget(char **argv)
{
  QZDir qzdir(argv[1]);
  QStringList lst;
  QString str;
  char buffer[1024];
  bool is_ok;

  while (scanf("%s", buffer)>0) {
    is_ok = qzdir.cd(buffer);
    printf("QZD:cd %s\n", is_ok?"OK":"FAIL");
    printf("PWD:[%s]\n", (const char*)qzdir.absPath());
    lst = qzdir.getList("*", QZFL_ALL, 0);

    int idx;
    for (idx=0 ; idx<lst.count() ; idx++) {
      printf("D:%s\n", (const char *)lst[idx]);
    }
  }

  return 0;
}
#endif

#if   0
#include "ansiparser.h"
#include "qzfile.h"

#define TEST_FUNC(x)  test_ansi(x)
int
test_ansi(char **argv)
{
  QZFile  f(argv[1]);
  if (!f.exists()) {
    printf("No file!! \n");
  }
  f.open();

  AnsiParser  parser(&f, "euc-kr");
  int idx=0, len=0;

  len = parser.getParaCount();
  printf("Paragraph count:%d\n", len);
  Paragraph para;
  for (idx=0 ; idx<len ; idx++) {
    para = parser.getParagraph(idx);
    printf("[%0d] '%s'\n", idx, (const char*)*para.m_str);
  }
  return 0;
}

#endif

/**
 * Main entry function.
 */
int
main(int argc, char **argv)
{
#if 0   /* for testing  */
  return test_seek(argv);
  //return test_print(argv[1]);
  //return test_unicode(NULL);
  //return test_getnames(argv);
  //return test_dirget(argv);
  //return TEST_FUNC(argv);
#else
  QPEApplication   a(argc, argv);

  AViewConfigManager    *mgr = AViewConfigManager::getInstance();


  /* install translator...      */
  QTranslator   trans(NULL);
  QString       lang_path;
  const char    *lang = getenv("LANG");
  if (!lang) lang = "en";
  lang_path.sprintf("/opt/QtPalmtop/i18n/%s", lang);
  if (trans.load("qaview", lang_path)) {
    a.installTranslator(&trans);
  }

  /* creating main window */
  AView w(NULL, "main_window");
  a.setStylusOperation(&w, QPEApplication::RightOnHold);
  a.showMainWidget(&w);

  bool r = a.exec();

  return r;
#endif
}
