#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include <qpixmap.h>
#include <qstring.h>
#include <qsize.h>
#include "content.h"

class View;

class Document : public Content {
    public:
        Document(View *v);

        virtual void setParagraph(int para, int offset=0) = 0;
        virtual int getParaCount() = 0;
        virtual bool getDocumentLocation(int &para, int &offset) = 0;
        virtual bool getProgress(unsigned long &offset, unsigned long &all) = 0;
        virtual bool hasDocument() = 0;
        virtual bool setDocument(QString filename, int parser_id,
                int encoding_id, int para, int offset) = 0;
        virtual void resetDocument() = 0;
        virtual bool canSearch() { return false; };
        virtual void search(QString key, int flags) { };

        // return false if there is no more page.
        virtual bool nextPage() = 0;
        virtual bool prevPage() = 0;
};

#endif
