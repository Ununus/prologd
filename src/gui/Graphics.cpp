#include "Graphics.h"
#include <QPixmap>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QSettings>
#include <QApplication>
#include <QThread>
#include <queue>

#include <QDebug>

GraphicsWidget::GraphicsWidget(QWidget *parent)
  : QWidget(parent)
  , m_main_layout(new QVBoxLayout)
  , m_draw_area(new GraphicsCanvas)
  , m_input_width(new QSpinBox)
  , m_input_height(new QSpinBox)
  , m_clear_when_exec(new QCheckBox("Очищать при запуске")) {
  setWindowFlags((windowFlags() ^ Qt::WindowContextHelpButtonHint) | Qt::WindowMinMaxButtonsHint);
  setWindowTitle("Графика");

  QHBoxLayout *control_layout = new QHBoxLayout;
  QPushButton *clear_pb = new QPushButton("Очистить");
  QPushButton *accept_pb = new QPushButton("Принять");
  QLabel *size_lb = new QLabel("Размер холста:");

  m_input_width->setRange(1, 3840);
  m_input_height->setRange(1, 2160);
  QSettings s(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat, this);
  m_input_width->setValue(s.value("graphAreaWidth", 640).toInt());
  m_input_height->setValue(s.value("graphAreaHeight", 350).toInt());

  control_layout->addWidget(clear_pb);
  control_layout->addWidget(m_clear_when_exec);
  control_layout->addWidget(size_lb);
  control_layout->addWidget(m_input_width);
  control_layout->addWidget(m_input_height);
  control_layout->addWidget(accept_pb);
  control_layout->setAlignment(Qt::AlignmentFlag::AlignLeft);

  m_main_layout->addLayout(control_layout);
  m_main_layout->addWidget(m_draw_area);

  setLayout(m_main_layout);

  m_clear_when_exec->setCheckable(true);
  m_clear_when_exec->setChecked(true);
  // m_draw_area->setFixedSize(m_input_width->value(), m_input_height->value());
  resize(800, 600);
  m_draw_area->resize(m_input_width->value(), m_input_height->value());
  m_draw_area->canvas().resize(m_input_width->value(), m_input_height->value());
  connect(clear_pb, SIGNAL(clicked(bool)), SLOT(okClicked(bool)));
  connect(accept_pb, SIGNAL(clicked(bool)), SLOT(acClicked(bool)));
}
bool GraphicsWidget::isClearOnExec() const {
  return m_clear_when_exec->isChecked();
}
void GraphicsWidget::okClicked(bool) {
  m_draw_area->canvas().clear();
  m_draw_area->update();
}
void GraphicsWidget::acClicked(bool) {
  m_main_layout->removeWidget(m_draw_area);
  m_draw_area->resize(m_input_width->value(), m_input_height->value());
  m_draw_area->canvas().resize(m_input_width->value(), m_input_height->value());
  m_main_layout->addWidget(m_draw_area);
}
void GraphicsWidget::closeEvent(QCloseEvent *event) {
  emit signalWantClose();
}
GraphicsWidget::~GraphicsWidget() {
  QSettings s(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat, this);
  s.setValue("graphAreaWidth", m_input_width->value());
  s.setValue("graphAreaHeight", m_input_height->value());
}

GraphicsCanvas *GraphicsWidget::drawArea() {
  return m_draw_area;
}

GraphicsCanvas::GraphicsCanvas(QWidget *parent)
  : QWidget{ parent } {}

void GraphicsCanvas::paintEvent(QPaintEvent *) {
  // qDebug() << "Paint Event";
  QPainter painter(this);
  // painter.setBrush(brush);
  // painter.setPen(pen);
  // painter.drawPath(path);
  QPixmap pixmap;
  if (!pixmap.convertFromImage(m_canvas.image())) {
    qDebug() << "Error";
  }
  painter.drawPixmap(0, 0, width(), height(), pixmap, 0, 0, m_canvas.image().width(), m_canvas.image().height());
}
