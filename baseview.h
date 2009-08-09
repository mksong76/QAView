#ifndef __BASE_VIEWER_H__
#define __BASE_VIEWER_H__
/**
 * Base viewer class.
 * vim:ts=8:sw=2:smarttab:et
 */

#include <qwidget.h>
/**
 * Slide show modes.
 */
enum {
  SLIDE_SHOW_NONE   = 0,
  SLIDE_SHOW_START  = 1,
  SLIDE_SHOW_TOGGLE = 2
};

class   BaseView : public QWidget
{
  Q_OBJECT
  public :
    BaseView(QWidget *parent=NULL, const char *name=NULL, WFlags f=0);
    virtual void setSlideShowMode(int mode) = 0;
    virtual void loadConfig() = 0;
    virtual void setParagraph(int para) = 0;
    virtual int getParaCount() = 0;
    virtual bool getDocumentLocation(int &para, int &offset) = 0;
    virtual bool hasDocument() = 0;
    virtual bool setDocument(QString filename, int parser_id, int encoding_id,
            int para, int offset) = 0;
    virtual void resetDocument() = 0;

    virtual bool canSearch() = 0;
    virtual void search(QString key, int flags) = 0;

    virtual void nextPage() = 0;
    virtual void prevPage() = 0;
    virtual void rotateRight() = 0;
    virtual void rotateLeft() = 0;
    virtual int getRotation() { return 0; };
  signals:
    void loadNext();
    void loadPrev();
};

#endif
