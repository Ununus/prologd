#ifndef PROLOGD_H
#define PROLOGD_H

#include <QObject>
#include <QStringList>
#include <QImage>

class CanvasArea
{
public:
  QImage& image() { return m_image; }
  void resize(int w, int h);

  void ellipse(int x, int y, int w, int h, unsigned int c);
  void floodFill(int x, int y, unsigned int c);
  void setPixel(int x, int y, unsigned int c);
  void clearView(unsigned int c);
  void rectangle(int x1, int y1, int x2, int y2, unsigned int c);
  void line(int x1, int y1, int x2, int y2, unsigned int c);
  void clear();

private:
  QImage m_image;
};

class PrologDWorker : public QObject
{
  Q_OBJECT
public:
  PrologDWorker (CanvasArea *canvas, QObject *parent = nullptr);
  void setOutQustion(bool f) { m_outQuestion = f; }
  bool EnableRunning{ true };

  CanvasArea& canvas() { return *m_canvas; }

  QStringList inputList;
  QString inputStr;
  bool haveInput{ false };
public slots:
  void run(const QStringList &program, const QStringList &input);
signals:
  void signalStdOut(QString);
  void signalStdErr(QString);
  void signalWorkEnded();
  void signalWantInput(QString);
  void signalShowGWindow();
  void signalCanvasUpdated();
private:
  bool m_outQuestion{ true };
  CanvasArea *m_canvas;
};

#endif // PROLOGD_H
