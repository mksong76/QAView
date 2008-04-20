#include "askdlg.h"
#include "parser.h"
#include <qapplication.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <stdio.h>

enum { ADM_NONE, ADM_JUMP, ADM_SEARCH };

AskDialog::AskDialog( QWidget* parent,  const char* name, WFlags fl )
    : AskForm( parent, name, fl )
{
  setFocusPolicy(QWidget::StrongFocus);
  m_mode = ADM_NONE;

  connect(m_wInput, SIGNAL(returnPressed()), this, SLOT(inputEnter()));
}

AskDialog::~AskDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

bool
AskDialog::jumpParagraph(int current, int max)
{
  if (m_mode!=ADM_NONE) return false;
  m_wTitle->setText(tr("Go to paragraph"));
  QString   mine;
  mine = tr("Range : 0 - %1").arg(max-1);
  m_wMsg->setText(mine);
  mine.sprintf("%d", current);
  m_wInput->setText(mine);
  m_max = max;

  m_mode = ADM_JUMP;
  showDialog();
  return true;
}

bool
AskDialog::searchParagraph(const QString &str)
{
  if (m_mode!=ADM_NONE) return false;
  m_wTitle->setText(tr("Search"));

  m_wMsg->setText("search text in document");
  m_wInput->setText("");

  m_mode = ADM_SEARCH;
  showDialog();
  return true;
}

void
AskDialog::showDialog()
{
  QWidget   *w = (QWidget*)parent();

  move((w->width()-this->width())/2,
      (w->height()-this->height())/2);
  show();
  qApp->processEvents();
  m_wInput->selectAll();
  m_wInput->setFocus();
}

void
AskDialog::inputEnter()
{
  if (m_mode==ADM_NONE) return;

  switch (m_mode) {
    case ADM_JUMP:
      {
        bool    ok;
        int     dst = m_wInput->text().toUInt(&ok);
        if (ok && dst>=0 && dst<m_max) {
          emit jumpTo(dst);
          endDialog();
        } else {
          m_wInput->selectAll();
          m_wInput->setFocus();
        }
      }
      break;
    case ADM_SEARCH:
      emit search(m_wInput->text(), SK_KEYWORD);
      break;
  }
}

void
AskDialog::endDialog()
{
  if (m_mode!=ADM_NONE) {
    QWidget *w = (QWidget*)parent();
    m_mode = ADM_NONE;
    hide();
    w->setFocus();
  }
}

void
AskDialog::keyPressEvent(QKeyEvent *ev)
{
  //printf("AskDialog::keyPressEvent(0x%04x)\n", ev->key());
  switch (ev->key()) {
    case Qt::Key_Escape:
      endDialog();
      break;
  }
  ev->accept();
}

void
AskDialog::keyReleaseEvent(QKeyEvent *ev)
{
  //printf("AskDialog::keyReleaseEvent(0x%04x)\n", ev->key());
  ev->accept();
}

