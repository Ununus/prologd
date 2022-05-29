#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include <QDialog>
#include <QImage>
#include "../PrologD.h"

class QVBoxLayout;
class QSpinBox;
class QPushButton;

class GraphicsWidget : public QWidget
{
  Q_OBJECT
public:
  explicit GraphicsWidget(QWidget *parent = nullptr);
  CanvasArea& canvas() { return m_canvas; }
protected:
  void paintEvent(QPaintEvent *) override;
private:
  CanvasArea m_canvas;
};

class GraphicsDialog : public QDialog {
  Q_OBJECT
public:
  explicit GraphicsDialog(QWidget *parent = nullptr);
  ~GraphicsDialog();
  GraphicsWidget *drawArea();
  bool isClearOnExec() const;
protected slots:
  void okClicked(bool);
  void acClicked(bool);
private:
  QVBoxLayout *m_main_layout;
  GraphicsWidget *m_draw_area;
  QSpinBox *m_input_width;
  QSpinBox *m_input_height;
  QPushButton *m_clear_when_exec;
};

#endif // GRAPHICSWIDGET_H
