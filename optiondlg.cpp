/**
 * Option Dialog
 *
 * vim:sw=2:et
 */
#include "optiondlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "aconfig.h"
#include <stdio.h>
#include <qfontdatabase.h>
#include <qslider.h>

/* 
 *  Constructs a OptionDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
OptionDlg::OptionDlg( QWidget* parent,  const char* name, WFlags fl )
    : OptionForm( parent, name, fl )
{
  connect(m_btnOK, SIGNAL(clicked()), this, SLOT(updateSettings()));
  connect(m_btnCancel, SIGNAL(clicked()), this, SIGNAL(endDialog()));

  // font option ( check OptionDlg::updateExFont method );
  m_wFonts->insertItem(tr("Default Font"), 0);
  //m_wFonts->insertItem("Progress Bar", 1);

  // color option ( check OptionDlg::updateExColor method )
  m_wColors->insertItem(tr("Background"), 0);
  m_wColors->insertItem(tr("Normal Text"), 1);
  m_wColors->insertItem(tr("HighLight Text 1"), 2);
  m_wColors->insertItem(tr("HighLight Text 2"), 3);
  m_wColors->insertItem(tr("HighLight Text 3"), 4);
  m_wColors->insertItem(tr("HighLight Text 4"), 5);
  m_wColors->insertItem(tr("HighLight Text 5"), 6);
  m_wColors->insertItem(tr("HighLight Text 6"), 7);
  m_wColors->insertItem(tr("HighLight Text 7"), 8);
  m_wColors->insertItem(tr("Progress Bar FG"), 9);
  m_wColors->insertItem(tr("Progress Bar BG"), 10);
  m_wColors->insertItem(tr("Progress Bar Text"), 11);

  m_wFontName->insertStringList(QFontDatabase().families());

  connect(m_wFontName, SIGNAL(activated(int)), this, SLOT(updateFontFace()));
  connect(m_wFontSize, SIGNAL(valueChanged(int)), this, SLOT(updateFontFace()));
  connect(m_wFonts, SIGNAL(activated(int)), this, SLOT(changeFont()));
  connect(m_wBold, SIGNAL(toggled(bool)), this, SLOT(updateFontFace()));
  connect(m_wFakeBold, SIGNAL(toggled(bool)), this, SLOT(updateFontFace()));

  connect(m_wColors, SIGNAL(activated(int)), this, SLOT(changeColor()));
  connect(m_wColorEdit, SIGNAL(returnPressed()), this, SLOT(updateColorFace()));
  connect(m_wRed, SIGNAL(valueChanged(int)), this, SLOT(rgbChange()));
  connect(m_wGreen, SIGNAL(valueChanged(int)), this, SLOT(rgbChange()));
  connect(m_wBlue, SIGNAL(valueChanged(int)), this, SLOT(rgbChange()));

  connect(m_wMargin, SIGNAL(valueChanged(int)), this, SLOT(updateMargin(int)));
  connect(m_wLineMargin, SIGNAL(valueChanged(int)),
                                      this, SLOT(updateLineMargin(int)));
  connect(m_wRotation, SIGNAL(valueChanged(int)),
                                      this, SLOT(updateRotation(int)));
  connect(m_wShowBar, SIGNAL(toggled(bool)),
                                      this, SLOT(updateBar()));
  connect(m_wBarHeight, SIGNAL(valueChanged(int)),
                                      this, SLOT(updateBar()));

  connect(m_wScrollHeight, SIGNAL(valueChanged(int)),
                                      this, SLOT(updateScrollHeight(int)));
  connect(m_wScrollDelay, SIGNAL(valueChanged(int)),
                                      this, SLOT(updateScrollDelay(int)));

  connect(m_wScalingMethod,SIGNAL(activated(int)),
      this, SLOT(updateScaleMethod(int)));
  connect(m_wScaleFactor,SIGNAL(valueChanged(int)),
      this, SLOT(updateScaleFactor(int)));
  connect(m_wScaleUp,SIGNAL(stateChanged(int)),
      this, SLOT(updateScaleUp(int)));
  connect(m_wHSlices,SIGNAL(valueChanged(int)),
      this, SLOT(updateSlicingCount()));
  connect(m_wVSlices,SIGNAL(valueChanged(int)),
      this, SLOT(updateSlicingCount()));

  connect(m_wScrollMethod,SIGNAL(activated(int)),
      this, SLOT(updateScrollPolicy(int)));
  connect(m_wPageMethod, SIGNAL(activated(int)),
      this, SLOT(updatePagingPolicy(int)));

  setFocusPolicy(QWidget::StrongFocus);

  m_cfg = NULL;
}

/*
 *  Destroys the object and frees any allocated resources
 */
OptionDlg::~OptionDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void
OptionDlg::keyPressEvent(QKeyEvent *ev)
{
  //printf("OptionDlg::keyPressEvent(0x%04x\n", ev->key());
  switch (ev->key()) {
    case Qt::Key_Escape:
      emit endDialog();
      break;
    case Qt::Key_F33:
      emit updateSettings();
      break;
    default:
      ev->ignore();
  }
}

void
OptionDlg::keyReleaseEvent(QKeyEvent *ev)
{
}

void
OptionDlg::updateSettings()
{
  g_cfg->apply(m_cfg);
  g_cfg->sync();
  emit settingChanged();
  emit endDialog();
}

void
OptionDlg::updateFontFace()
{
  QString fnt_name = m_wFontName->currentText();
  int fnt_size = m_wFontSize->value();
  int fnt_weight = m_wBold->isChecked() ? QFont::Bold : QFont::Normal;

  m_wFontView->setFont(QFont(fnt_name, fnt_size, fnt_weight, false));

  if (!m_loading) {
    updateExFont(true);
  }
}

void
OptionDlg::updateColorFace()
{
  m_wColorView->setBackgroundColor(QColor(m_wColorEdit->text()));
  if (!m_loading) updateExColor(true);
}

void
OptionDlg::updateExFont(bool save)
{
  int idx = m_wFonts->currentItem();
  if (save) {
    switch (idx) {
      case 0: // Default Font.
        m_cfg->getString(ACFG_FontName) = m_wFontName->currentText();
        m_cfg->getInt(ACFG_FontSize) = m_wFontSize->value();
        m_cfg->getBool(ACFG_FontBold) = m_wBold->isChecked();
        m_cfg->getBool(ACFG_FakeBold) = m_wFakeBold->isChecked();
        break;
      default:
        return;
    }
  } else {
    QString fnt_name;
    int     fnt_size;
    bool    fnt_bold, fnt_fakebold;
    bool    fnt_style = false;
    switch (idx) {
      case 0:
        fnt_name = m_cfg->getString(ACFG_FontName);
        fnt_size = m_cfg->getInt(ACFG_FontSize);
        fnt_bold = m_cfg->getBool(ACFG_FontBold);
        fnt_fakebold = m_cfg->getBool(ACFG_FakeBold);
        fnt_style = true;
        break;
      default :
        return;
    }
    /* set gui font name*/
    int cnt;
    for (cnt=0 ; cnt<m_wFontName->count() ; cnt++) {
      if (fnt_name==m_wFontName->text(cnt)) {
        m_wFontName->setCurrentItem(cnt);
        break;
      }
    }
    if (cnt>=m_wFontName->count()) {
      m_wFontName->insertItem(fnt_name, m_wFontName->count());
      m_wFontName->setCurrentItem(cnt);
    }

    m_wFontSize->setValue(fnt_size);

    if (fnt_style) {
      m_wBold->setEnabled(true);
      m_wFakeBold->setEnabled(true);
      m_wBold->setChecked(fnt_bold);
      m_wFakeBold->setChecked(fnt_fakebold);
    } else {
      m_wBold->setEnabled(false);
      m_wFakeBold->setEnabled(false);
    }
  }
}

int color_names[] = {
  ACFG_Background,
  ACFG_TextColor,
  ACFG_HighLight1,
  ACFG_HighLight2,
  ACFG_HighLight3,
  ACFG_HighLight4,
  ACFG_HighLight5,
  ACFG_HighLight6,
  ACFG_HighLight7,
  ACFG_BarFG,
  ACFG_BarBG,
  ACFG_BarText };

void
OptionDlg::updateExColor(bool save)
{
  int idx = m_wColors->currentItem();
  if (idx<0 || idx>11) return;
  if (save) {
    m_cfg->getColor(color_names[idx]).setNamedColor(m_wColorEdit->text());
  } else {
    QColor co = m_cfg->getColor(color_names[idx]);
    m_wColorEdit->setText(co.name());
    m_wRed->setValue(co.red());
    m_wGreen->setValue(co.green());
    m_wBlue->setValue(co.blue());
    updateColorFace();
  }
}

void
OptionDlg::changeFont()
{
  m_loading = true;
  updateExFont(false);
  updateFontFace();
  m_loading = false;
}

void
OptionDlg::changeColor()
{
  m_loading = true;
  updateExColor(false);
  updateColorFace();
  m_loading = false;
}

void
OptionDlg::rgbChange()
{
  if (m_loading) return;
  QColor    new_color(m_wRed->value(), m_wGreen->value(), m_wBlue->value());
  m_wColorEdit->setText(new_color.name());
  updateColorFace();
  updateExColor(true);
}

void
OptionDlg::updateRotation(int rot)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_Rotation) = rot;
  }
}

void
OptionDlg::updateScrollHeight(int ht)
{
  if (!m_loading)
    m_cfg->getInt(ACFG_ScrollHeight) = ht;
}

void
OptionDlg::updateScrollDelay(int out)
{
  if (!m_loading)
    m_cfg->getInt(ACFG_ScrollDelay) = out;
}

void
OptionDlg::updateBar()
{
  if (m_loading) return;

  m_cfg->getBool(ACFG_BarVisible) = m_wShowBar->isChecked();
  m_cfg->getInt(ACFG_BarHeight) = m_wBarHeight->value();
}

void
OptionDlg::updateLineMargin(int lm)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_LineMargin) = lm;
  }
}

void
OptionDlg::updateMargin(int m)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_Margin) = m;
  }
}

void
OptionDlg::updateScaleEx()
{
  if (m_wScalingMethod->currentItem()==1) {
    m_wScaleFactor->setEnabled(true);
    m_wScaleUp->setEnabled(false);
  } else {
    m_wScaleFactor->setEnabled(false);
    m_wScaleUp->setEnabled(true);
  }
}

void
OptionDlg::updateScaleMethod(int value)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_ScalingMethod) = value;
  }
  updateScaleEx();
}

void
OptionDlg::updateScaleFactor(int value)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_ScaleFactor) = value;
  }
}

void
OptionDlg::updateScaleUp()
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_ScaleUp) = m_wScaleUp->isChecked();
  }
}

void
OptionDlg::updateSlicingMethod(int value)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_SlicingMethod) = m_wSlicingMethod->currentItem();
  }
  updateSlicingEx();
}

void
OptionDlg::updateSlicingEx()
{
  if (m_wSlicingMethod->currentItem()==1) {
    m_wHSlices->setEnabled(true);
    m_wVSlices->setEnabled(true);
  } else {
    m_wHSlices->setEnabled(false);
    m_wVSlices->setEnabled(false);
  }
}

void
OptionDlg::updateSlicingCount()
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_HSlicing) = m_wHSlices->value();
    m_cfg->getInt(ACFG_VSlicing) = m_wVSlices->value();
  }
}

void
OptionDlg::updateScrollPolicy(int value)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_ScrollPolicy) = m_wScrollMethod->currentItem();
  }
}

void
OptionDlg::updatePagingPolicy(int value)
{
  if (!m_loading) {
    m_cfg->getInt(ACFG_PagingPolicy) = m_wPageMethod->currentItem();
  }
}


void
OptionDlg::loadSettings()
{
  if (!m_cfg) {
    m_cfg = g_cfg->copy();
    m_loading = true;

    updateExFont(false);
    updateFontFace();
    updateExColor(false);
    updateColorFace();

    m_wRotation->setValue(m_cfg->getInt(ACFG_Rotation));
    m_wLineMargin->setValue(m_cfg->getInt(ACFG_LineMargin));
    m_wMargin->setValue(m_cfg->getInt(ACFG_Margin));
    m_wShowBar->setChecked(m_cfg->getBool(ACFG_BarVisible));
    m_wBarHeight->setValue(m_cfg->getInt(ACFG_BarHeight));
    m_wScrollHeight->setValue(m_cfg->getInt(ACFG_ScrollHeight));
    m_wScrollDelay->setValue(m_cfg->getInt(ACFG_ScrollDelay));

    m_wScalingMethod->setCurrentItem(m_cfg->getInt(ACFG_ScalingMethod));
    m_wScaleFactor->setValue(m_cfg->getInt(ACFG_ScaleFactor));
    m_wScaleUp->setChecked(m_cfg->getBool(ACFG_ScaleUp));
    updateScaleEx();

    m_wSlicingMethod->setCurrentItem(m_cfg->getInt(ACFG_SlicingMethod));
    m_wHSlices->setValue(m_cfg->getInt(ACFG_HSlicing));
    m_wVSlices->setValue(m_cfg->getInt(ACFG_VSlicing));
    updateSlicingEx();

    m_wScrollMethod->setCurrentItem(m_cfg->getInt(ACFG_ScrollPolicy));
    m_wPageMethod->setCurrentItem(m_cfg->getInt(ACFG_PagingPolicy));
    m_loading = false;
  }
}
