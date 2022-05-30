#include "Settings.h"
#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QSpinBox>
#include <QFontComboBox>
#include <QTabWidget>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QBitArray>
#include <QTranslator>
#include <QAction>

#include <QDebug>

//static const char* TOOL_BAR_VIS = "toolBarVisible";
//static const char* OUTPUT_BAR_VIS = "outputBarVisible";
static const char* SAVE_BEFORE = "saveBeforeExec";
static const char* FONT_FAMILY = "fontFamily";
static const char* FONT_SIZE = "fontSize";
static const char* FONT_SCALE = "fontScale";
constexpr int FONT_SIZE_DEF = 10;
constexpr int FONT_SIZE_LO = 6;
constexpr int FONT_SIZE_HI = 72;
constexpr int FONT_SCALE_DEF = 2;
constexpr int FONT_SCALE_LO = 0;
constexpr int FONT_SCALE_HI = 8;

constexpr int SPIN_BOX_WIDTH = 40;

template<class T> T clamp(T val, T lo, T hi) {
  if (val < lo) return lo;
  if (val > hi) return hi;
  return val;
}

Settings::Settings(QWidget *parent)
  : QDialog (parent)
  //, m_settings (new QSettings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat, this))
  , m_font_family (new QFontComboBox)
  , m_font_size (new QSpinBox)
{
  setFixedSize(360, 240);
  setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

  // Загрузить настройки
  QSettings m_settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
  m_font_size->setValue(clamp(m_settings.value(FONT_SIZE, FONT_SIZE_DEF).toInt(), FONT_SIZE_LO, FONT_SIZE_HI));
  m_actions.font_scale_slider->setValue(clamp(m_settings.value(FONT_SCALE, FONT_SCALE_DEF).toInt(), FONT_SCALE_LO, FONT_SCALE_HI));

  QFont font; font.setFamily(m_settings.value(FONT_FAMILY, "Cascadia Mono").toString());
  m_font_family->setFontFilters(QFontComboBox::FontFilter::MonospacedFonts);
  if (m_font_family->count() == 0) {
    m_font_family->setFontFilters(QFontComboBox::FontFilter::AllFonts);
  }
  m_font_family->setCurrentFont(font);

  //m_actions.action_tool_bar->setChecked (m_settings.value(TOOL_BAR_VIS, true).toBool());
  //m_actions.action_output_bar->setChecked (m_settings.value(OUTPUT_BAR_VIS, true).toBool());
  m_actions.action_tool_bar->setChecked(true);
  m_actions.action_output_bar->setChecked(true);
  m_actions.action_save_before_execute->setChecked (m_settings.value(SAVE_BEFORE, false).toBool());
  m_actions.font_scale_label->setText(QString::number(getFontScale()) + "%");
  //QString orientation = settings->value("splitOrientation", "Horizontal").toString();
  //
  QVBoxLayout *main_layout = new QVBoxLayout;

  QGroupBox *font_settings_group = new QGroupBox (tr("Font"));
  QHBoxLayout *font_settings_layout = new QHBoxLayout;
  font_settings_layout->addWidget(m_font_family);
  font_settings_layout->addWidget(new QLabel(tr("Size: ")));
  font_settings_layout->addWidget(m_font_size);
  font_settings_layout->setAlignment(Qt::AlignmentFlag::AlignLeft);
  font_settings_group->setLayout(font_settings_layout);
  m_font_size->setRange(FONT_SIZE_LO, FONT_SIZE_HI);
  m_font_size->setFixedWidth(SPIN_BOX_WIDTH);

  QHBoxLayout *ok_close_layout = new QHBoxLayout;
  QPushButton *ok_button = new QPushButton (tr("Ok"));
  //QPushButton *close_button = new QPushButton (tr("Cencel"));
  //ok_close_layout->addSpacing(120);
  ok_close_layout->addSpacing(240);
  ok_close_layout->addWidget(ok_button);
  //ok_close_layout->addWidget(close_button);

  QVBoxLayout *permain_layout = new QVBoxLayout;
  permain_layout->setAlignment(Qt::AlignmentFlag::AlignTop);
  permain_layout->addWidget(font_settings_group);
  main_layout->addLayout(permain_layout);
  main_layout->addLayout(ok_close_layout);
  setLayout(main_layout);

  //connect(close_button, SIGNAL(clicked(bool)), SLOT(close()));
  connect(ok_button, SIGNAL(clicked(bool)), SLOT(okButtonPressed()));

  connect(m_actions.font_scale_slider, SIGNAL(valueChanged(int)), this, SLOT(fontScaleValueChanged(int)));
  setWindowTitle(tr("Settings"));
}
Settings::~Settings()
{
  QSettings m_settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
  m_settings.setValue(FONT_FAMILY, getFont().family());
  m_settings.setValue(FONT_SIZE, m_font_size->value());
  m_settings.setValue(FONT_SCALE, m_actions.font_scale_slider->value());

//  if (mainWidget->orientation() == Qt::Horizontal)
//    m_settings->setValue("splitOrientation", "Horizontal");
//  else
//    m_settings->setValue("splitOrientation", "Vertical");
  //m_settings->setValue("geometry", saveGeometry());
  //m_settings.setValue(TOOL_BAR_VIS, m_actions.action_tool_bar->isChecked());
  //m_settings.setValue(OUTPUT_BAR_VIS, m_actions.action_output_bar->isChecked());
  m_settings.setValue(SAVE_BEFORE, m_actions.action_save_before_execute->isChecked());
}
void Settings::increaseFontScale()
{
  int value = m_actions.font_scale_slider->value();
  if (value < FONT_SCALE_HI) m_actions.font_scale_slider->setValue(value + 1);
}
void Settings::decreaseFontScale()
{
  int value = m_actions.font_scale_slider->value();
  if (value > FONT_SCALE_LO) m_actions.font_scale_slider->setValue(value - 1);
}
void Settings::fontScaleValueChanged(int)
{
  m_actions.font_scale_label->setText(QString::number(getFontScale()) + "%");
  emit signalTextEditorSettingsChanged();
}
void Settings::okButtonPressed ()
{
  emit signalTextEditorSettingsChanged ();
  close();
}
Actions* Settings::getActions() {
  return &m_actions;
}
QFont Settings::getFont () const
{
  QFont font = m_font_family->currentFont();
  font.setPointSizeF(getFontScale() / 100. * m_font_size->value());
  return font;
}
int Settings::getFontScale () const
{
  static int scales[9]{50, 75, 100, 125, 150, 175, 200, 250, 300};
  return scales[m_actions.font_scale_slider->value()];
}
