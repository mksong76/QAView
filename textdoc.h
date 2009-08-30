#ifndef __TEXT_DOCUMENT_H__
#define __TEXT_DOCUMENT_H__

#include <qfont.h>
#include <qpainter.h>
#include "formatter.h"
#include "document.h"

class View;
class TParser;

class TextDocument : public Document
{
    public:
        TextDocument(View *p);

        void loadConfig();
        void setParagraph(int p, int o=0);
        int getParaCount();
        bool hasDocument();
        bool nextPage();
        bool prevPage();
        bool setDocument(QString filename, int parser_id,
                int encoding_id, int para, int offset);
        bool getDocumentLocation(int &para, int &offset);
        bool getProgress(unsigned long &offset, unsigned long &all);
        void resetDocument();
        void setParserAndLocation(TParser *ps, int para, int offset);
    protected:
        void paint(QPainter &p);
        void resize(const QSize &s);

    private:
        // settings.
        QFont   m_font;
        QColor  m_colors[9];

        QSize   m_vsize;

        Formatter       m_fmt;
        TParser         *m_parser;
        unsigned long   m_totalLength;
};

#endif
