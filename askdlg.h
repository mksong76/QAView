#ifndef ASKDIALOG_H
#define ASKDIALOG_H
#include "askform.h"

class AskDialog : public AskForm
{
    Q_OBJECT
  private:
    int     m_mode;
    int     m_max;

  protected:
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
    void showDialog();

  public:
    AskDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~AskDialog();

    bool jumpParagraph(int current, int max);
    bool searchParagraph(const QString &str);

  public slots:
    void inputEnter();
    void endDialog();

  signals:
    void jumpTo(int para);
    void search(QString str, int flags);
};

#endif // ASKDIALOG_H
