#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include <QDialog>
#include <QImage>
#include "../PrologD.h"

class QVBoxLayout;
class QSpinBox;
class QPushButton;
class QCheckBox;

class GraphicsCanvas : public QWidget {
  Q_OBJECT
public:
  explicit GraphicsCanvas(QWidget *parent = nullptr);
  CanvasArea &canvas() {
    return m_canvas;
  }

protected:
  void paintEvent(QPaintEvent *) override;

private:
  CanvasArea m_canvas;
};

class GraphicsWidget : public QWidget {
  Q_OBJECT
public:
  explicit GraphicsWidget(QWidget *parent = nullptr);
  ~GraphicsWidget();
  GraphicsCanvas *drawArea();
  bool isClearOnExec() const;
signals:
  void signalWantClose();
protected slots:
  void okClicked(bool);
  void acClicked(bool);

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  QVBoxLayout *m_main_layout;
  GraphicsCanvas *m_draw_area;
  QSpinBox *m_input_width;
  QSpinBox *m_input_height;
  QCheckBox *m_clear_when_exec;
};

#endif  // GRAPHICSWIDGET_H
