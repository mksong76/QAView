/****************************************************************************
** Form interface generated from reading ui file 'fileselect.ui'
**
** Created: Sat Mar 20 23:01:33 2004
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FILESELECT_H
#define FILESELECT_H

#include <qvariant.h>
#include <qwidget.h>
#include <qpixmap.h>
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QComboBox;
class QListView;
class QListViewItem;
class QPushButton;
class QZDir;

#include "timestamp.h"
#include "fileopen.h"

class FileSelect : public FileSelectForm
{
    Q_OBJECT

  public:
    QPixmap     m_pFile, m_pDir;

  protected:
    QZDir       *m_dir;

  public:
    FileSelect( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~FileSelect();

    QString getDirectory();
  private:
    void keyPressEvent(QKeyEvent*ev);
    void keyReleaseEvent(QKeyEvent*ev);

    int         m_key;
    TimeStamp   m_stamp;

  signals:
    void openFile(QString str, int parser_id, int encoding_id);
    void endOpenFile();

  public slots:
    void openDirectory();
    void refreshDirectory();
    void cdUp();
    void setDirectory(const QString &str);
    void setFile(const QString &name);
    void itemClicked(QListViewItem *item);
    void openFile();

    void selectParser(int idx);
};

#endif // FILESELECT_H
