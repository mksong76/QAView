#include "unicode.h"
#include <string.h>

/*
   Converting function prototype.
 */
typedef int MB2WC(unsigned int hdl,
      wchar_t *buf, const char *str, int length);
typedef int WC2MB(unsigned int hdl,
      char *buf, wchar_t ch, int length);

typedef struct {
  const char            *name;
  const char            *lname;
  const unsigned int    flag;
  MB2WC                 *mbtowc;
  WC2MB                 *wctomb;
} UnicodeCodec;

typedef unsigned int    conv_t;
typedef wchar_t         ucs4_t;
typedef struct {
  unsigned short indx; /* index into big table */
  unsigned short used; /* bitmask of used entries */
} Summary16;
#define RET_ILSEQ       -1
#define RET_ILUNI       -2
#define RET_TOOSMALL    -3
#define RET_TOOFEW(x)   -4
#define abort()         return -1

#include "ascii.h"
#include "ksc5601.h"
#include "johab_hangul.h"
#include "johab.h"
#include "cp949.h"
#include "utf8.h"

static UnicodeCodec m_convs[] = {
    { "euc-kr", "EUC-KR", 0, (MB2WC*)&cp949_mbtowc, (WC2MB*)&cp949_wctomb },
    { "cp949", "CP949", 0, (MB2WC*)&cp949_mbtowc, (WC2MB*)&cp949_wctomb },
    { "johab", "JOHAB", 0, (MB2WC*)&johab_mbtowc, NULL },
    { "utf8", "UTF8", 0, (MB2WC*)&utf8_mbtowc, (WC2MB*)&utf8_wctomb },
    { NULL, NULL, 0, NULL, NULL }
};

int Unicode::EUCKR = 0;
int Unicode::UTF8 = 3;
int Unicode::DEFAULT = Unicode::EUCKR;

static int m_length = sizeof(m_convs)/sizeof(UnicodeCodec)-1;
static int m_default = 0;
static CodecInfo *m_codec_info = NULL;

bool
Unicode::setDefaultEncoding(const char *name)
{
  int   idx = getEncoding(name, NEED_ALL);
  if (idx<0) return false;

  DEFAULT = idx;
  return true;
}

int
Unicode::getEncoding(const char *name, int need)
{
  if (name==NULL) {
    return Unicode::DEFAULT;
  }

  int   idx;
  for (idx=0; idx<m_length ; idx++) {
    if (strcmp(name, m_convs[idx].name)==0) {
      if ((need&NEED_MB_TO_WC) && m_convs[idx].mbtowc==NULL)
        return -1;
      if ((need&NEED_WC_TO_MB) && m_convs[idx].wctomb==NULL)
        return -1;
      return idx;
    }
  }
  return -1;
}

/**
 * get all codec information.
 */
const CodecInfo *
Unicode::getAllEncodings()
{
  if (m_codec_info)
    return m_codec_info;

  /* malloc memory for codec */
  m_codec_info = new CodecInfo[m_length+1];

  /* store codec info */
  int idx;
  for (idx=0 ; idx<m_length ; idx++) {
    m_codec_info[idx].key = m_convs[idx].name;
    m_codec_info[idx].name = m_convs[idx].lname;
    m_codec_info[idx].support = \
        (m_convs[idx].mbtowc?NEED_MB_TO_WC:0)||
        (m_convs[idx].wctomb?NEED_WC_TO_MB:0);
    m_codec_info[idx].enc = idx;
  }

  /* mark last */
  memset(&m_codec_info[idx], 0, sizeof(CodecInfo));
  m_codec_info[idx].enc = -1;

  return m_codec_info;
}

int
Unicode::mbtowc(wchar_t *buf, const char *str, int length, int enc)
{
  if (enc==-1) enc = m_default;
  if (enc<m_length && enc>=0 && m_convs[enc].mbtowc)
    return m_convs[enc].mbtowc(m_convs[enc].flag, buf, str, length);
  return -1;
}

int
Unicode::mbtowcStr(wchar_t *buf, const char *str, int length, int enc)
{
  if (enc==-1) enc = m_default;
  if (enc<m_length && enc>=0 && m_convs[enc].mbtowc) {
    MB2WC *func = m_convs[enc].mbtowc;
    unsigned int flag = m_convs[enc].flag;
    int wc_count=0, mb_count=0, cnt;

    if (buf) {
      while (mb_count<length) {
        cnt = func(flag, buf+wc_count, str+mb_count, length-mb_count);
        if (cnt<0) {
          mb_count++;
        } else {
          wc_count++;
          mb_count+=cnt;
        }
      }
    } else {
      wchar_t   buff[2];
      while (mb_count<length) {
        cnt = func(flag, buff, str+mb_count, length-mb_count);
        if (cnt<0) {
          mb_count++;
        } else {
          wc_count++;
          mb_count+=cnt;
        }
      }
    }
    return wc_count;
  }
  return -1;
}

int
Unicode::wctomb(char *buf, wchar_t str, int length, int enc)
{
  if (enc==-1) enc = m_default;
  if (enc<m_length && enc>=0 && m_convs[enc].wctomb)
    return m_convs[enc].wctomb(m_convs[enc].flag, buf, str, length);
  return -1;
}


int
Unicode::wctombStr(char *buf, const wchar_t *str, int length, int enc)
{
  if (enc==-1) enc = m_default;
  if (enc<m_length && enc>=0 && m_convs[enc].mbtowc) {
    WC2MB *func = m_convs[enc].wctomb;
    unsigned int flag = m_convs[enc].flag;
    int wc_count=0, mb_count=0, cnt;

    if (buf) {
      while (wc_count<length) {
        cnt = func(flag, buf+mb_count, str[wc_count], 4);
        if (cnt<0) {
          mb_count++;
        } else {
          wc_count++;
          mb_count+=cnt;
        }
      }
    } else {
      char   buff[4];
      while (wc_count<length) {
        cnt = func(flag, buff, str[wc_count], 4);
        if (cnt<0) {
          mb_count++;
        } else {
          wc_count++;
          mb_count+=cnt;
        }
      }
    }
    return mb_count;
  }
  return -1;
}
