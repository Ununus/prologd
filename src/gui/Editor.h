#ifndef EDITOR_H
#  ifndef EDITOR_H
#    define EDITOR_H

#    include "Highlighter.h"
#    include <QPlainTextEdit>
#    include <QFileInfo>
class LineNumberArea;
class QCompleter;

class PlainTextEditWithLineNumberArea : public QPlainTextEdit {
  Q_OBJECT
public:
  explicit PlainTextEditWithLineNumberArea(QWidget *parent = nullptr);
  int lineNumberAreaWidth();

  void lineNumberAreaPaintEvent(QPaintEvent *event);

protected:
  void keyPressEvent(QKeyEvent *e) override;
  void resizeEvent(QResizeEvent *event) override;
private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void updateLineNumberArea(const QRect &rect, int dy);

private:
  LineNumberArea *m_line_number_area;
  // PlainTextEditWithLineNumberArea*
};

class Editor : public PlainTextEditWithLineNumberArea {
  Q_OBJECT
public:
  explicit Editor(QWidget *parent);

  QString getInCursorWord();

  void setModifiedByAnotherProgram(bool flag = true);
  bool isModifiedByAnotherProgram() const;
  bool isTextModified() const;
  bool isUntitled();
  void setUntitled();

  QString getFileName() const;
  QString getFilePath() const;
  QFileInfo getFileInfo() const;
  void loadFile(QFile *file);
  void saveToFile(QFile *file);

  void setSearchString(const QString &string);
  void setCompleter(QCompleter *c);
  QString textUnderCursor() const;
public slots:
  void comment();
  void uncomment();
  void formatDocument();

protected:
  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;
  void wheelEvent(QWheelEvent *e) override;
  void dropEvent(QDropEvent *e) override;
  void focusOutEvent(QFocusEvent *e) override;
private slots:
  void slotInsertCompletion(const QString &completion);
signals:
  void fileDropped(QString filepath);
  void positiveWheel();
  void negativeWheel();

private:
  void pressEnter();
  void pressHome(bool sh_mod);

  bool m_file_modified_by_another_program;
  QFileInfo m_file_info;
  LineNumberArea *m_line_number_area;
  Highlighter *m_highliter;
  QCompleter *m_completer;
  bool m_ctrl_pressed{ false };
};

class LineNumberArea : public QWidget {
public:
  LineNumberArea(PlainTextEditWithLineNumberArea *editor)
    : QWidget(editor) {
    m_editor = editor;
  }
  QSize sizeHint() const Q_DECL_OVERRIDE {
    return QSize(m_editor->lineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
    m_editor->lineNumberAreaPaintEvent(event);
  }

private:
  PlainTextEditWithLineNumberArea *m_editor;
};

#  endif  // EDITOR_H
