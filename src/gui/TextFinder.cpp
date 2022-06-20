#include "TextFinder.h"
#include <QLabel>
#include <QKeyEvent>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

TextFinder::TextFinder(QWidget *parent)
  : QWidget(parent)
  , line_edit_find(new QLineEdit)
  , line_edit_replace(new QLineEdit)
  , button_find_next(new QPushButton)
  , button_find_prev(new QPushButton)
  , button_replace(new QPushButton)
  , button_replace_all(new QPushButton)
  , button_advanced(new QPushButton) {
  button_find_prev->setIcon(QIcon(":/images/search-find-prev.png"));
  button_find_next->setIcon(QIcon(":/images/search-find-next.png"));
  button_replace->setText(tr("Replace"));
  button_replace_all->setText(tr("Replace All"));
  button_advanced->setText(tr("Advanced"));
  button_advanced->setEnabled(false);
  QLabel *findLabel = new QLabel(tr("Find:"));
  QLabel *replaceLabel = new QLabel(tr("Replace:"));
  QGridLayout *layout = new QGridLayout;
  layout->addWidget(findLabel, 0, 0);
  layout->addWidget(replaceLabel, 1, 0);
  layout->addWidget(line_edit_find, 0, 1);
  layout->addWidget(line_edit_replace, 1, 1);
  layout->addWidget(button_find_prev, 0, 2);
  layout->addWidget(button_find_next, 0, 3);
  layout->addWidget(button_replace, 1, 2, 1, 2);
  // layout->addWidget(replaceAllButton, 1, 4);
  // layout->addWidget(advancedButton, 0, 4);
  layout->setMargin(4);
  setLayout(layout);

  line_edit_find->installEventFilter(this);
  line_edit_replace->installEventFilter(this);
}
bool TextFinder::eventFilter(QObject *target, QEvent *event) {
  if (target == line_edit_find) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Return)
        emit button_find_next->clicked(true);
    }
  } else if (target == line_edit_replace) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Return)
        emit button_replace->clicked(true);
    }
  }
  return false;
}
