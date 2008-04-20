/**
 * AView MainWindow class.
 * 
 * It will process various request from viewer. Handling dialog and options.
 * 
 * @file aview.h
 * vim:sw=2:sts=2:et
 */
#ifndef __A_VIEW_H__
#define __A_VIEW_H__

#include <qmainwindow.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>

#include "timestamp.h"

class   QZFile;
class   TParser;
class   FileSelect;
class   OptionDlg;
class   GenConfig;
class   AskDialog;
class   BaseView;
class   TextView;
class   ImageView;
class   FileLister;

enum {
    VIEW_NONE,
    VIEW_TEXT,
    VIEW_IMAGE
};

class AView : public QMainWindow
{
  Q_OBJECT
  private:
    BaseView    *m_view;
    ImageView   *m_iview;
    TextView    *m_tview;
    int         m_mode;

    FileSelect  *m_browser;
    OptionDlg   *m_optiondlg;
    AskDialog   *m_wAsk;

    FileLister  *m_flist;
    QString     m_filename;

    QWidget     *m_dialog;
    bool        m_isDialog;
    bool        m_closeReal;

    QToolBar    *m_tbMain, *m_tbIcon;
    QMenuBar    *m_tbMenu;

    QPopupMenu  *m_mnFile, *m_mnRescent, *m_mnPopup;

    QAction     *m_open, *m_close, *m_find, *m_option;
    QAction     *m_rotateRight, *m_rotateLeft;

    /* key press status */
    int         m_keyPressed;
    TimeStamp   m_keyTime;

    /* ui status */
    bool        m_fullScreen, m_fullBeforeDialog;
    int         m_tbHeight;

  private:
    void resizeEvent(QResizeEvent *ev);
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
    void closeEvent(QCloseEvent *ev);

    void showScreen(bool isFull);
    void updateViewSize();
    void showView();

  public slots:
    void openFile();
    void closeFile();
    void closeNow();
    void closeDialog();
    void openDialog(QWidget *w);
    void openFile(QString name);
    void openFile(QString name, int parser_id, int encoding_id);
    void loadPrev();
    void loadNext();
    void toggleFullScreen();
    void updateRescentFiles();
    void rescentSelected(int id);
    void showOptions();
    void showPopup();
    void showJump();
    void showSearch();
    void showMaximized();
    void showNormal();
    void rotateRight();
    void rotateLeft();
    void setParagraph(int para);
    void search(QString key, int flags);
    void reloadConfiguration();

  public :
    AView(QMainWindow *parent=NULL, const char *name=NULL,
        WFlags f=WType_TopLevel);
    ~AView();
};

#endif
