#ifndef __UNICODE_CONVERTER_H__
#define __UNICODE_CONVERTER_H__

#include <wchar.h>

#define         DEFAULT_ENCODING        (-1)

#define         NEED_MB_TO_WC           1
#define         NEED_WC_TO_MB           2
#define         NEED_ALL                (NEED_MB_TO_WC|NEED_WC_TO_MB)
#define         NEED_ANY                0

typedef struct {
  const char  *key, *name;
  int   enc;
  short support;
} CodecInfo;

class   Unicode {
  private:
    static int DEFAULT;
  public:
    static int EUCKR;
    static int UTF8;
    static int getEncoding(const char *name, int need=(NEED_ALL));
    static const CodecInfo *getAllEncodings();

    static bool setDefaultEncoding(const char *name);

    /**
     * String conversion.
     */
    static int mbtowcStr(wchar_t *buf,
        const char *str, int length=-1, int enc=-1);
    static int wctombStr(char *buf,
        const wchar_t *src, int length=-1, int enc=-1);

    /**
     * on character converting.
     * @return length of multibyte characters which were used for converting.
     *          or output.
     */
    static int mbtowc(wchar_t *buf,
        const char *str, int length=-1, int enc=-1);
    static int wctomb(char *buf,
        wchar_t src, int length=-1, int enc=-1);

    static int detect(const char *buf, int length);
};



#endif
