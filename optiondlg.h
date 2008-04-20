#ifndef OPTIONDLG_H
#define OPTIONDLG_H

#include "optionform.h"

class GenConfig;

class OptionDlg : public OptionForm
{
    Q_OBJECT

  public:
    OptionDlg( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~OptionDlg();

    GenConfig   *m_cfg;
    bool        m_loading;
  private:
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

    void updateExFont(bool save);
    void updateExColor(bool save);

  signals:
    void endDialog();
    void settingChanged();

  private slots:
    void updateSettings();

    void updateFontFace();
    void changeFont();

    void updateColorFace();
    void changeColor();
    void rgbChange();

    void updateLineMargin(int value);
    void updateMargin(int value);
    void updateRotation(int value);

    void updateScrollHeight(int value);
    void updateScrollDelay(int value);

    void updateScaleMethod(int value);
    void updateScaleFactor(int value);
    void updateScaleUp();
    void updateScaleEx();

    void updateSlicingMethod(int value);
    void updateSlicingEx();
    void updateSlicingCount();

    void updateScrollPolicy(int value);
    void updatePagingPolicy(int value);

    void updateBar();
  public slots:
    void loadSettings();
};

#endif // OPTIONDLG_H
