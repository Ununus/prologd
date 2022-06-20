#include "AboutProgram.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QApplication>
#include <QDesktopWidget>
#include <QTextBrowser>
#include <QScrollBar>
#include <QDebug>

Help::Help(QWidget *parent)
  : QDialog(parent)
  , m_help_text(new QTextBrowser) {
  setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
  setWindowTitle(tr("Help"));

  QVBoxLayout *layout = new QVBoxLayout;
  QHBoxLayout *head_layout = new QHBoxLayout;
  m_help_text->setOpenExternalLinks(false);
  head_layout->addWidget(m_help_text);

  QPushButton *closeButton = new QPushButton("OK");
  QHBoxLayout *ok_layout = new QHBoxLayout;
  ok_layout->addWidget(closeButton);
  ok_layout->setAlignment(Qt::AlignmentFlag::AlignRight);

  layout->addLayout(head_layout);
  layout->addLayout(ok_layout);
  setLayout(layout);
  setWindowIcon(QIcon(":/images/prolog1.png"));
  connect(closeButton, SIGNAL(clicked(bool)), SLOT(close()));

  QFile file(qApp->applicationDirPath() + "/prologd-help.html");
  ;
  if (file.exists()) {
    file.open(QFile::OpenModeFlag::ReadOnly);
    m_help_text->setHtml(QString(file.readAll()));
    file.close();
  } else {
    m_help_text->setHtml(tr("No documentation available"));
  }
  m_help_text->zoomIn(2);
  // const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
  // resize(availableGeometry.width() * 4 / 6, availableGeometry.height() * 4 / 6);
  // move((availableGeometry.width()) / 6 + 20, (availableGeometry.height()) / 6 + 20);
  resize(800, 600);
}

void Help::setCurosrIfFound(QString word) {
  m_help_text->moveCursor(QTextCursor::MoveOperation::Start);
  bool fnd = false;
  if (!word.isEmpty()) {
    fnd = m_help_text->find(QString("Встроенный предикат ") + word, QTextDocument::FindWholeWords);
  }
  if (fnd) {
    // Передвинуть найденное наверх.
    // Почему то скролл не работает, двигаю курсором
    auto cur = m_help_text->textCursor();
    m_help_text->moveCursor(QTextCursor::MoveOperation::End);
    m_help_text->setTextCursor(cur);
  }
}

AboutProgram::AboutProgram(QWidget *parent)
  : QDialog(parent) {
  setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
  setWindowTitle(tr("About Program"));

  QTabWidget *tabWidget = new QTabWidget;
  QVBoxLayout *layout = new QVBoxLayout;
  QHBoxLayout *head = new QHBoxLayout;
  QLabel *programIcon = new QLabel;
  programIcon->setPixmap(QPixmap(":/images/prolog1.png").scaled(42, 42, Qt::IgnoreAspectRatio, Qt::FastTransformation));
  programIcon->setFixedSize(42, 42);
  QLabel *programName = new QLabel(tr("Prolog-D"));
  QLabel *programVersion = new QLabel(tr("Version: ") + " " + qApp->applicationVersion().chopped(2));
  head->addWidget(programIcon);
  head->addWidget(programName);
  head->addWidget(programVersion);
  layout->addLayout(head);
  layout->addWidget(tabWidget);
  QPushButton *closeButton = new QPushButton("OK");
  QHBoxLayout *okLayout = new QHBoxLayout;
  okLayout->addWidget(closeButton);
  okLayout->setAlignment(Qt::AlignmentFlag::AlignRight);
  layout->addLayout(okLayout);

  QPlainTextEdit *aboutProgramTextEdit = new QPlainTextEdit;
  aboutProgramTextEdit->setPlainText(
    tr("Tutorial system - interpreter\n"
       "1987-1996 Grigoriev S.G. - idea and algorithms\n"
       "1996-2008 Alexeev M.N., Baldin V.M. - realization\n"
       "2017-2022 Voronin A.V.  - interface\n"));
  aboutProgramTextEdit->setReadOnly(true);
  tabWidget->addTab(aboutProgramTextEdit, tr("About Program"));

  QString helpTxt = QString(
    "Ctrl + '<' -> МЕНЬШЕ\n"
    "Ctrl + '>' -> БОЛЬШЕ\n"
    "Ctrl + '=' -> РАВНО\n"
    "Ctrl + '*' -> УМНОЖЕНИЕ\n"
    "Ctrl + '+' -> СЛОЖЕНИЕ\n"
    "Ctrl + '1' -> НЕ\n"
    "Ctrl + '2' -> КОПИЯ\n"
    "Ctrl + '3' -> #\n"
    "Ctrl + '4' -> ВЫВОД\n"
    "Ctrl + '5' -> СЦЕП\n"
    "Ctrl + '6' -> :-.\n"
    "Ctrl + '7' -> ?\n"
    "Ctrl + '8' -> ДЛИНА\n"
    "Ctrl + '9' -> ()\n"
    "Ctrl + '0' -> ВВОДСИМВ\n"
    "Ctrl + '|' -> |\n"
    "Ctrl + '/' -> |\n"
    "Ctrl + '[' -> []\n"
    "Ctrl + ';' -> :-.\n");

  QPlainTextEdit *helpAbout = new QPlainTextEdit;
  helpAbout->setPlainText(helpTxt);
  helpAbout->setReadOnly(true);
  tabWidget->addTab(helpAbout, tr("Shortcuts"));

  setLayout(layout);
  setFixedWidth(360);
  setFixedHeight(240);
  connect(closeButton, SIGNAL(clicked(bool)), SLOT(close()));
}
