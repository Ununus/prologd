#include "MainWindow.h"
#include <QApplication>
#include <QSplitter>
#include <QFileSystemWatcher>
#include <QCompleter>
#include <QInputDialog>
#include <QSettings>
#include <QAction>
#include <QMessageBox>
#include <QDir>
#include <QFileIconProvider>
#include <QDesktopWidget>
#include <QToolBar>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QThread>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>

#include <algorithm>

#include <QDebug>

// TODO: брать это из prlib
const int NamesCnt = 49;
extern const char *KeyNames[];

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_main_widget(new QSplitter(Qt::Horizontal))
  , m_put_widget(new QSplitter(Qt::Vertical))
  , m_editors(new QTabWidget)
  , m_output_text(new PlainTextEditWithLineNumberArea(this))
  , m_input_text(new PlainTextEditWithLineNumberArea(this))
  , m_input_dialog(nullptr)
  , m_file_system_watcher(new QFileSystemWatcher(this))
  , m_filepath_2_tab_content(new QMap<QString, TabContent>)
  , m_editor_font(new QFont)
  , m_text_finder(new TextFinder)
  , m_about_program(nullptr)
  , m_help(nullptr)
  , m_settings(new Settings(this))
  , m_completer(new QCompleter)
  , m_execution_thread(nullptr)
  , m_prolog_worker(nullptr)
  , m_input_spaces_as_separators(new QCheckBox)
  , m_output_print_questions(new QCheckBox)
  , m_grp(new GraphicsWidget(nullptr))
  , m_input_console(new QGroupBox)
  , m_input_console_line_edit(new QLineEdit) {
  setWindowTitle(tr("Пролог-Д"));

  Actions *actions = m_settings->getActions();
  setMenuBar(actions->menu_bar);
  addToolBar(Qt::LeftToolBarArea, actions->tool_bar);

  QSettings qsettings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
  const QByteArray geometry = qsettings.value("geometry", QByteArray()).toByteArray();
  if (!restoreGeometry(geometry)) {
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    resize(availableGeometry.width() * 6 / 8, availableGeometry.height() * 6 / 8);
    move((availableGeometry.width()) / 8, (availableGeometry.height()) / 8);
    // showMaximized();
  }

  m_editors->setMovable(true);
  m_editors->setTabsClosable(true);
  newFile();

  QWidget *outputWidget = new QWidget;
  QVBoxLayout *output_vlay = new QVBoxLayout;
  QHBoxLayout *output_hlay = new QHBoxLayout;
  QLabel *output_label = new QLabel(tr("Вывод"));
  output_label->setStyleSheet("QLabel { background-color : white; border: 1px solid lightgray; }");
  m_output_print_questions->setText(tr("Выводить вопросы"));
  // output_pb->setIcon(QIcon(":/images/file-new.png"));
  // m_output_print_questions->setCheckable(true);
  m_output_print_questions->setChecked(true);
  // m_output_print_questions->setFixedWidth(32);
  output_hlay->addWidget(output_label);
  output_hlay->addSpacing(4);
  output_hlay->addWidget(m_output_print_questions);
  output_hlay->setAlignment(Qt::AlignmentFlag::AlignLeft);
  output_vlay->addLayout(output_hlay);
  output_vlay->addWidget(m_output_text);
  output_vlay->setMargin(2);
  output_vlay->setSpacing(0);
  outputWidget->setLayout(output_vlay);
  m_output_text->setFont(m_settings->getFont());
  m_output_text->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
  m_output_text->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
  // outputTextEdit->setReadOnly(true);

  QWidget *inputWidget = new QWidget;
  QVBoxLayout *input_vlay = new QVBoxLayout;

  QVBoxLayout *m_input_console_layout = new QVBoxLayout;
  QHBoxLayout *m_input_line_edit_layout = new QHBoxLayout;
  QLabel *m_console_label = new QLabel(">_");
  m_input_line_edit_layout->addWidget(m_console_label);
  m_input_line_edit_layout->addWidget(m_input_console_line_edit);
  m_input_line_edit_layout->setMargin(0);
  m_input_console_layout->addLayout(m_input_line_edit_layout);
  m_input_console->setLayout(m_input_console_layout);
  input_vlay->addWidget(m_input_console);
  m_input_console->setVisible(false);
  // m_input_console->setStyleSheet("border: 1px solid red; margin-top: 0.5em;");
  // m_input_console_line_edit->setStyleSheet("border: 1px solid gray; border-radius: 6px;");

  QHBoxLayout *input_hlay = new QHBoxLayout;
  QLabel *input_label = new QLabel(tr("Ввод"));
  input_label->setStyleSheet("QLabel { background-color : white; border: 1px solid lightgray; }");
  m_input_spaces_as_separators->setText(tr("Использовать пробелы как разделитель"));
  // input_pb->setIcon(QIcon(":/images/file-new.png"));
  // m_input_spaces_as_separators->setCheckable(true);
  m_input_spaces_as_separators->setChecked(true);
  // m_input_spaces_as_separators->setFixedWidth(32);
  input_hlay->addWidget(input_label);
  input_hlay->addSpacing(4);
  input_hlay->addWidget(m_input_spaces_as_separators);
  input_hlay->setAlignment(Qt::AlignmentFlag::AlignLeft);
  input_vlay->addLayout(input_hlay);
  input_vlay->addWidget(m_input_text);
  input_vlay->setMargin(2);
  input_vlay->setSpacing(0);
  inputWidget->setLayout(input_vlay);

  m_input_text->setFont(m_settings->getFont());
  m_input_text->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
  m_input_text->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);

  QWidget *topWidget = new QWidget;
  QVBoxLayout *topLayout = new QVBoxLayout;
  topLayout->addWidget(m_editors);
  topLayout->addWidget(m_text_finder);
  topLayout->setMargin(0);
  topLayout->setSpacing(0);
  topWidget->setLayout(topLayout);
  // putWidget->addWidget(outputTabWidget);
  m_put_widget->addWidget(outputWidget);
  m_put_widget->addWidget(inputWidget);
  QList<int> sizes;
  sizes << 400 << 1;
  m_put_widget->setSizes(sizes);

  m_main_widget->setHandleWidth(0);
  m_main_widget->addWidget(topWidget);
  m_main_widget->addWidget(m_put_widget);

  sizes[0] = 1;
  sizes[1] = 400;
  m_main_widget->setSizes(sizes);
  m_main_widget->setStretchFactor(0, 1);
  m_main_widget->setStretchFactor(1, 0);
  setCentralWidget(m_main_widget);
  m_text_finder->setVisible(false);

  QStringList keyWords;
  for (int i = 0; i < NamesCnt; ++i) {
    keyWords.append(QString(KeyNames[i]));
  }
  std::sort(keyWords.begin(), keyWords.end());
  QStringListModel *stringListModel = new QStringListModel(keyWords, m_completer);
  m_completer->setModel(stringListModel);
  m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
  m_completer->setCaseSensitivity(Qt::CaseSensitive);
  m_completer->setWrapAround(false);

  actions->tool_bar->setVisible(actions->action_tool_bar->isChecked());
  // outputTabWidget->setVisible(actions->actionOutputBar->isChecked());
  actions->action_abort->setEnabled(false);
  connect(m_editors, SIGNAL(tabCloseRequested(int)), SLOT(closeEditor(int)));
  m_connection_current_index_changed = connect(m_editors, SIGNAL(currentChanged(int)), SLOT(setCurrentEditor(int)));
  connect(m_file_system_watcher, SIGNAL(fileChanged(QString)), SLOT(updateEditor(QString)));

  connect(m_text_finder->button_find_next, SIGNAL(clicked(bool)), SLOT(editFindNext()));
  connect(m_text_finder->button_find_prev, SIGNAL(clicked(bool)), SLOT(editFindPrev()));
  connect(m_text_finder->button_replace, SIGNAL(clicked(bool)), SLOT(editReplace()));

  connect(m_settings, SIGNAL(signalTextEditorSettingsChanged()), SLOT(updateEditor()));

  connect(actions->action_new_file, SIGNAL(triggered(bool)), SLOT(newFile()));
  connect(actions->action_open_file, SIGNAL(triggered(bool)), SLOT(openFile()));
  connect(actions->action_save_file, SIGNAL(triggered(bool)), SLOT(saveFile()));
  connect(actions->action_save_file_as, SIGNAL(triggered(bool)), SLOT(saveFileAs()));
  connect(actions->action_close_file, SIGNAL(triggered(bool)), SLOT(closeFile()));
  connect(actions->action_close_all_files, SIGNAL(triggered(bool)), SLOT(closeAllFiles()));
  connect(actions->action_exit, SIGNAL(triggered(bool)), SLOT(close()));

  connectEditActions();

  connect(actions->action_find, SIGNAL(triggered(bool)), SLOT(editFind()));
  connect(actions->action_find_next, SIGNAL(triggered(bool)), SLOT(editFindNext()));
  connect(actions->action_find_prev, SIGNAL(triggered(bool)), SLOT(editFindPrev()));
  connect(actions->action_find_selected_next, SIGNAL(triggered(bool)), SLOT(editFindSelectedNext()));
  connect(actions->action_find_selected_prev, SIGNAL(triggered(bool)), SLOT(editFindSelectedPrev()));

  connect(actions->action_tool_bar, SIGNAL(triggered(bool)), m_settings->getActions()->tool_bar, SLOT(setVisible(bool)));
  connect(actions->action_output_bar, SIGNAL(triggered(bool)), m_put_widget, SLOT(setVisible(bool)));
  connect(actions->action_change_split_orientation, SIGNAL(triggered(bool)), SLOT(changeSplitOrientation()));
  connect(actions->action_full_screen, SIGNAL(triggered(bool)), SLOT(fullScreen()));

  connect(actions->action_execute, SIGNAL(triggered(bool)), SLOT(prologExecute()));
  connect(actions->action_abort, SIGNAL(triggered(bool)), SLOT(prologAbort()));
  connect(actions->action_trace, SIGNAL(triggered(bool)), SLOT(prologTracing()));
  connect(actions->action_tools_settings, SIGNAL(triggered(bool)), m_settings, SLOT(show()));

  connect(actions->action_graphics, SIGNAL(triggered(bool)), m_grp, SLOT(showNormal()));
  connect(actions->action_help, SIGNAL(triggered(bool)), SLOT(showHelp()));
  connect(actions->action_about_prolog, SIGNAL(triggered(bool)), SLOT(showAboutProlog()));

  connect(m_input_console_line_edit, SIGNAL(returnPressed()), SLOT(inputConsoleReturn()));
  connect(m_grp, SIGNAL(signalWantClose()), SLOT(prologAbort()));
}
MainWindow::~MainWindow() {
  QSettings qsettings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
  qsettings.setValue("geometry", saveGeometry());

  delete m_grp;
}
void MainWindow::connectEditActions() {
  // Сохраняю connections, чтобы переключить при смене таба
  Actions *actions = m_settings->getActions();
  static QList<QMetaObject::Connection> connections;
  for (auto &it : connections) {
    disconnect(it);
  }
  connections.clear();
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  connections.append(connect(actions->action_cut, SIGNAL(triggered(bool)), editor, SLOT(cut())));
  connections.append(connect(actions->action_copy, SIGNAL(triggered(bool)), editor, SLOT(copy())));
  connections.append(connect(actions->action_paste, SIGNAL(triggered(bool)), editor, SLOT(paste())));
  connections.append(connect(actions->action_undo, SIGNAL(triggered(bool)), editor, SLOT(undo())));
  connections.append(connect(actions->action_redo, SIGNAL(triggered(bool)), editor, SLOT(redo())));
  connections.append(connect(actions->action_comment, SIGNAL(triggered(bool)), editor, SLOT(comment())));
  connections.append(connect(actions->action_uncomment, SIGNAL(triggered(bool)), editor, SLOT(uncomment())));
  connections.append(connect(actions->action_format, SIGNAL(triggered(bool)), editor, SLOT(formatDocument())));
  connections.append(connect(editor, SIGNAL(copyAvailable(bool)), actions->action_cut, SLOT(setEnabled(bool))));
  connections.append(connect(editor, SIGNAL(copyAvailable(bool)), actions->action_copy, SLOT(setEnabled(bool))));
  connections.append(connect(editor, SIGNAL(undoAvailable(bool)), actions->action_undo, SLOT(setEnabled(bool))));
  connections.append(connect(editor, SIGNAL(redoAvailable(bool)), actions->action_redo, SLOT(setEnabled(bool))));
  connections.append(connect(editor, SIGNAL(undoAvailable(bool)), actions->action_save_file, SLOT(setEnabled(bool))));
  connections.append(connect(editor, SIGNAL(fileDropped(QString)), this, SLOT(loadFileToTopTabWidget(QString))));
  connections.append(connect(editor, SIGNAL(positiveWheel()), m_settings, SLOT(increaseFontScale())));
  connections.append(connect(editor, SIGNAL(negativeWheel()), m_settings, SLOT(decreaseFontScale())));
  bool isCopyEnabled = editor->textCursor().selectionEnd() != editor->textCursor().selectionStart();
  actions->action_copy->setEnabled(isCopyEnabled);
  actions->action_cut->setEnabled(isCopyEnabled);
  actions->action_undo->setEnabled(editor->document()->isUndoAvailable());
  actions->action_save_file->setEnabled(editor->document()->isModified());
  actions->action_redo->setEnabled(editor->document()->isRedoAvailable());
}
void MainWindow::keyPressEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Escape) {
    // Закрываю открытые дилоговые окна
    bool any_closed = false;
    if (m_about_program && m_about_program->isVisible()) {
      m_about_program->hide();
      any_closed = true;
    }
    if (m_grp && m_grp->isVisible()) {
      m_grp->hide();
      any_closed = true;
    }
    if (m_help && m_help->isVisible()) {
      m_help->hide();
      any_closed = true;
    }
    if (!any_closed && m_text_finder->isVisible()) {
      editFind();
    }
  }
}
void MainWindow::closeEvent(QCloseEvent *e) {
  if (closeAllFiles()) {
    e->accept();
  } else {
    e->ignore();
  }
}
bool MainWindow::loadFileToTopTabWidget(const QString &filePath) {
  bool revValue = false;
  Editor *editor = m_filepath_2_tab_content->value(filePath).editor;
  if (editor) {
    m_editors->setCurrentIndex(m_editors->indexOf(editor));
    revValue = true;
  } else {
    editor = static_cast<Editor *>(m_editors->currentWidget());
    bool isEmptyTab = editor->isUntitled() && !editor->isTextModified() && editor->document()->isEmpty();
    if (isEmptyTab) {
      if (loadFileToEditor(filePath, editor)) {
        m_editors->setTabText(m_editors->currentIndex(), editor->getFileName());
        m_file_system_watcher->addPath(filePath);
        revValue = true;
      }
    } else {
      editor = new Editor(this);
      if (loadFileToEditor(filePath, editor)) {
        m_editors->setCurrentIndex(m_editors->addTab(editor, editor->getFileName()));
        m_file_system_watcher->addPath(filePath);
        revValue = true;
      } else {
        delete editor;
        editor = nullptr;
      }
    }
  }
  if (editor) {
    if (editor->isUntitled()) {
      m_editors->setTabIcon(m_editors->currentIndex(), QIcon(":/images/file.png"));
    } else {
      QFileIconProvider ip;
      QIcon icon = ip.icon(editor->getFileInfo());
      m_editors->setTabIcon(m_editors->currentIndex(), icon);
    }
  }
  return revValue;
}
bool MainWindow::loadFileToEditor(const QString &filePath, Editor *editor) {
  QFile file(filePath);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, tr("Программа"), tr("Не могу прочитать файл %1:\n%2.").arg(QDir::toNativeSeparators(filePath), file.errorString()));
    return false;
  }
  editor->loadFile(&file);
  (*m_filepath_2_tab_content)[filePath].editor = editor;
  file.close();
  return true;
}
bool MainWindow::safeToFileFromEditor(Editor *editor) {
  if (editor->isUntitled())
    return saveFileAs();
  if (safeToFileFromEditor(editor->getFilePath(), editor))
    return true;
  else
    return saveFileAs();
}
bool MainWindow::safeToFileFromEditor(const QString &filePath, Editor *editor) {
  if (!editor->isUntitled()) {
    m_file_system_watcher->removePath(editor->getFilePath());
  }
  if (m_filepath_2_tab_content->contains(filePath)) {
    m_file_system_watcher->removePath(filePath);
    (*m_filepath_2_tab_content)[filePath].editor->setUntitled();
  }
  QFile file(filePath);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(this, tr("Программа"), tr("Не могу записать файл %1:\n%2.").arg(QDir::toNativeSeparators(filePath), file.errorString()));
    return false;
  }
  if (filePath != editor->getFilePath()) {
    m_filepath_2_tab_content->remove(editor->getFilePath());
    (*m_filepath_2_tab_content)[filePath].editor = editor;
  }
  editor->saveToFile(&file);
  file.close();
  m_file_system_watcher->addPath(filePath);
  m_editors->setTabText(m_editors->currentIndex(), editor->getFileName());
  return true;
}
void MainWindow::updateEditor(Editor *editor) {
  if (editor == nullptr)
    editor = static_cast<Editor *>(m_editors->currentWidget());

  if (m_text_finder->isVisible() && !m_text_finder->line_edit_find->text().isEmpty())
    editor->setSearchString(m_text_finder->line_edit_find->text());
  else
    editor->setSearchString("");

  // QFontMetrics metrics(editor->font());
  // editor->setTabStopDistance(settingsDialog->getTextTabSize() * metrics.horizontalAdvance(' '));

  editor->setCompleter(m_completer);

  static bool blockEnter = false;
  if (blockEnter)
    return;
  blockEnter = true;
  if (editor->isModifiedByAnotherProgram()) {
    editor->setModifiedByAnotherProgram(false);
    if (QFile::exists(editor->getFilePath())) {
      int reply =
        QMessageBox::question(this, tr("Пролог-Д"), tr("Этот файл был изменён другой программой\nПерезагрузить?"), tr("Перезагрузить"), tr("Оставить"));
      if (reply == 0)
        loadFileToEditor(editor->getFilePath(), editor);
    } else {
      int reply = QMessageBox::question(this, tr("Пролог-Д"), tr("Этот файл больше не существует\nОставить?"), tr("Оставить"), tr("Закрыть"));
      if (reply == 1)
        closeFile();
    }
  }
  blockEnter = false;

  editor->setFont(m_settings->getFont());
  m_output_text->setFont(m_settings->getFont());
  m_input_text->setFont(m_settings->getFont());
}
void MainWindow::searchInEditor(Editor *editor, bool backwardFlag) {
  QString searchString = m_text_finder->line_edit_find->text();
  if (!searchString.isEmpty()) {
    if (m_text_finder->isVisible())
      editor->setSearchString(searchString);
    if (backwardFlag) {
      if (!editor->find(searchString, QTextDocument::FindBackward)) {
        QTextCursor currentCursor = editor->textCursor();
        QTextCursor zeroCursor = currentCursor;
        zeroCursor.setPosition(editor->toPlainText().size());
        editor->setTextCursor(zeroCursor);
        if (!editor->find(searchString, QTextDocument::FindBackward)) {
          editor->setTextCursor(currentCursor);
        }
      }
    } else {
      if (!editor->find(searchString)) {
        QTextCursor currentCursor = editor->textCursor();
        QTextCursor zeroCursor = currentCursor;
        zeroCursor.setPosition(0);
        editor->setTextCursor(zeroCursor);
        if (!editor->find(searchString)) {
          editor->setTextCursor(currentCursor);
        }
      }
    }
  }
}

void MainWindow::newFile() {
  Editor *editor = new Editor(this);
  int idx = m_editors->addTab(editor, tr("Без имени"));
  m_editors->setCurrentIndex(idx);
  updateEditor(editor);
  m_editors->setTabIcon(m_editors->currentIndex(), QIcon(":/images/file.png"));
}
void MainWindow::openFile() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Открыть"), QString(), tr("Пролог-Д - файлы (*.pld);;Prolog - файлы (*.pl);;Все файлы (*)"));
  if (!filePath.isEmpty()) {
    loadFileToTopTabWidget(filePath);
    updateEditor();
  }
}
bool MainWindow::saveFile() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  return safeToFileFromEditor(editor);
}
bool MainWindow::saveFileAs() {
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setNameFilter(tr("Пролог-Д - файлы (*.pld);;Prolog - файлы (*.pl);;Все файлы (*)"));
  if (dialog.exec() != QDialog::Accepted)
    return false;
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  auto selected = dialog.selectedFiles();
  if (selected.empty())
    return false;
  return safeToFileFromEditor(selected.first(), editor);
}
bool MainWindow::saveAllFiles() {
  bool isAllSaved = true;
  for (int i = 0, si = m_editors->count(); i < si; i++) {
    Editor *editor = static_cast<Editor *>(m_editors->widget(i));
    if (editor->isModifiedByAnotherProgram()) {
      isAllSaved = false;
      continue;
    }
    if (!safeToFileFromEditor(editor))
      isAllSaved = false;
  }
  return isAllSaved;
}
bool MainWindow::closeFile() {
  return closeEditor(m_editors->currentIndex());
}
bool MainWindow::closeAllFiles() {
  disconnect(m_connection_current_index_changed);
  bool retValue = true;
  while (m_editors->count() > 1) {  // can't be == 0
    if (!closeFile()) {
      retValue = false;
      break;
    }
  }
  if (retValue && !closeFile())
    retValue = false;
  m_connection_current_index_changed = connect(m_editors, SIGNAL(currentChanged(int)), SLOT(setCurrentEditor(int)));
  return retValue;
}
void MainWindow::editFind() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  QString selected = editor->textCursor().selectedText();
  if (m_text_finder->isVisible()) {
    if (!selected.isEmpty() && selected != m_text_finder->line_edit_find->text()) {
      m_text_finder->line_edit_find->setText(selected);
    } else {
      m_text_finder->setVisible(false);
      m_editors->currentWidget()->setFocus();
    }
  } else {
    m_text_finder->setVisible(true);
    if (selected.isEmpty()) {
      m_text_finder->line_edit_find->setFocus();
    } else {
      m_text_finder->line_edit_find->setText(selected);
    }
  }
  m_settings->getActions()->action_find->setChecked(m_text_finder->isVisible());
  if (m_text_finder->isVisible())
    editor->setSearchString(m_text_finder->line_edit_find->text());
  else
    editor->setSearchString("");
}
void MainWindow::editFindNext() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  searchInEditor(editor);
}
void MainWindow::editFindPrev() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  searchInEditor(editor, true);
}
void MainWindow::editFindSelectedNext() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  m_text_finder->line_edit_find->setText(editor->textCursor().selectedText());
  searchInEditor(editor);
}
void MainWindow::editFindSelectedPrev() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  m_text_finder->line_edit_find->setText(editor->textCursor().selectedText());
  searchInEditor(editor, true);
}
void MainWindow::editReplace() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  QString selected = editor->textCursor().selectedText();
  QString searchString = m_text_finder->line_edit_find->text();
  if (selected.isEmpty() || searchString.isEmpty()) {
    return;
  } else if (selected == searchString) {
    editor->textCursor().insertText(m_text_finder->line_edit_replace->text());
    editFindNext();
  }
}
void MainWindow::changeSplitOrientation() {
  if (m_main_widget->orientation() == Qt::Horizontal) {
    m_main_widget->setOrientation(Qt::Vertical);
    m_put_widget->setOrientation(Qt::Horizontal);
  } else {
    m_main_widget->setOrientation(Qt::Horizontal);
    m_put_widget->setOrientation(Qt::Vertical);
  }
}

void MainWindow::fullScreen() {
  setWindowState(windowState() ^ Qt::WindowFullScreen);
}
void MainWindow::prologExecute() {
  if (m_settings->getActions()->action_save_before_execute->isChecked())
    saveFile();

  if (m_grp->isClearOnExec()) {
    m_grp->drawArea()->canvas().clear();
  }

  if (m_prolog_worker) {
    delete m_prolog_worker;
    m_prolog_worker = nullptr;
  }
  if (m_execution_thread) {
    m_execution_thread->terminate();
    m_execution_thread->wait();
    delete m_execution_thread;
    m_execution_thread = nullptr;
  }

  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());

  m_execution_thread = new QThread(this);
  m_prolog_worker = new PrologDWorker(&m_grp->drawArea()->canvas());
  m_prolog_worker->setOutQustion(m_output_print_questions->isChecked());
  m_prolog_worker->moveToThread(m_execution_thread);

  connect(m_prolog_worker, SIGNAL(signalPredicatOut(bool)), SLOT(prologPredicatOut(bool)));
  connect(m_prolog_worker, SIGNAL(signalPrologOut(QString)), SLOT(prologOut(QString)));
  connect(m_prolog_worker, SIGNAL(signalUserOut(QString)), SLOT(prologUserOut(QString)));
  connect(m_prolog_worker, SIGNAL(signalErrorOut(QString)), SLOT(prologErrorOut(QString)));

  connect(this, SIGNAL(executeProlog(QStringList, QStringList)), m_prolog_worker, SLOT(run(QStringList, QStringList)));
  connect(m_prolog_worker, SIGNAL(signalWorkEnded()), SLOT(prologEndWork()));
  connect(m_prolog_worker, SIGNAL(signalWantInput(QString)), SLOT(prologConsoleInput(QString)));
  connect(m_prolog_worker, SIGNAL(signalCanvasUpdated()), m_grp->drawArea(), SLOT(update()));
  connect(m_prolog_worker, SIGNAL(signalCanvasUpdated()), m_grp, SLOT(showNormal()));

  m_settings->getActions()->action_execute->setEnabled(false);
  m_settings->getActions()->action_trace->setEnabled(false);
  m_settings->getActions()->action_abort->setEnabled(true);

  m_output_text->clear();

  m_execution_thread->start();

  if (m_input_spaces_as_separators->isChecked()) {
    // emit executeProlog(editor->toPlainText().split('\n'), m_input_text->toPlainText().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts));
    emit executeProlog(editor->toPlainText().split('\n'), m_input_text->toPlainText().split(QRegularExpression("\\s+"), QString::SkipEmptyParts));
  } else {
    // emit executeProlog(editor->toPlainText().split('\n'), m_input_text->toPlainText().split('\n', Qt::SkipEmptyParts));
    emit executeProlog(editor->toPlainText().split('\n'), m_input_text->toPlainText().split('\n', QString::SkipEmptyParts));
  }
}
void MainWindow::prologAbort() {
  if (m_prolog_worker) {
    delete m_prolog_worker;
    m_prolog_worker = nullptr;
  }
  if (m_execution_thread) {
    m_execution_thread->terminate();
    m_execution_thread->wait();
    delete m_execution_thread;
    m_execution_thread = nullptr;
  }
  m_settings->getActions()->action_execute->setEnabled(true);
  m_settings->getActions()->action_trace->setEnabled(true);
  m_settings->getActions()->action_abort->setEnabled(false);

  m_prolog_want_input = false;
  m_input_console->setTitle("");
  m_input_console->setVisible(false);

  m_grp->hide();
}
void MainWindow::prologTracing() {}
void MainWindow::showHelp() {
  Editor *editor = static_cast<Editor *>(m_editors->currentWidget());
  if (!m_help) {
    m_help = new Help(this);
  }
  m_help->showNormal();
  m_help->setCurosrIfFound(editor->getInCursorWord());
}
void MainWindow::showAboutProlog() {
  if (!m_about_program) {
    m_about_program = new AboutProgram(this);
  }
  m_about_program->showNormal();
}
void MainWindow::setCurrentEditor(int index) {
  if (index == -1)
    return;
  updateEditor();
  connectEditActions();
}
void MainWindow::updateEditor(QString filePath) {
  Editor *editor = m_filepath_2_tab_content->value(filePath).editor;
  if (editor) {
    editor->setModifiedByAnotherProgram();
    updateEditor();
  }
}
bool MainWindow::closeEditor(int index) {
  Editor *editor = static_cast<Editor *>(m_editors->widget(index));
  if (editor->document()->isModified() && !editor->document()->isEmpty()) {
    int reply = QMessageBox::question(this, tr("Пролог-Д"), tr("Этот файл был изменён\nСохранить?"), tr("Сохранить"), tr("Нет"), tr("Отмена"));
    if (reply == 0)
      safeToFileFromEditor(editor);
    else if (reply == 2)
      return false;
  }
  m_filepath_2_tab_content->remove(editor->getFilePath());
  if (!editor->isUntitled()) {
    m_file_system_watcher->removePath(editor->getFilePath());
  }
  delete editor;
  if (m_editors->count() == 0) {
    newFile();
    connectEditActions();
  }
  return true;
}
void MainWindow::prologEndWork() {
  if (m_prolog_worker) {
    delete m_prolog_worker;
    m_prolog_worker = nullptr;
  }
  if (m_execution_thread) {
    m_execution_thread->quit();
    m_execution_thread->wait();
    delete m_execution_thread;
    m_execution_thread = nullptr;
  }
  m_settings->getActions()->action_execute->setEnabled(true);
  m_settings->getActions()->action_trace->setEnabled(true);
  m_settings->getActions()->action_abort->setEnabled(false);
  m_grp->drawArea()->update();

  m_prolog_want_input = false;
  m_input_console->setTitle("");
  m_input_console->setVisible(false);
}
void MainWindow::inputConsoleReturn() {
  if (!m_prolog_want_input)
    return;
  m_prolog_worker->inputStr = m_input_console_line_edit->text();
  m_prolog_want_input = false;
  m_input_console->setTitle("");
  m_input_console->setVisible(false);

  m_prolog_worker->haveInput = true;
}
void MainWindow::prologConsoleInput(QString caption) {
  m_prolog_want_input = true;
  m_input_console->setTitle(caption);
  m_input_console->setVisible(true);
  m_input_console_line_edit->setFocus();
  m_input_console_line_edit->selectAll();
}
void MainWindow::prologPredicatOut(bool value) {
  static QString yesno[] = { "НЕТ", "ДА" };
  m_output_text->appendHtml("<font color=\"#0a22D6\">" + yesno[value]);
  m_lastAppendHtml = true;
}
void MainWindow::prologOut(QString str) {
  m_output_text->appendHtml("<font color=\"#0a22D6\">" + str);
  m_lastAppendHtml = true;
}
void MainWindow::prologUserOut(QString str) {
  if (m_lastAppendHtml) {
    m_output_text->appendHtml("");
  }
  m_output_text->insertPlainText(str);
  m_lastAppendHtml = false;
}
void MainWindow::prologErrorOut(QString str) {
  m_output_text->appendHtml("<font color=\"Crimson\">" + str);
  m_lastAppendHtml = true;
}
