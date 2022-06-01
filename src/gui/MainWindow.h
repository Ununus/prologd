#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../PrologD.h"
#include "Editor.h"
#include "Actions.h"
#include "TextFinder.h"
#include "AboutProgram.h"
#include "Settings.h"
#include "Graphics.h"

class QSplitter;
class QFileSystemWatcher;
class QSettings;
class QInputDialog;

// Структуры вынес, чтобы добавить сюда свой input для editor
struct TabContent {
  Editor* editor { nullptr };
};

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
public slots:
  bool loadFileToTopTabWidget (const QString &filePath);
protected:
  bool loadFileToEditor (const QString &filePath, Editor* Editor);
  bool safeToFileFromEditor (const QString &filePath, Editor* Editor);
  bool safeToFileFromEditor (Editor* Editor);
  void keyPressEvent(QKeyEvent *e) override;
  void closeEvent(QCloseEvent *event) override;
private:
  void searchInEditor (Editor *editor, bool backwardFlag = false);

  void connectEditActions();
private slots:
  void newFile();
  void openFile();
  bool saveFile();
  bool saveFileAs();
  bool saveAllFiles();
  bool closeFile();
  bool closeAllFiles();

  void editFind();
  void editFindNext();
  void editFindPrev();
  void editFindSelectedNext();
  void editFindSelectedPrev();
  void editReplace();

  void changeSplitOrientation();

  void prologExecute();
  void prologAbort();
  void prologTracing();
  void prologEndWork();

  void fullScreen();

  void showHelp();
  void showAboutProlog();

  void updateEditor(Editor *editor = nullptr);
  void updateEditor (QString filePath);
  void setCurrentEditor (int index);
  bool closeEditor(int index);

  void prologInputBox(QString);
signals:
  void executeProlog(QStringList, QStringList);
  void inputReq(QString);
private:
  QSplitter* m_main_widget;
  QSplitter* m_put_widget;
  QTabWidget *m_editors;
  PlainTextEditWithLineNumberArea *m_output_text;
  PlainTextEditWithLineNumberArea *m_input_text;
  QInputDialog *m_input_dialog;
  QFileSystemWatcher *m_file_system_watcher;
  QMap<QString, TabContent> *m_filepath_2_tab_content;
  QFont *m_editor_font;
  TextFinder *m_text_finder;
  AboutProgram *m_about_program;
  Help *m_help;
  Settings *m_settings;
  QCompleter *m_completer;
  QThread *m_execution_thread;
  PrologDWorker* m_prolog_worker;
  QMetaObject::Connection m_connection_current_index_changed;
  QCheckBox *m_input_spaces_as_separators;
  QCheckBox *m_output_print_questions;
  GraphicsDialog *m_grp;
};

#endif // MAINWINDOW_H
