#ifndef ABOUTPROGRAM_H
#define ABOUTPROGRAM_H

#include <QDialog>

class QTextBrowser;

class Help : public QDialog
{
  Q_OBJECT
public:
  explicit Help (QWidget *parent = nullptr);

  void setCurosrIfFound(QString word);

private:
  QTextBrowser *m_help_text;
};

class AboutProgram : public QDialog
{
  Q_OBJECT
public:
  explicit AboutProgram(QWidget *parent = nullptr);
};

#endif // ABOUTPROGRAM_H
