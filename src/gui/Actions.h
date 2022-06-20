#ifndef ACTIONS_H
#define ACTIONS_H

class QMenuBar;
class QToolBar;
class QMenu;
class QAction;
class QSlider;
class QLabel;

#include <QFont>

class Actions {
public:
  Actions();
  void setActionsFont(const QFont &font);
  void initializeActions();
  void initializeFileActions();
  void initializeEditActions();
  void initializeSearchActions();
  void initializeViewActions();
  void initializeExecutionActions();
  void initializeHelpActions();

  QMenuBar *menu_bar;
  QToolBar *tool_bar;

  QMenu *menu_file;
  QAction *action_new_file;
  QAction *action_open_file;
  QAction *action_save_file;
  QAction *action_save_file_as;
  QAction *action_save_all_files;
  QAction *action_close_file;
  QAction *action_close_all_files;
  QAction *action_exit;

  QMenu *menu_edit;
  QAction *action_cut;
  QAction *action_copy;
  QAction *action_paste;
  QAction *action_undo;
  QAction *action_redo;
  QAction *action_comment;
  QAction *action_uncomment;
  QAction *action_format;

  QMenu *menu_search;
  QAction *action_find;
  QAction *action_find_next;
  QAction *action_find_prev;
  QAction *action_find_selected_next;
  QAction *action_find_selected_prev;

  QMenu *menu_view;
  QAction *action_tool_bar;
  QAction *action_output_bar;
  QAction *action_change_split_orientation;
  QAction *action_full_screen;
  QAction *action_graphics;

  QMenu *menu_execution;
  QAction *action_execute;
  QAction *action_abort;
  QAction *action_trace;
  QAction *action_save_before_execute;
  QAction *action_tools_settings;

  QMenu *menu_help;
  QAction *action_help;
  QAction *action_about_prolog;
  QAction *action_about_qt;

  QSlider *font_scale_slider;
  QLabel *font_scale_label;
};

#endif  // ACTIONS_H
