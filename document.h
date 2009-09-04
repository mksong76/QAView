#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include <qpixmap.h>
#include <qstring.h>
#include <qsize.h>
#include "content.h"
#include "filetype.h"

class View;

class Document : public Content {
    public:
        Document(View *v);

        /**
         * Set paragraph and offset of the data.
         * @param[in] para Paragraph index (0 ~ N-1).
         * @param[in] offset Offset in the paragraph.
         */
        virtual void setParagraph(int para, int offset=0) = 0;

        /**
         * Get number of paragraphs in this document.
         * @return Number of paragraphs.
         */
        virtual int getParaCount() = 0;

        /**
         * Get paragraph and offset of the data.
         * @param[out] para Paragraph index (0 ~ N-1).
         * @param[out] offset Offset in the paragraph.
         * @return @c true If it succeed, otherwise it returns @c false.
         */
        virtual bool getDocumentLocation(int &para, int &offset) = 0;
        virtual bool getProgress(unsigned long &offset, unsigned long &all) = 0;
        virtual bool hasDocument() = 0;
        virtual bool setDocument(QString filename, FILE_TYPE ft,
                FILE_ENCODING fe, int para, int offset) = 0;
        virtual void resetDocument() = 0;
        virtual bool canSearch() { return false; };
        virtual void search(QString key, int flags) { };

        // return false if there is no more page.
        virtual bool nextPage() = 0;
        virtual bool prevPage() = 0;
};

#endif
