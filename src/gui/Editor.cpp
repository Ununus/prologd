#include "Editor.h"
#include <QTextStream>
#include <QPainter>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QMimeData>
#include <QTextDocumentFragment>
#include <QTextBlock>
#include <QDebug>

static bool canBeFunction(QString &str, int start, int &end) {
  bool othop = false;
  int op = 0;
  for (int i = start; i < str.size(); ++i) {
    if (i == start && !str[i].isLetter()) {
      end = i + 1;
      return false;
    }
    if (op == 0 && !othop && (str[i] == ':' || str[i] == '.' || str[i] == ',' || str[i] == ';'))
      return true;
    if (str[i] == "\"" && (i == 0 || (i > 1 && str[i - 1] != '\\')))
      othop = !othop;
    if (othop)
      continue;
    if (str[i] == '(') {
      ++op;
    } else if (str[i] == ')') {
      --op;
      if (op == 0) {
        end = i + 1;
        return true;
      }
    }
  }
  end = str.size();
  return true;
};

PlainTextEditWithLineNumberArea::PlainTextEditWithLineNumberArea(QWidget *parent)
  : QPlainTextEdit(parent)
  , m_line_number_area(new LineNumberArea(this)) {
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));

  updateLineNumberAreaWidth(0);

  QPalette p = palette();
  p.setColor(QPalette::Highlight, QColor(0, 125, 225));
  p.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
  setPalette(p);
}
int PlainTextEditWithLineNumberArea::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  // int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits + 1);
  int space = 3 + fontMetrics().width(QLatin1Char('9')) * (digits + 1);
  return space;
}
void PlainTextEditWithLineNumberArea::updateLineNumberAreaWidth(int) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}
void PlainTextEditWithLineNumberArea::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    m_line_number_area->scroll(0, dy);
  else
    m_line_number_area->update(0, rect.y(), m_line_number_area->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}
void PlainTextEditWithLineNumberArea::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(m_line_number_area);
  QColor lightBlue(210, 210, 250);
  painter.fillRect(event->rect(), lightBlue);
  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();
  QFontMetrics fnt_metric(font());
  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      // painter.drawText(4, top, m_line_number_area->width() - fnt_metric.horizontalAdvance('0'), fnt_metric.height(), Qt::AlignRight, number);
      painter.drawText(4, top, m_line_number_area->width() - fnt_metric.width('0'), fnt_metric.height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}
void PlainTextEditWithLineNumberArea::keyPressEvent(QKeyEvent *e) {
  if (e->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier) && (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)) {
    auto modifs = e->modifiers();
    modifs.setFlag(Qt::KeyboardModifier::ShiftModifier, false);
    e->setModifiers(modifs);
  }
  QPlainTextEdit::keyPressEvent(e);
}
void PlainTextEditWithLineNumberArea::resizeEvent(QResizeEvent *event) {
  QPlainTextEdit::resizeEvent(event);

  QRect cr = contentsRect();
  m_line_number_area->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}
Editor::Editor(QWidget *parent)
  : PlainTextEditWithLineNumberArea(parent)
  , m_file_modified_by_another_program(false)
  , m_line_number_area(new LineNumberArea(this))
  , m_highliter(new Highlighter(document())) {
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setLineWrapMode(QPlainTextEdit::NoWrap);
}
QString Editor::getInCursorWord() {
  auto cur = textCursor();
  if (cur.hasSelection()) {
    return cur.selectedText();
  }
  cur.movePosition(QTextCursor::MoveOperation::StartOfWord);
  cur.movePosition(QTextCursor::MoveOperation::EndOfWord, QTextCursor::MoveMode::KeepAnchor);
  return cur.selectedText();
}
bool Editor::isTextModified() const {
  return document()->isModified();
}
void Editor::loadFile(QFile *file) {
  m_file_info = QFileInfo(*file);
  setPlainText(QTextStream(file).readAll());
  m_file_modified_by_another_program = false;
}
void Editor::saveToFile(QFile *file) {
  m_file_info = QFileInfo(*file);
  QTextStream out(file);
  out << toPlainText();
  document()->setModified(false);
  m_file_modified_by_another_program = false;
}
QString Editor::getFileName() const {
  return m_file_info.fileName();
}
QString Editor::getFilePath() const {
  return m_file_info.filePath();
}
QFileInfo Editor::getFileInfo() const {
  return m_file_info;
}
void Editor::setModifiedByAnotherProgram(bool flag) {
  m_file_modified_by_another_program = flag;
}
bool Editor::isModifiedByAnotherProgram() const {
  return m_file_modified_by_another_program;
}
bool Editor::isUntitled() {
  return getFileName().isEmpty();
}
void Editor::setUntitled() {
  m_file_info = QFileInfo();
}
void Editor::pressEnter() {
  QTextCursor cur = textCursor();
  int cur_col = cur.columnNumber();
  bool isFunc = false;
  int ed = 0;
  int cnt_spaces = 0;
  int cnt_tabs = 0;
  QString str = cur.block().text();
  for (int i = 0; i < str.size(); ++i) {
    if (str[i] == ' ') {
      ++cnt_spaces;
      continue;
    }
    if (str[i] == '\t') {
      ++cnt_tabs;
      continue;
    }
    if (str[i].isSpace()) {
      continue;
    }
    if (str[i] == '%')
      break;
    isFunc = canBeFunction(str, i, ed);
    if (isFunc) {
      isFunc = false;
      for (int j = ed; j + 1 < str.size(); ++j) {
        if (str[j].isSpace())
          continue;
        if ((str[j] == ':' || str[j] == '<') && str[j + 1] == '-') {
          isFunc = true;
          ed = j + 2;
          break;
        }
        break;
      }
    }
    break;
  }

  cur.insertText("\n");
  if (isFunc && cur_col >= ed) {
    //    QString spaces;
    //    spaces.resize(ed, ' ');
    //    cur.insertText(spaces);
    cur.insertText("\t");
  } else if (cnt_spaces > 0) {
    QString spaces;
    spaces.resize(cnt_spaces, ' ');
    cur.insertText(spaces);
  } else if (cnt_tabs > 0) {
    QString spaces;
    spaces.resize(cnt_tabs, '\t');
    cur.insertText(spaces);
  }
}
void Editor::pressHome(bool sh_mod) {
  QTextCursor cur = textCursor();
  cur.clearSelection();
  cur.movePosition(QTextCursor::MoveOperation::StartOfLine, QTextCursor::KeepAnchor);
  QString stxt = cur.selectedText();
  cur.clearSelection();
  if (!stxt.trimmed().isEmpty()) {
    for (int i = 0; i < stxt.size(); ++i) {
      if (!stxt[i].isSpace())
        break;
      cur.movePosition(QTextCursor::MoveOperation::Right);
    }
  }
  auto newCursor = textCursor();
  auto cntMove = newCursor.position() - cur.position();
  if (sh_mod) {
    newCursor.movePosition(QTextCursor::MoveOperation::Left, QTextCursor::KeepAnchor, cntMove);
  } else {
    newCursor.movePosition(QTextCursor::MoveOperation::Left, QTextCursor::MoveAnchor, cntMove);
  }
  setTextCursor(newCursor);
}
void Editor::keyPressEvent(QKeyEvent *keyEvent) {
  m_ctrl_pressed = keyEvent->key() == Qt::Key_Control;
  if (m_completer && !m_completer->popup()->isVisible()) {
    if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
      pressEnter();
      keyEvent->ignore();
      return;
    }
  }
  if (keyEvent->key() == Qt::Key_Home && !keyEvent->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
    pressHome(keyEvent->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier));
    return;
  }
  // Быстрый ввод ключевых слов
  if (keyEvent->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
    if (keyEvent->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier) && keyEvent->nativeVirtualKey() == 187) {
      insertPlainText(QString("СЛОЖЕНИЕ(,)"));
      return;
    }
    QTextCursor cur;
    auto corcur = [&](int ct) {
      cur = textCursor();
      cur.movePosition(QTextCursor::MoveOperation::Left, QTextCursor::MoveMode::MoveAnchor, ct);
      setTextCursor(cur);
    };
    switch (keyEvent->nativeVirtualKey()) {
    case 187:
      insertPlainText(QString("РАВНО(,)"));
      corcur(2);
      return;
    case 188:
      insertPlainText(QString("МЕНЬШЕ(,)"));
      corcur(2);
      return;
    case 190:
      insertPlainText(QString("БОЛЬШЕ(,)"));
      corcur(2);
      return;
    case 107:
      insertPlainText(QString("СЛОЖЕНИЕ(,)"));
      corcur(2);
      return;
    case 106:
      insertPlainText(QString("УМНОЖЕНИЕ(,)"));
      corcur(2);
      return;
    case 220: insertPlainText(QString("|")); return;
    case 111: insertPlainText(QString("|")); return;
    case 191: insertPlainText(QString("?")); return;
    case 55: insertPlainText(QString("?")); return;
    case 186:
      insertPlainText(QString(":-."));
      corcur(1);
      return;
    case 54:
      insertPlainText(QString(":-."));
      corcur(1);
      return;
    case 51: insertPlainText(QString("#")); return;
    case 57:
      insertPlainText(QString("()"));
      corcur(1);
      return;
    case 219:
      insertPlainText(QString("[]"));
      corcur(1);
      return;
    case 49:
      insertPlainText(QString("НЕ()"));
      corcur(1);
      return;
    case 50:
      insertPlainText(QString("КОПИЯ(,)"));
      corcur(2);
      return;
    case 52:
      insertPlainText(QString("ВЫВОД()"));
      corcur(1);
      return;
    case 53:
      insertPlainText(QString("СЦЕП(,)"));
      corcur(2);
      return;
    case 56:
      insertPlainText(QString("ДЛИНА(,)"));
      corcur(2);
      return;
    case 48:
      insertPlainText(QString("ВВОДСИМВ()"));
      corcur(1);
      return;
    }
  }
  // if (keyEvent->key() == Qt::Key_Return && keyEvent->modifiers() == Qt::ShiftModifier)
  // keyEvent->setModifiers(Qt::NoModifier);
  if (m_completer && m_completer->popup()->isVisible()) {
    switch (keyEvent->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: keyEvent->ignore(); return;
    default: break;
    }
  }

  // Что то с completer ом связанное
  bool isShortcut = ((keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() == Qt::Key_Space);  // CTRL+E
  if (!m_completer || !isShortcut)
    PlainTextEditWithLineNumberArea::keyPressEvent(keyEvent);
  const bool ctrlOrShift = keyEvent->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!m_completer || (ctrlOrShift && keyEvent->text().isEmpty()))
    return;
  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
  bool hasModifier = (keyEvent->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();
  if (!isShortcut && (hasModifier || keyEvent->text().isEmpty() || completionPrefix.length() < 1 || eow.contains(keyEvent->text().right(1)))) {
    m_completer->popup()->hide();
    return;
  }
  if (completionPrefix != m_completer->completionPrefix()) {
    m_completer->setCompletionPrefix(completionPrefix);
    if (!isShortcut && m_completer->currentCompletion() == completionPrefix) {
      m_completer->popup()->hide();
      return;
    }
    m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
  }
  QRect cr = cursorRect();
  cr.setWidth(m_completer->popup()->sizeHintForColumn(0) + m_completer->popup()->verticalScrollBar()->sizeHint().width());
  m_completer->complete(cr);
}
void Editor::keyReleaseEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Control)
    m_ctrl_pressed = false;
  PlainTextEditWithLineNumberArea::keyReleaseEvent(e);
}
void Editor::wheelEvent(QWheelEvent *e) {
  if (m_ctrl_pressed) {
    QPoint numDegrees = e->angleDelta();
    if (!numDegrees.isNull()) {
      if (numDegrees.y() > 0) {
        emit positiveWheel();
      } else if (numDegrees.y() < 0) {
        emit negativeWheel();
      }
    }
    e->accept();
  } else {
    PlainTextEditWithLineNumberArea::wheelEvent(e);
  }
}
void Editor::dropEvent(QDropEvent *e) {
  const QMimeData *data = e->mimeData();
  if (data->hasText()) {
    // QStringList filepaths(QString(data->text()).split('\n', Qt::SplitBehaviorFlags::SkipEmptyParts));
      QStringList filepaths(QString(data->text()).split('\n', QString::SplitBehavior::SkipEmptyParts));
    for (QString filepath : filepaths) {
      if (filepath.startsWith("file:///")) {
        filepath.remove(0, 8);
        emit fileDropped(filepath);
      }
    }
  }
  e->accept();
}
void Editor::focusOutEvent(QFocusEvent *e) {
  m_ctrl_pressed = false;
  PlainTextEditWithLineNumberArea::focusOutEvent(e);
}
void Editor::setSearchString(const QString &string) {
  m_highliter->setSearchString(string);
}
void Editor::setCompleter(QCompleter *c) {
  if (m_completer)
    QObject::disconnect(c, 0, this, 0);
  m_completer = c;
  if (!m_completer)
    return;
  m_completer->setWidget(this);
  m_completer->setCompletionMode(QCompleter::PopupCompletion);
  // c->setCaseSensitivity(Qt::CaseInsensitive);
  m_completer->popup()->setFont(font());
  QObject::connect(m_completer, SIGNAL(activated(QString)), this, SLOT(slotInsertCompletion(QString)));
}
void Editor::slotInsertCompletion(const QString &completion) {
  if (m_completer->widget() != this)
    return;
  QTextCursor tc = textCursor();
  int extra = completion.length() - m_completer->completionPrefix().length();
  tc.movePosition(QTextCursor::Left);
  tc.movePosition(QTextCursor::EndOfWord);
  tc.insertText(completion.right(extra));
  setTextCursor(tc);
}
QString Editor::textUnderCursor() const {
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  return tc.selectedText();
}

void Editor::comment() {
  auto stcur = textCursor();
  int sel_start = stcur.selectionStart();
  int sel_end = stcur.selectionEnd();
  stcur.setPosition(sel_end);
  int ed_line = stcur.blockNumber();
  stcur.setPosition(sel_start);
  int st_line = stcur.blockNumber();
  stcur.movePosition(QTextCursor::MoveOperation::StartOfBlock);
  stcur.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, ed_line - st_line);
  stcur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  auto str_list = stcur.selection().toPlainText().split('\n');
  for (auto &str : str_list) {
    bool ha = false;
    for (int i = 0; i < str.size(); ++i) {
      if (!str[i].isSpace()) {
        ha = true;
        break;
      }
    }
    if (ha) {
      str.push_front('%');
    }
  }
  auto strr = str_list.join('\n');
  stcur.beginEditBlock();
  stcur.removeSelectedText();
  stcur.insertText(strr);
  stcur.movePosition(QTextCursor::MoveOperation::StartOfBlock);
  stcur.endEditBlock();
  setTextCursor(stcur);
}
void Editor::uncomment() {
  auto stcur = textCursor();
  int sel_start = stcur.selectionStart();
  int sel_end = stcur.selectionEnd();
  stcur.setPosition(sel_end);
  int ed_line = stcur.blockNumber();
  stcur.setPosition(sel_start);
  int st_line = stcur.blockNumber();
  stcur.movePosition(QTextCursor::MoveOperation::StartOfBlock);
  stcur.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, ed_line - st_line);
  stcur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  auto str_list = stcur.selection().toPlainText().split('\n');
  for (auto &str : str_list) {
    int adr = -1;
    for (int i = 0; i < str.size(); ++i) {
      if (str[i] == '%') {
        adr = i;
        break;
      }
    }
    if (adr >= 0) {
      str.remove(adr, 1);
    }
  }
  auto strr = str_list.join('\n');
  stcur.beginEditBlock();
  stcur.removeSelectedText();
  stcur.insertText(strr);
  stcur.movePosition(QTextCursor::MoveOperation::StartOfBlock);
  stcur.endEditBlock();
  setTextCursor(stcur);
}
void Editor::formatDocument() {
  auto curs = textCursor();
  int st_line = curs.blockNumber();
  int line_diff = 0;

  auto isOpr = [](QString &str, int start, int &end) {
    if (start + 1 >= str.size())
      return false;
    end = start + 2;
    return (str[start] == ':' || str[start] == '<') && str[start + 1] == '-';
  };
  auto haveNext = [](QString &str, int start) {
    for (int i = start; i < str.size(); ++i) {
      if (str[i].isSpace())
        continue;
      if (str[i] == '%')
        break;
      return true;
    }
    return false;
  };
  auto getStrToInsert = [](int cnt_spaces) {
    QString str;
    for (int i = 0; i < cnt_spaces; ++i) {
      str.append(' ');
    }
    return str;
  };
  auto str_list = toPlainText().split('\n');
  bool fnd_function = false;
  bool fnd_opr = false;
  bool fnd_function_2 = false;
  bool fst = false;
  int cnt_in_func = 0;
  int str_idx_after_opr = 0;
  int pos_after_opr = 0;
  for (int i = 0; i < str_list.size(); ++i) {
    str_list[i] = str_list[i].trimmed();
    if (str_list[i].isEmpty()) {
      if (fnd_opr) {
        str_list.erase(str_list.begin() + i);
        --i;
        if (i <= st_line) {
          --line_diff;
        }
      }
      continue;
    }
    QString &str = str_list[i];
    int st = 0, ed = 0;
    while (st < str.size() && str[st] != '%') {
      if (str[st].isSpace()) {
        ++st;
        continue;
      }
      if (fnd_opr) {
        if (fnd_function_2) {
          fnd_function_2 = false;
          if (str[st] == ',') {
            ++st;
            if (!haveNext(str, st)) {
              fst = true;
              break;
            }
            // auto insstr = getStrToInsert(pos_after_opr);
            // str.insert(st, "\n" + insstr);
            str.insert(st, "\n\t");
            if (i <= st_line) {
              ++line_diff;
            }
            fst = false;
            // st += insstr.size() + 1;
            st += 2;
          } else if (str[st] == '.' || str[st] == ';') {
            ++st;
            //            if (cnt_in_func > 1) {
            //              str_list[str_idx_after_opr].insert(pos_after_opr, "\n\t");
            //              if (str_idx_after_opr == i) st += 2;
            //              if (str_idx_after_opr<= st_line) {
            //                ++line_diff;
            //              }
            //              cnt_in_func = 0;
            //            }
            fst = fnd_function = fnd_opr = false;
            if (haveNext(str, st)) {
              // auto insstr = getStrToInsert(pos_after_opr);
              // str.insert(st,  "\n" + insstr);
              str.insert(st, "\n\t");
              if (i <= st_line) {
                ++line_diff;
              }
              // st += insstr.size() + 1;
              st += 2;
            }
          }
        } else {
          ed = st + 1;
          fnd_function_2 = canBeFunction(str, st, ed) || str[st] == '!';
          if (fnd_function_2) {
            ++cnt_in_func;
            if (fst) {
              str.insert(st, '\t');
              ++ed;
              //              auto insstr = getStrToInsert(pos_after_opr);
              //              str.insert(st, insstr);
              //              ed += insstr.size();
              fst = false;
            }
          } else {
            fnd_function = fnd_opr = false;
          }
          st = ed;
        }

      } else {
        cnt_in_func = 0;
        fst = false;
        if (fnd_function) {
          if (isOpr(str, st, ed)) {
            fnd_opr = true;
            str_idx_after_opr = i;
            pos_after_opr = st = ed;
            if (!haveNext(str, st)) {
              fst = true;
              break;
            }
            // str.insert(st, "\n\t");
          } else {
            fnd_function = false;
            ++st;
          }
        } else {
          fnd_function = canBeFunction(str, st, ed);
          st = ed;
        }
      }
    }
  }
  curs.beginEditBlock();
  curs.select(QTextCursor::SelectionType::Document);
  curs.removeSelectedText();
  insertPlainText(str_list.join('\n'));
  curs.setPosition(document()->findBlockByLineNumber(st_line + line_diff).position());
  curs.endEditBlock();
  setTextCursor(curs);
}
