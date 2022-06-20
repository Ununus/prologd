#ifndef SETTINGS_H
#define SETTINGS_H

#include "Actions.h"
#include <QDialog>

class QSettings;
class QTabWidget;
class QListWidget;
class QCheckBox;
class QSpinBox;
class QFontComboBox;
class QListWidgetItem;

class Settings : public QDialog {
  Q_OBJECT
public:
  Settings(QWidget *parent);
  ~Settings();

  Actions *getActions();
  QFont getFont() const;
  int getFontScale() const;

signals:
  void signalTextEditorSettingsChanged();
public slots:
  void increaseFontScale();
  void decreaseFontScale();
private slots:
  void okButtonPressed();
  void fontScaleValueChanged(int value);

private:
  Actions m_actions;
  // QSettings     *m_settings;

  QFontComboBox *m_font_family;
  QSpinBox *m_font_size;
  // int textTabSize;
};

#endif  // SETTINGS_H
