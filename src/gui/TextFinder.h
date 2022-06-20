#ifndef TEXTFINDER_H
#define TEXTFINDER_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTextDocument;

class TextFinder : public QWidget {
  Q_OBJECT
public:
  explicit TextFinder(QWidget *parent = 0);

  QLineEdit *line_edit_find;
  QLineEdit *line_edit_replace;
  QPushButton *button_find_next;
  QPushButton *button_find_prev;
  QPushButton *button_replace;
  QPushButton *button_replace_all;
  QPushButton *button_advanced;

protected:
  bool eventFilter(QObject *target, QEvent *event);
};

#endif  // TEXTFINDER_H
