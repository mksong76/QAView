#include <qtoolbar.h>
#include <qpe/resource.h>
#include <stdio.h>
#include <unistd.h>
#include <qpe/qpeapplication.h>
#include <qlistview.h>
#include <qpe/qpetoolbar.h>
#include <qpe/qpemenubar.h>
#include "aview.h"
#include "qzfile.h"
#include "parser.h"
#include "fileselect.h"
#include "aconfig.h"
#include "optiondlg.h"
#include "askdlg.h"
#include "view.h"
#include "document.h"
#include "textdoc.h"
#include "filelister.h"
#include "filetype.h"

#define     ICON_FILE_CLOSE     "qaview/fileclose"
#define     ICON_FILE_OPEN      "qaview/fileopen"
#define     ICON_OPTIONS        "qaview/options"
#define     ICON_ROTATE_LEFT    "qaview/rotatel"
#define     ICON_ROTATE_RIGHT   "qaview/rotater"

static QPixmap
IconScale(QString pix)
{
    return Resource::loadPixmap(pix);
}

AView::AView(QMainWindow *parent,const char*name, WFlags f)
    : QMainWindow (parent,name,f|WStyle_Customize|WStyle_NoBorder|
            WType_TopLevel|WStyle_NoBorderEx)
{
    setCaption(tr("AView"));

    /* composing toolbar and menu */
    m_tbMain = new QPEToolBar(this);
    m_tbMain->setHorizontalStretchable(true);
    m_tbMenu = new QPEMenuBar(m_tbMain);

    m_mnFile = new QPopupMenu(this);
    m_mnRescent = new QPopupMenu(this);
    m_mnPopup = new QPopupMenu(this);
    m_tbIcon = new QPEToolBar(this);

    m_open = new QAction("Open", IconScale(ICON_FILE_OPEN),
            tr("&Open"), 0, this, 0);
    connect(m_open, SIGNAL(activated()), this, SLOT(openFile()));

    m_close = new QAction("Close", IconScale(ICON_FILE_CLOSE),
            tr("&Close"), 0, this, 0);
    connect(m_close, SIGNAL(activated()), this, SLOT(closeFile()));

    m_option = new QAction("Options", IconScale(ICON_OPTIONS),
            tr("Op&tions"), 0, this, 0);
    connect(m_option, SIGNAL(activated()), this, SLOT(showOptions()));

    m_rotateRight = new QAction("Rotate Right", IconScale(ICON_ROTATE_RIGHT),
            tr("Rotate &Right"), 0, this, 0);
    connect(m_rotateRight, SIGNAL(activated()), this, SLOT(rotateRight()));

    m_rotateLeft = new QAction("Rotate Left", IconScale(ICON_ROTATE_LEFT),
            tr("Rotate &Left"), 0, this, 0);
    connect(m_rotateLeft, SIGNAL(activated()), this, SLOT(rotateLeft()));

    m_open->addTo(m_mnFile);
    m_close->addTo(m_mnFile);
    m_option->addTo(m_mnFile);
    m_rotateLeft->addTo(m_mnFile);
    m_rotateRight->addTo(m_mnFile);

    m_open->addTo(m_tbIcon);
    m_option->addTo(m_tbIcon);
    m_close->addTo(m_tbIcon);

    m_tbMenu->insertItem(tr("&File"), m_mnFile);
    m_tbMenu->insertItem(tr("&Rescent"), m_mnRescent);

    connect(m_mnRescent, SIGNAL(activated(int)),
            this, SLOT(rescentSelected(int)));
    connect(m_mnPopup, SIGNAL(activated(int)),
            this, SLOT(rescentSelected(int)));
    setToolBarsMovable(false);
    addToolBar(m_tbMain);
    addToolBar(m_tbIcon);
    m_tbMenu->show();

    m_flist = new FileLister();

    m_view = new View(this, "view");
    m_view->show();
    m_doc = NULL;
    m_textdoc = new TextDocument(m_view);
    m_imagedoc = NULL;
    m_fullScreen = false;
    m_fullBeforeDialog = false;

    m_tbHeight = m_tbIcon->minimumSizeHint().height();
    if (m_tbMain->minimumSizeHint().height()>m_tbHeight)
        m_tbHeight = m_tbMain->minimumSizeHint().height();

    showMaximized();
    updateViewSize();

    m_browser = new FileSelect(this, "file_select");
    m_browser->hide();
    connect(m_browser, SIGNAL(endOpenFile()), this, SLOT(closeDialog()));
    connect(m_browser, SIGNAL(openFile(QString,FILE_TYPE,FILE_ENCODING)),
            this, SLOT(openFile(QString,FILE_TYPE,FILE_ENCODING)));

    m_optiondlg = new OptionDlg(this, "option_dialog");
    m_optiondlg->hide();
    connect(m_optiondlg, SIGNAL(endDialog()), this, SLOT(closeDialog()));
    connect(m_optiondlg, SIGNAL(settingChanged()),
            this, SLOT(reloadConfiguration()));

    /* Dialog for jumping to specified location */
    m_wAsk = new AskDialog(this);
    m_wAsk->hide();
    connect(m_wAsk, SIGNAL(jumpTo(int)), this, SLOT(setParagraph(int)));
    connect(m_wAsk, SIGNAL(search(QString, int)),
            this, SLOT(search(QString, int)));

    /* initialize values */
    m_closeReal = false;
    m_isDialog = false;
    showNormal();

    /* load configuratin */
    m_textdoc->loadConfig();
    //m_imagedoc->loadConfig();

    /* load configuration */
    if (g_cfg->getBool(ACFG_LoadLastFile)) {
        QString last_file = g_cfg->getLastFile();
        if (!last_file.isNull()) {
            openFile(last_file);
            m_browser->setDirectory(last_file);
        }
    } else {
        if (g_cfg->getBool(ACFG_LoadLastDir)) {
            m_browser->setDirectory(g_cfg->getLastDir());
        }
    }
    updateRescentFiles();

    m_keyUsedTime.setToNow();

    setFocusPolicy(QWidget::StrongFocus);
    this->setFocus();
}

AView::~AView()
{
    /* do nothing?? */
}

void
AView::resizeEvent(QResizeEvent *ev)
{
    QSize dsize = QApplication::desktop()->size();
    QSize newsize = ev->size();
    printf("AView::resizEvent(%d,%d)\n", newsize.width(), newsize.height());
    if (m_fullScreen) {
        hide();
        showMaximized();
    }
    updateViewSize();
    QMainWindow::resizeEvent(ev);
}

void
AView::keyPressEvent(QKeyEvent *ev)
{
    if (ev->isAutoRepeat()) return;
    //printf("AView::keyPressEvent(0x%04x)\n", ev->key());
    m_keyPressed = ev->key();
    m_keyPressTime.setToNow();
}

bool
AView::doAction(int key_code, int holding)
{
    bool deepPress = holding>700;

    // Too fast key press need to be ignored.
    if (m_keyUsedTime.passed()<200) {
        m_keyUsedTime.setToNow();
        return false;
    }

    switch(key_code) {
        case Qt::Key_Space:
            this->nextPage();
            break;
        case Qt::Key_Escape:
            if (deepPress)
                this->closeNow();
            else
                this->closeFile();
            break;
        case Qt::Key_Return:
            if (deepPress)
                this->showPopup();
            else
                this->toggleFullScreen();
            break;
        case Qt::Key_Down:
            if (m_doc==NULL) {
                openFile();
            } else {
                if (deepPress)
                    /*m_view->setSlideShowMode(2)*/;
                else
                    this->nextPage();
            }
            break;
        case Qt::Key_Up:
            if (m_doc==NULL) {
                openFile();
            } else {
                if (deepPress) {
                    showJump();
                } else {
                    this->prevPage();
                }
            }
            break;
        case Qt::Key_Right:
            m_view->rotateRight();
            break;
        case Qt::Key_Left:
            m_view->rotateLeft();
            break;
        case Qt::Key_O:
            openFile();
            break;
        case Qt::Key_T:
            showOptions();
            break;
        case Qt::Key_S:
            /*m_view->setSlideShowMode(2)*/;
            break;
        case Qt::Key_G:
            showJump();
            break;
        case Qt::Key_Slash:
            showSearch();
            break;
        default:
            return false;
    }
    m_keyUsedTime.setToNow();
    return true;
}

void
AView::keyReleaseEvent(QKeyEvent *ev)
{
    if (ev->isAutoRepeat()) return;

    int holding = 0;
    int p_key = ev->key();

    //printf("AView::keyReleaseEvent(0x%04x)\n", ev->key());
    if (p_key==Qt::Key_F33)
        p_key = Qt::Key_Return;

    if (m_keyPressed==p_key) {
        holding = m_keyPressTime.passed();
    } else {
        m_keyPressed = Qt::Key_unknown;
        ev->ignore();
        return;
    }
    m_keyPressed = Qt::Key_unknown;

    printf("AView::keyEvent(code=0x%04x,holding=%d)\n", p_key, holding);
    if (!doAction(p_key, holding)) {
        ev->ignore();
    }
}

void
AView::closeEvent(QCloseEvent *ev)
{
    if (m_closeReal) {
        ev->accept();
        return;
    }
    //printf("AView::closeEvent!!!\n");
    ev->ignore();
}

void
AView::openFile()
{
    openDialog(m_browser);
    if (m_doc && m_doc->hasDocument()) {
        m_browser->setDirectory(m_filename);
    } else {
        if (!g_cfg->getBool(ACFG_LoadLastDir)) {
            QString str = g_cfg->getLastFile();
            if (str.isEmpty() || str.isNull()) {
                m_browser->setDirectory("/");
            } else {
                m_browser->setDirectory(str);
            }
        } else {
            m_browser->setDirectory("/");
        }
    }
    m_browser->m_wFileList->setFocus();
    m_browser->repaint();
}

void
AView::closeDialog()
{
    if (m_isDialog) {
        setUpdatesEnabled(false);
        m_dialog->hide();
        setCentralWidget(NULL);
        m_isDialog = false;
        m_view->show();

        if (!m_fullBeforeDialog) {
            m_tbMain->show();
            m_tbIcon->show();
            setUpdatesEnabled(true);
            update();
        } else {
            showScreen(true);
        }
        this->setFocus();
    }
}

void
AView::openDialog(QWidget *w)
{
    m_fullBeforeDialog = m_fullScreen;

    setUpdatesEnabled(false);

    if (m_fullScreen) {
        m_fullScreen = false;
        showNormal();
    }
    m_tbMain->hide();
    m_tbIcon->hide();
    m_view->hide();
    m_isDialog = true;
    m_dialog = w;
    setCentralWidget(m_dialog);
    w->show();
    w->setFocus();

    setUpdatesEnabled(true);

    update();
}

#include <qlabel.h>
#include <qapplication.h>

void
AView::openFile(QString name)
{
    openFile(name, FT_AUTO, FE_AUTO);
}

void
AView::openFile(QString name, FILE_TYPE type_id, FILE_ENCODING encoding_id)
{
    int       para, offset, old_para, old_offset;
    QString   old_filename;

    QLabel    my_label(tr("Open file.... "), this);
    my_label.setMargin(20);
    my_label.resize(200, 60);
    my_label.move((width()-my_label.width())/2,(height()-my_label.height())/2);
    my_label.show();
    qApp->processEvents();

    /* store last document information */
    if (m_doc!=NULL && m_doc->getDocumentLocation(old_para, old_offset)) {
        old_filename = m_filename;
    } else {
        old_filename = (const char*)NULL;
        old_para = -1;
        old_offset = -1;
    }

    g_cfg->getLastPosition(name, para, offset);

    Document  *new_doc = NULL;
    if (type_id==FT_AUTO) {
        type_id = FileLister::getType(name, encoding_id);
    }

    switch (type_id) {
        case FT_IMAGE:
            printf("AView::openFile() fail to open image\n");
            break;
        case FT_TEXT:
            new_doc = m_textdoc;
            break;
    }

    if (new_doc==NULL) {
        my_label.setText(tr("Fail to open the file.."));
        qApp->processEvents();
        sleep(1);
        return;
    }

    /* change to new file */
    /* TODO we should select appropriate viewer for the file. */
    if (new_doc->setDocument(name, type_id, encoding_id, para, offset)) {
        if (!old_filename.isNull()) {
            g_cfg->setLastPosition(old_filename, old_para, old_offset);
            updateRescentFiles();
        }
        setCaption(tr("AView - %1").arg(name));
        m_filename = name;
        setDocument(new_doc);
    } else {
        printf("fail to setDocument(%s)\n", (const char*)name.utf8());
    }
}

void
AView::showMaximized()
{
    printf("AView::showMaximized:full=%s\n",
            m_fullScreen?"TRUE":"FALSE");
    if (!m_fullScreen) {
        showNormal();
    } else {
        hide();
        showFullScreen();
        resize(QPEApplication::desktop()->size());
        show();
        setFocus();
    }
}

void
AView::showNormal()
{
    printf("AView::showNormal:full=%s\n", m_fullScreen?"TRUE":"FALSE");
    QMainWindow::showNormal();
    QSize dsize = QPEApplication::desktop()->size();
    dsize.rheight()-=38;
    clearWFlags(WStyle_StaysOnTop);
    resize(dsize);
    setFocus();
}

void
AView::showScreen(bool isFull)
{
    printf("AView::showScreen(%d->%d)\n", m_fullScreen, isFull);

    if (isFull!=m_fullScreen) {
        setUpdatesEnabled(false);
        if (isFull) {
            m_fullScreen = true;
            showFullScreen();
        } else {
            m_fullScreen = false;
            showNormal();
        }
        /* when text view mode!! we must change toolbar status */
        if (centralWidget()==NULL) {
            if (m_fullScreen) {
                m_tbMain->hide();
                m_tbIcon->hide();
            } else {
                m_tbMain->show();
                m_tbIcon->show();
            }
        }
        setUpdatesEnabled(true);
        update();
    }
    updateViewSize();
    repaint();
}

void
AView::updateViewSize()
{
    if (!m_fullScreen) {
        m_view->setGeometry(0,m_tbHeight,width(), height()-m_tbHeight);
    } else {
        QSize dsize = QApplication::desktop()->size();
        m_view->setGeometry(0, 0, dsize.width(), dsize.height());
    }
}

void
AView::toggleFullScreen()
{
    showScreen(!m_fullScreen);
}

void
AView::updateRescentFiles()
{
    m_mnRescent->clear();
    QStringList   lst = g_cfg->getRescentFiles();
    QString       f_name;
    int idx=0;
    for (idx=0 ; idx<lst.count() ; idx++) {
        m_mnRescent->insertItem(lst[idx], idx, idx);
    }
}

void
AView::rescentSelected(int id)
{
    if (id>=0) {
        openFile(g_cfg->getRescentFiles()[id]);
    }
}

void
AView::showOptions()
{
    m_optiondlg->loadSettings();
    openDialog(m_optiondlg);
}


void
AView::showPopup()
{
    QRect mrect = rect();

    m_mnPopup->clear();
    m_open->addTo(m_mnPopup);
    m_close->addTo(m_mnPopup);
    m_option->addTo(m_mnPopup);
    m_rotateLeft->addTo(m_mnPopup);
    m_rotateRight->addTo(m_mnPopup);
    m_mnPopup->insertSeparator();

    int idx=0;
    QString name;
    QStringList   lst = g_cfg->getRescentFiles();
    for (idx=0 ; idx<lst.count() && idx<10 ; idx++) {
        name = lst[idx].right(20);
        m_mnPopup->insertItem(m_browser->m_pFile, name, idx);
    }

    QSize p_size = m_mnPopup->sizeHint();
    QPoint    popupPoint((mrect.width()-p_size.width())/2,
            (mrect.height()-p_size.height())/2);
    popupPoint = mapToGlobal(popupPoint);

    m_mnPopup->setActiveItem(0);

    m_mnPopup->exec(popupPoint, 0);

    /* restore to full screen mode */
    if (m_fullScreen) {
        hide();
        show();
    }
}

void
AView::rotateRight()
{
    m_view->rotateRight();
}

void
AView::rotateLeft()
{
    m_view->rotateLeft();
}

/**
 * close file.
 * it will pop document if pushed document exist.
 */
void
AView::closeFile()
{
    if (m_doc) {
        int     para, offset;
        if (m_doc->getDocumentLocation(para, offset)) {
            g_cfg->setLastPosition(m_filename, para, offset);
        }
        m_filename = (const char*)NULL;
        m_doc->resetDocument();
        setCaption(tr("AView"));
        setDocument(NULL);
    } else {
        closeNow();
    }
}

void
AView::closeNow()
{
    if (m_doc) {
        int     para, offset;
        if (m_doc->getDocumentLocation(para, offset)) {
            g_cfg->setLastPosition(m_filename, para, offset);
        }
    }
    g_cfg->setLastDir(m_browser->getDirectory());
    g_cfg->sync();
    m_closeReal = true;
    close();
}

void
AView::setParagraph(int para)
{
    if (m_doc) m_doc->setParagraph(para);
}

void
AView::search(QString key, int flags)
{
    if (m_doc) m_doc->search(key, flags);
}

void
AView::showJump()
{
    if (m_doc) {
        int     para, offset;
        if (m_doc->getDocumentLocation(para, offset)) {
            m_wAsk->jumpParagraph(para, m_doc->getParaCount());
        }
    }
}

void
AView::showSearch()
{
    if (m_doc && m_doc->canSearch()) {
        m_wAsk->searchParagraph("");
    }
}

void
AView::reloadConfiguration()
{
    m_textdoc->loadConfig();
    //m_imagedoc->loadConfig();
    m_view->loadConfig();
}

void
AView::setDocument(Document *doc)
{
    m_doc = doc;
    m_view->setMain(m_doc);
    m_view->updateAll();
}

void
AView::loadNext()
{
    if (m_doc) {
        int idx;
        QString new_file;
        idx = m_flist->setFile(m_filename);
        if (idx<0) return;
        new_file = m_flist->getFile(idx+1);
        if (new_file.isNull()) return;
        openFile(new_file, FT_AUTO, FE_AUTO);
    }
}

void
AView::loadPrev()
{
    if (m_doc) {
        int idx;
        QString new_file;
        idx = m_flist->setFile(m_filename);
        if (idx<1) return;
        new_file = m_flist->getFile(idx-1);
        if (new_file.isNull()) return;
        openFile(new_file, FT_AUTO, FE_AUTO);
    }
}

void
AView::viewMousePressEvent(QMouseEvent *ev)
{
    m_mousePressed = ev->button();
    m_mouseMove = false;
    m_mousePressTime.setToNow();
    m_mx = ev->x();
    m_my = ev->y();
}

int
AView::getAreaIndex(int x, int y, int width, int height, int rotation)
{
    int   nx, ny, nwidth, nheight;
    if (rotation<0) rotation = m_view->getRotation();
    while (rotation<0) rotation += 360;
    while (rotation>=360) rotation -= 360;
    switch (rotation) {
        case 0:
            nx = x; ny = y; nwidth = width; nheight = height;
            break;
        case 90:
            ny = width-x; nx = y; nwidth = height; nheight = width;
            break;
        case 180:
            nx = width-x; ny = height-y; nwidth = width; nheight = height;
            break;
        case 270:
            ny = x; nx = height-y; nwidth = height; nheight = width;
            break;
    }
    printf("AView::getAreaIndex(nx=%d,ny=%d,nw=%d,nh=%d,rot=%d)\n",
            nx, ny, nwidth, nheight,rotation);

    int idx = 0;
    if (ny>(nheight*1/3)) idx += 3;
    if (ny>(nheight*2/3)) idx += 3;
    if (nx>(nwidth*1/3)) idx += 1;
    if (nx>(nwidth*2/3)) idx += 1;

    return idx;
}

#define MOVEKEY(from,to) ((from)*9+(to))
#define TOP_LEFT        0
#define TOP_CENTER      1
#define TOP_RIGHT       2
#define MIDDLE_LEFT     3
#define MIDDLE_CENTER   4
#define MIDDLE_RIGHT    5
#define BOTTOM_LEFT     6
#define BOTTOM_CENTER   7
#define BOTTOM_RIGHT    8

void
AView::viewMouseReleaseEvent(QMouseEvent *ev)
{
    if (m_mouseMove) {
        m_mousePressed = 0;
        int fidx = getAreaIndex(m_mx,m_my,m_view->width(),m_view->height());
        int tidx = getAreaIndex(ev->x(),ev->y(),
                m_view->width(), m_view->height());
        switch (MOVEKEY(fidx,tidx)) {
            case MOVEKEY(BOTTOM_LEFT,BOTTOM_RIGHT):
            case MOVEKEY(BOTTOM_RIGHT,TOP_RIGHT):
            case MOVEKEY(TOP_RIGHT,TOP_LEFT):
            case MOVEKEY(TOP_LEFT,BOTTOM_LEFT):
                rotateLeft();
                m_keyUsedTime.setToNow();
                break;
            case MOVEKEY(BOTTOM_RIGHT,BOTTOM_LEFT):
            case MOVEKEY(TOP_RIGHT,BOTTOM_RIGHT):
            case MOVEKEY(TOP_LEFT,TOP_RIGHT):
            case MOVEKEY(BOTTOM_LEFT,TOP_LEFT):
                rotateRight();
                m_keyUsedTime.setToNow();
                break;
            case MOVEKEY(MIDDLE_CENTER,MIDDLE_LEFT):
            case MOVEKEY(MIDDLE_RIGHT,MIDDLE_CENTER):
            case MOVEKEY(MIDDLE_RIGHT,MIDDLE_LEFT):
                loadNext();
                m_keyUsedTime.setToNow();
                break;
            case MOVEKEY(MIDDLE_CENTER,MIDDLE_RIGHT):
            case MOVEKEY(MIDDLE_LEFT,MIDDLE_CENTER):
            case MOVEKEY(MIDDLE_LEFT,MIDDLE_RIGHT):
                loadPrev();
                m_keyUsedTime.setToNow();
                break;
            case MOVEKEY(MIDDLE_CENTER,TOP_CENTER):
                closeFile();
                m_keyUsedTime.setToNow();
                break;
            case MOVEKEY(MIDDLE_CENTER,BOTTOM_CENTER):
                openFile();
                m_keyUsedTime.setToNow();
                break;
            case MOVEKEY(BOTTOM_CENTER,TOP_CENTER):
                closeNow();
                break;
        }
        return;
    }

    int holding = m_mousePressTime.passed();
    printf("AView::click!!! (%d,%d) holding=%d\n",
            ev->x(), ev->y(), holding);

    switch (getAreaIndex(ev->x(), ev->y(), m_view->width(), m_view->height())) {
        case MIDDLE_CENTER: // center
            doAction(QKeyEvent::Key_Return, holding);
            break;
        case TOP_LEFT:
        case TOP_CENTER:
        case TOP_RIGHT:
            doAction(QKeyEvent::Key_Up, holding);
            break;
        case BOTTOM_LEFT:
        case BOTTOM_CENTER:
        case BOTTOM_RIGHT:
            doAction(QKeyEvent::Key_Down, holding);
            break;
    }
}

void
AView::viewMouseMoveEvent(QMouseEvent *ev)
{
    int   dx, dy, dist;
    if (m_mousePressed==0) return;
    dx = m_mx-ev->x();
    dy = m_my-ev->y();
    dist = (int)(sqrt(dx*dx+dy*dy)+0.5f);
    if (dist > 16) {
        //printf("AView::moved to far %d\n", dist);
        m_mouseMove = true;
    }
}

void
AView::nextPage()
{
    if (m_doc==NULL) return;
    if (m_doc->nextPage()) {
        m_view->updateMain(HINT_PAGE_DOWN);
        m_view->updateBar();
        return;
    }
    loadNext();
}

void
AView::prevPage()
{
    if (m_doc==NULL) return;
    if (m_doc->prevPage()) {
        m_view->updateMain(HINT_PAGE_UP);
        m_view->updateBar();
        return;
    }
    loadPrev();
}
