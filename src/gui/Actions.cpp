#include "Actions.h"
#include <QMenuBar>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QSlider>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

Actions::Actions()
  : menu_bar(new QMenuBar)
  , tool_bar(new QToolBar)
  , menu_file(new QMenu)
  , action_new_file(new QAction)
  , action_open_file(new QAction)
  , action_save_file(new QAction)
  , action_save_file_as(new QAction)
  , action_save_all_files(new QAction)
  , action_close_file(new QAction)
  , action_close_all_files(new QAction)
  , action_exit(new QAction)
  , menu_edit(new QMenu)
  , action_cut(new QAction)
  , action_copy(new QAction)
  , action_paste(new QAction)
  , action_undo(new QAction)
  , action_redo(new QAction)
  , action_comment(new QAction)
  , action_uncomment(new QAction)
  , action_format(new QAction)
  , menu_search(new QMenu)
  , action_find(new QAction)
  , action_find_next(new QAction)
  , action_find_prev(new QAction)
  , action_find_selected_next(new QAction)
  , action_find_selected_prev(new QAction)
  , menu_view(new QMenu)
  , action_tool_bar(new QAction)
  , action_output_bar(new QAction)
  , action_change_split_orientation(new QAction)
  , action_full_screen(new QAction)
  , action_graphics(new QAction)
  , menu_execution(new QMenu)
  , action_execute(new QAction)
  , action_abort(new QAction)
  , action_trace(new QAction)
  , action_save_before_execute(new QAction)
  , action_tools_settings(new QAction)
  , menu_help(new QMenu)
  , action_help(new QAction)
  , action_about_prolog(new QAction) {
  tool_bar->layout()->setContentsMargins(0, 0, 0, 0);
  tool_bar->layout()->setSpacing(0);
  tool_bar->setFloatable(false);
  tool_bar->setMovable(false);

  initializeActions();
  menu_file->setTitle(QObject::tr("&Файл"));
  menu_edit->setTitle(QObject::tr("&Правка"));
  menu_search->setTitle(QObject::tr("&Поиск"));
  menu_view->setTitle(QObject::tr("&Вид"));
  menu_execution->setTitle(QObject::tr("&Исполнение"));
  menu_help->setTitle(QObject::tr("&Справка"));

  menu_file->addAction(action_new_file);
  menu_file->addAction(action_open_file);
  menu_file->addAction(action_save_file);
  menu_file->addAction(action_save_file_as);
  menu_file->addAction(action_save_all_files);
  menu_file->addAction(action_close_file);
  menu_file->addAction(action_close_all_files);
  menu_file->addSeparator();
  menu_file->addAction(action_exit);

  menu_edit->addAction(action_cut);
  menu_edit->addAction(action_copy);
  menu_edit->addAction(action_paste);
  menu_edit->addSeparator();
  menu_edit->addAction(action_undo);
  menu_edit->addAction(action_redo);
  menu_edit->addSeparator();
  menu_edit->addAction(action_comment);
  menu_edit->addAction(action_uncomment);
  menu_edit->addAction(action_format);

  menu_search->addAction(action_find);
  menu_search->addAction(action_find_next);
  menu_search->addAction(action_find_prev);
  menu_search->addAction(action_find_selected_next);
  menu_search->addAction(action_find_selected_prev);

  menu_view->addSeparator();
  menu_view->addAction(action_tool_bar);
  menu_view->addAction(action_output_bar);
  menu_view->addSeparator();
  menu_view->addAction(action_change_split_orientation);
  menu_view->addSeparator();
  menu_view->addAction(action_full_screen);
  menu_view->addSeparator();
  menu_view->addAction(action_tools_settings);
  menu_view->addAction(action_graphics);

  menu_execution->addAction(action_execute);
  menu_execution->addAction(action_abort);
  // menuExecution->addAction(actionTrace);
  menu_execution->addSeparator();
  menu_execution->addAction(action_save_before_execute);

  menu_help->addAction(action_help);
  menu_help->addAction(action_about_prolog);

  menu_bar->addMenu(menu_file);
  menu_bar->addMenu(menu_edit);
  menu_bar->addMenu(menu_search);
  menu_bar->addMenu(menu_view);
  menu_bar->addMenu(menu_execution);
  menu_bar->addMenu(menu_help);

  tool_bar->addSeparator();
  tool_bar->addAction(action_new_file);
  tool_bar->addAction(action_open_file);
  tool_bar->addAction(action_save_file);
  tool_bar->addSeparator();
  tool_bar->addAction(action_abort);
  tool_bar->addAction(action_execute);
  // toolBar->addAction(actionTrace);
  tool_bar->addSeparator();
  tool_bar->addAction(action_graphics);
  tool_bar->addAction(action_help);
  tool_bar->addAction(action_about_prolog);
  tool_bar->addSeparator();
  tool_bar->addAction(action_comment);
  tool_bar->addAction(action_uncomment);
  tool_bar->addAction(action_format);
  tool_bar->addSeparator();
  tool_bar->addAction(action_tools_settings);

  QWidget *text_scale_widget = new QWidget;
  QVBoxLayout *text_scale_layout = new QVBoxLayout;
  font_scale_slider = new QSlider(Qt::Vertical);
  QSizePolicy policy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
  font_scale_slider->setSizePolicy(policy);
  font_scale_slider->setTickInterval(1);
  font_scale_slider->setTickPosition(QSlider::TicksBothSides);
  // slider->setSingleStep(1);
  font_scale_slider->setRange(0, 8);
  font_scale_slider->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  font_scale_label = new QLabel("100%");
  text_scale_layout->addWidget(font_scale_slider);
  text_scale_layout->addWidget(font_scale_label);
  text_scale_widget->setLayout(text_scale_layout);
  tool_bar->addWidget(text_scale_widget);
  tool_bar->addSeparator();
}
void Actions::initializeActions() {
  initializeFileActions();
  initializeEditActions();
  initializeSearchActions();
  initializeViewActions();
  initializeExecutionActions();
  initializeHelpActions();
}
void Actions::initializeFileActions() {
  action_new_file->setText(QObject::tr("&Новый"));
  action_new_file->setToolTip(QObject::tr("Создать новый файл") + " (Ctrl+N)");
  action_new_file->setIcon(QIcon(":/images/file-new.png"));
  action_new_file->setShortcut(Qt::CTRL + Qt::Key_N);

  action_open_file->setText(QObject::tr("&Открыть..."));
  action_open_file->setToolTip(QObject::tr("Открыть существующий файл") + " (Ctrl+O)");
  action_open_file->setIcon(QIcon(":/images/file-open.png"));
  action_open_file->setShortcut(Qt::CTRL + Qt::Key_O);

  action_save_file->setText(QObject::tr("&Сохранить"));
  action_save_file->setToolTip(QObject::tr("Сохранить текущий файл") + " (Ctrl+S)");
  action_save_file->setIcon(QIcon(":/images/file-save.png"));
  action_save_file->setShortcut(Qt::CTRL + Qt::Key_S);

  action_save_file_as->setText(QObject::tr("&Сохранить как..."));
  action_save_file_as->setIcon(QIcon(":/images/file-save.png"));

  action_save_all_files->setText(QObject::tr("&Сохранить все"));
  action_save_all_files->setIcon(QIcon(":/images/file-save.png"));

  action_close_file->setText(QObject::tr("&Закрыть"));
  action_close_file->setIcon(QIcon(":/images/file-close.png"));

  action_close_all_files->setText(QObject::tr("&Закрыть все"));
  action_close_all_files->setIcon(QIcon(":/images/file-close-all.png"));

  action_exit->setText(QObject::tr("&Выход"));
  action_exit->setStatusTip(QObject::tr("Выйти из приложения"));
  action_exit->setIcon(QIcon(":/images/file-exit.png"));
  action_exit->setShortcut(Qt::ALT + Qt::Key_F4);
}
void Actions::initializeEditActions() {
  action_cut->setText(QObject::tr("&Вырезать"));
  action_cut->setIcon(QIcon(":/images/edit-cut.png"));
  action_cut->setShortcut(Qt::CTRL + Qt::Key_X);

  action_copy->setText(QObject::tr("&Копировать"));
  action_copy->setIcon(QIcon(":/images/edit-copy.png"));
  action_copy->setShortcut(Qt::CTRL + Qt::Key_C);

  action_paste->setText(QObject::tr("&Вставить"));
  action_paste->setIcon(QIcon(":/images/edit-paste.png"));
  action_paste->setShortcut(Qt::CTRL + Qt::Key_V);

  action_undo->setText(QObject::tr("&Отменить"));
  action_undo->setIcon(QIcon(":/images/edit-undo.png"));
  action_undo->setShortcut(Qt::CTRL + Qt::Key_Z);

  action_redo->setText(QObject::tr("&Повторить"));
  action_redo->setIcon(QIcon(":/images/edit-redo.png"));
  action_redo->setShortcut(Qt::CTRL + Qt::Key_Y);

  action_comment->setText(QObject::tr("&Закомментировать"));
  action_comment->setToolTip(QObject::tr("Закомментировать выделенные строки") + " (Ctrl+K)");
  action_comment->setIcon(QIcon(":/images/edit-comment.png"));
  action_comment->setShortcut(Qt::CTRL + Qt::Key_K);

  action_uncomment->setText(QObject::tr("&Раскомментировать"));
  action_uncomment->setToolTip(QObject::tr("Раскомментировать выделенные строки") + " (Ctrl+U)");
  action_uncomment->setIcon(QIcon(":/images/edit-uncomment.png"));
  action_uncomment->setShortcut(Qt::CTRL + Qt::Key_U);

  action_format->setText(QObject::tr("&Форматировать документ"));
  action_format->setToolTip(QObject::tr("Форматировать текущий документ") + " (Ctrl+I)");
  action_format->setIcon(QIcon(":/images/edit-format.png"));
  action_format->setShortcut(Qt::CTRL + Qt::Key_I);
}
void Actions::initializeSearchActions() {
  action_find->setText(QObject::tr("&Поиск/Замена"));
  action_find->setIcon(QIcon(":/images/search-find.png"));
  action_find->setShortcut(Qt::CTRL + Qt::Key_F);
  action_find->setCheckable(true);

  action_find_next->setText(QObject::tr("&Найти следующее"));
  action_find_next->setIcon(QIcon(":/images/search-find-next.png"));
  action_find_next->setShortcut(Qt::Key_F3);

  action_find_prev->setText(QObject::tr("&Найти предыдущее"));
  action_find_prev->setIcon(QIcon(":/images/search-find-prev.png"));
  action_find_prev->setShortcut(Qt::SHIFT + Qt::Key_F3);

  action_find_selected_next->setText(QObject::tr("&Найти выделенное следующее"));
  action_find_selected_next->setIcon(QIcon(":/images/search-find-selected-next.png"));
  action_find_selected_next->setShortcut(Qt::CTRL + Qt::Key_F3);

  action_find_selected_prev->setText(QObject::tr("&Найти выделенное предыдущее"));
  action_find_selected_prev->setIcon(QIcon(":/images/search-find-selected-prev.png"));
  action_find_selected_prev->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_F3);
}
void Actions::initializeViewActions() {
  action_tool_bar->setText(QObject::tr("&Панель инструментов"));
  action_tool_bar->setCheckable(true);

  action_output_bar->setText(QObject::tr("&Панель вывода"));
  action_output_bar->setShortcut(Qt::Key_F4);
  action_output_bar->setCheckable(true);

  action_change_split_orientation->setText(QObject::tr("&Сменить ориентацию сплита"));
  action_change_split_orientation->setShortcut(Qt::CTRL + Qt::Key_F4);

  action_full_screen->setText(QObject::tr("&Полный экран"));
  action_full_screen->setIcon(QIcon(":/images/view-full-screen.png"));
  action_full_screen->setShortcut(Qt::Key_F11);
  action_full_screen->setCheckable(true);

  action_graphics->setText(QObject::tr("&Графика"));
  action_graphics->setIcon(QIcon(":/images/view-graphics.png"));
  // actionGraphics->setCheckable(true);
}
void Actions::initializeExecutionActions() {
  action_execute->setText(QObject::tr("&Запуск"));
  action_execute->setToolTip(QObject::tr("Запустить интерпретатор") + " (F5)");
  action_execute->setIcon(QIcon(":/images/execution-execute.png"));
  action_execute->setShortcut(Qt::Key_F5);

  action_abort->setText(QObject::tr("&Прервать"));
  action_abort->setToolTip(QObject::tr("Прервать выполнение") + " (F6)");
  action_abort->setIcon(QIcon(":/images/execution-abort.png"));
  action_abort->setShortcut(Qt::Key_F6);

  action_trace->setText(QObject::tr("&Трассировка"));
  action_trace->setToolTip(QObject::tr("Трассировка") + " (F7)");
  action_trace->setIcon(QIcon(":/images/execution-tracing.png"));
  action_trace->setShortcut(Qt::Key_F7);

  action_save_before_execute->setText(QObject::tr("&Сохранять перед запуском"));
  action_save_before_execute->setCheckable(true);

  action_tools_settings->setText(QObject::tr("&Настройки..."));
  action_tools_settings->setToolTip(QObject::tr("Настройки") + " (F9)");
  action_tools_settings->setIcon(QIcon(":/images/tools-settings.png"));
  action_tools_settings->setShortcut(Qt::Key_F9);
}
void Actions::initializeHelpActions() {
  action_help->setText(QObject::tr("&Помощь..."));
  action_help->setToolTip(QObject::tr("Помощь") + " (F1)");
  action_help->setIcon(QIcon(":/images/help.png"));
  action_help->setShortcut(Qt::Key_F1);

  action_about_prolog->setText(QObject::tr("&О Прологе-Д..."));
  action_about_prolog->setToolTip(QObject::tr("О Прологе-Д") + " (F2)");
  action_about_prolog->setIcon(QIcon(":/images/prolog1.png"));
  action_about_prolog->setShortcut(Qt::Key_F2);
}

void Actions::setActionsFont(const QFont &font) {
  action_new_file->setFont(font);
  action_open_file->setFont(font);
  action_save_file->setFont(font);
  action_save_file_as->setFont(font);
  action_save_all_files->setFont(font);
  action_close_file->setFont(font);
  action_close_all_files->setFont(font);
  action_exit->setFont(font);
  action_cut->setFont(font);
  action_copy->setFont(font);
  action_paste->setFont(font);
  action_undo->setFont(font);
  action_redo->setFont(font);
  action_find->setFont(font);
  action_find_next->setFont(font);
  action_find_prev->setFont(font);
  action_find_selected_next->setFont(font);
  action_find_selected_prev->setFont(font);
  action_tool_bar->setFont(font);
  action_output_bar->setFont(font);
  action_full_screen->setFont(font);
  action_tools_settings->setFont(font);
  action_execute->setFont(font);
  action_abort->setFont(font);
  action_trace->setFont(font);
  action_save_before_execute->setFont(font);
  action_about_prolog->setFont(font);
  menu_file->setFont(font);
  menu_edit->setFont(font);
  menu_search->setFont(font);
  menu_view->setFont(font);
  menu_execution->setFont(font);
  menu_help->setFont(font);
}
