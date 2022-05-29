#include "PrologD.h"

#include "gui/MainWindow.h"
#include <QApplication>
#include <QSettings>
#include <QTranslator>

#include <stdio.h>
#include <stdlib.h>

#include "prlib/scaner.h"
#include "prlib/pstructs.h"
#include "prlib/control.h"
#include "prlib/functions.h"

#include <memory>
#include <string>
#include <charconv>
#include <thread>
#include <chrono>
#include <queue>

#include <QPainter>
#include <QDebug>

const auto PROLOG_COLOR_BLACK = QColor(0, 0, 0).rgb();
const auto PROLOG_COLOR_BLUE = QColor(0, 0, 255).rgb();
const auto PROLOG_COLOR_GEEN = QColor(0, 255, 0).rgb();
const auto PROLOG_COLOR_LBLUE = QColor(0, 191, 255).rgb();
const auto PROLOG_COLOR_BROWN = QColor(150, 75, 0).rgb();
const auto PROLOG_COLOR_VIOLET = QColor(90, 0, 157).rgb();
const auto PROLOG_COLOR_DYELLOW = QColor(150, 150, 0).rgb();
const auto PROLOG_COLOR_GRAY = QColor(170, 170, 170).rgb();
const auto PROLOG_COLOR_DGRAY = QColor(85, 85, 85).rgb();
const auto PROLOG_COLOR_CYAN = QColor(125, 125, 255).rgb();
const auto PROLOG_COLOR_LGREEN = QColor(125, 255, 125).rgb();
const auto PROLOG_COLOR_LCYAN = QColor(50, 230, 255).rgb();
const auto PROLOG_COLOR_RED = QColor(125, 125, 255).rgb();
const auto PROLOG_COLOR_LVIOLET = QColor(159, 20, 255).rgb();
const auto PROLOG_COLOR_YELLOW = QColor(255, 255, 0).rgb();
const auto PROLOG_COLOR_WHITE = QColor(255, 255, 255).rgb();

auto ui2rgb(unsigned int c) {
  switch (c) {
  case 0:  return PROLOG_COLOR_BLACK;   //черный
  case 1:  return PROLOG_COLOR_BLUE;    //синий
  case 2:  return PROLOG_COLOR_GEEN;    //зеленый
  case 3:  return PROLOG_COLOR_LBLUE;   //голубой
  case 4:  return PROLOG_COLOR_BROWN;   //коричневый
  case 5:  return PROLOG_COLOR_VIOLET;  //фиолетовый
  case 6:  return PROLOG_COLOR_DYELLOW; //темно-желтый
  case 7:  return PROLOG_COLOR_GRAY;    //серый
  case 8:  return PROLOG_COLOR_DGRAY;   //стремно-серый
  case 9:  return PROLOG_COLOR_CYAN;    //светло-синий
  case 10: return PROLOG_COLOR_LGREEN;  //светло-зеленый
  case 11: return PROLOG_COLOR_LCYAN;   //светло-голубой
  case 12: return PROLOG_COLOR_RED;     //красный
  case 13: return PROLOG_COLOR_LVIOLET; //сиреневый
  case 14: return PROLOG_COLOR_YELLOW;  //желтый
  }
  return PROLOG_COLOR_WHITE; //белый
}
auto rgb2ui(QRgb color) {
  if (color == PROLOG_COLOR_BLACK  ) return 0;
  if (color == PROLOG_COLOR_BLUE   ) return 1;
  if (color == PROLOG_COLOR_GEEN   ) return 2;
  if (color == PROLOG_COLOR_LBLUE  ) return 3;
  if (color == PROLOG_COLOR_BROWN  ) return 4;
  if (color == PROLOG_COLOR_VIOLET ) return 5;
  if (color == PROLOG_COLOR_DYELLOW) return 6;
  if (color == PROLOG_COLOR_GRAY   ) return 7;
  if (color == PROLOG_COLOR_DGRAY  ) return 8;
  if (color == PROLOG_COLOR_CYAN   ) return 9;
  if (color == PROLOG_COLOR_LGREEN ) return 10;
  if (color == PROLOG_COLOR_LCYAN  ) return 11;
  if (color == PROLOG_COLOR_RED    ) return 12;
  if (color == PROLOG_COLOR_LVIOLET) return 13;
  if (color == PROLOG_COLOR_YELLOW ) return 14;
  return 15;
}

static int Nstr;
static int Ninp;
static PrologDWorker* prd = nullptr;

void out(const char* str) {
  emit prd->signalStdOut(QString::fromLocal8Bit(str));
}
void errout(const char* str) {
  char number[8];
  memset(number, 0, sizeof(char) * 8);
  std::to_chars(number, number + 8, Nstr + 1);
  emit prd->signalStdErr(QString::fromLocal8Bit("<font color=\"Crimson\">Строка №") + QString::fromLocal8Bit(number) + ". " + QString::fromLocal8Bit(str));
}
int InputStringFromDialog(char* buf, size_t size, char *caption) {
  emit prd->signalStdOut("<font color=\"#126799\">" + QString::fromLocal8Bit(caption));
  std::string line;
  while (Ninp < prd->inputList.size() && prd->inputList[Ninp].isEmpty()) ++Ninp;
  if (Ninp < prd->inputList.size()) {
    line = prd->inputList[Ninp].toLocal8Bit().toStdString();
    ++Ninp;
  } else {
    prd->haveInput = false;
    emit prd->signalWantInput(QString::fromLocal8Bit(caption));
    while (!prd->haveInput && prd->EnableRunning) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (prd->inputStr.isEmpty()) {
      prd->EnableRunning = false;
      return 1;
    }
    //out(caption);
    line = prd->inputStr.toLocal8Bit().toStdString();
  }
  if (size <= 0) return 1;
  int to = size - 1;
  if (static_cast<int>(line.size()) < to) to = static_cast<int>(line.size());
  for (int i = 0; i < to; ++i) {
    buf[i] = line[i];
  }
  buf[to] = 0;
  return 0;
}
unsigned int GetPixel(int x, int y) {
  //qDebug() << "GetPixel" << x << y;
  int iw = prd->canvas().image().width();
  int ih = prd->canvas().image().height();
  if (x < 0 || x >= iw || y < 0 || y >=  ih) {
    errout("Запрос за пределы холста");
    return 0;
  }
  return rgb2ui(prd->canvas().image().pixel(x, y));
}
void ClearView(unsigned int c) {
  //qDebug() << "ClearView" << c;
  //emit prd->signalClearView(c);
  prd->canvas().clearView(c);
  // конкретные обновления при масштабировании ведут себя не правильно
  //update(x-w, y-h, x+w, y+h);
  emit prd->signalCanvasUpdated();
}
void Ellipse(int x, int y, int w, int h, unsigned int c) {
  //qDebug() << "Ellipse" << x << y << w*2<<h*2 << c;
  //emit prd->signalEllipse(x-w, y-h, w*2, h*2, c);
  prd->canvas().ellipse(x-w, y-h, w*2, h*2, c);
  emit prd->signalCanvasUpdated();
}
void Rectangle(int x1, int y1, int x2, int y2, unsigned int c) {
  //qDebug() << "Rectangle" << x1 << y1 << x2 << y2 << c;
  //emit prd->signalRectangle(x1, y1, x2, y2, c);
  prd->canvas().rectangle(x1, y1, x2, y2, c);
  emit prd->signalCanvasUpdated();
}
void FloodFill(int x, int y, unsigned int c) {
  //qDebug() << "FloodFill" << x << y << c;
  //emit prd->signalFloodFill(x, y, c);
  int iw = prd->canvas().image().width();
  int ih = prd->canvas().image().height();
  if (x < 0 || x >= iw || y < 0 || y >=  ih) {
    errout("Рисование за пределами холста");
    return;
  }
  prd->canvas().floodFill(x, y, c);
  emit prd->signalCanvasUpdated();
}
void MoveTo_LineTo(int x1, int y1, int x2, int y2, unsigned int c) {
  //qDebug() << "MoveTo_LineTo" << x1 << y1 << x2 << y2 << c;
  //emit prd->signalMoveTo_LineTo(x1, y1, x2, y2, c);
  prd->canvas().line(x1, y1, x2, y2, c);
  emit prd->signalCanvasUpdated();
}
void SetPixel(int x, int y, unsigned int c) {
  //qDebug() << "SetPixel" << x << y << c;
  //emit prd->signalSetPixel(x, y, c);
  int iw = prd->canvas().image().width();
  int ih = prd->canvas().image().height();
  if (x < 0 || x >= iw || y < 0 || y >=  ih) {
    errout("Рисование за пределами холста");
    return;
  }
  prd->canvas().setPixel(x, y, c);
  emit prd->signalCanvasUpdated();
}
void horisontal(int x1, int y1, int x2, int y2) {
  errout("Функционал не реализован");
  //qDebug() << "horisontal" << x1 << y1 << x2 << y2;
}
void vertical(int x1, int y1, int x2, int y2) {
  errout("Функционал не реализован");
  //qDebug() << "vertical" << x1 << y1 << x2 << y2;
}

PrologDWorker::PrologDWorker(CanvasArea *canvas, QObject *parent)
  : QObject (parent)
  , m_canvas(canvas)
{

}
void PrologDWorker::run(const QStringList &program, const QStringList &input) {
  inputList = input;
  prd = this;
  int serr = 0, cerr = 0;
  std::unique_ptr<array> heap;
  std::unique_ptr<TScVar> ScVar;
  std::unique_ptr<TClVar> ClVar;
  try
  { // инициализация переменных, необходимых для работы интерпретатора
      heap = std::make_unique<array>(_maxarray_);
      ScVar = std::make_unique<TScVar>();
      ClVar = std::make_unique<TClVar>();
      ClVar->PrSetting = std::make_unique<TPrSetting>();
  }
  catch (const std::bad_alloc& er) {
      errout(er.what());
      emit signalWorkEnded();
      return;
  }
  catch (const std::runtime_error& er) {
      errout(er.what());
      emit signalWorkEnded();
      return;
  }
  catch (...) {
      errout("Prolog initialization failure");
      emit signalWorkEnded();
      return;
  }
  EnableRunning = true;
  serr = buildin(ScVar.get(), heap.get());
  Ninp = 0;
  for (Nstr = 0; !serr && !cerr && EnableRunning && Nstr < program.size(); Nstr++) {
    // трансляция построчно
    std::string line;
    QString lise = program[Nstr];
    lise.replace('\t', ' ');
    line = lise.toLocal8Bit().toStdString();

    char* p = const_cast<char*>(line.c_str()); // текущая строка
    try
    {
      serr = scaner(p, ScVar.get(), heap.get());

      if (!serr && ScVar->Query && ScVar->EndOfClause) //если конец предложения и вопрос то
      {
        if (m_outQuestion) //вывод вопроса
        {
          emit prd->signalStdOut("<font color=\"#0a22D6\">" + QString::fromLocal8Bit(p));
          //out(p);
        }
        cerr = control(ScVar.get(), ClVar.get(), heap.get(), &EnableRunning);
        ScVar->Query = ScVar->EndOfClause = false;     //на выполнение
      }
    }
    catch (...) {
      errout(const_cast<char*>("Ошибка исполнения"));
      //if (ClVar && ClVar->PrSetting) delete ClVar->PrSetting;
      emit signalWorkEnded();
      return;
    }
  }
  emit signalWorkEnded();
}
void CanvasArea::resize(int w, int h)
{
  QImage old = m_image;
  m_image = QImage(w, h, QImage::Format_RGB32);
  clear();
  int mw = std::min(w, old.width());
  int mh = std::min(h, old.height());
  for (int i = 0; i < mw; ++i) {
    for (int j = 0; j < mh; ++j) {
      m_image.setPixel(i, j, old.pixel(i, j));
    }
  }
}
void CanvasArea::ellipse(int x, int y, int w, int h, unsigned int c)
{
  QPainter painter(&m_image);
  painter.setPen(ui2rgb(c));
  painter.drawEllipse(x, y, w, h);

}
void CanvasArea::floodFill(int x, int y, unsigned int c)
{
  if (x < 0 || y < 0 || x >= m_image.width() || y >= m_image.height()) {
    return;
  }
  std::queue<std::pair<int, int>> q;
  QRgb col = m_image.pixel(x, y);
  QRgb newcol = ui2rgb(c);
  if (col == newcol) return;
  q.push({x, y});
  const int dx[] = {1, -1, 0, 0};
  const int dy[] = {0, 0, 1, -1};
  m_image.setPixel(x, y, newcol);
  while (!q.empty()) {
    auto[cx, cy] = q.front(); q.pop();
    for (int i = 0; i < 4; ++i) {
      int nx = cx + dx[i], ny = cy + dy[i];
      if (nx >= 0 && nx < m_image.width() && ny >= 0 && ny < m_image.height()) {
        if (m_image.pixel(nx, ny) == col) {
          q.push({nx, ny});
          m_image.setPixel(nx, ny, newcol);
        }
      }
    }
  }
}
void CanvasArea::setPixel(int x, int y, unsigned int c) {
  m_image.setPixel(x, y, ui2rgb(c));
}
void CanvasArea::clearView(unsigned int c)
{
  m_image.fill(ui2rgb(c));
}
void CanvasArea::rectangle(int x1, int y1, int x2, int y2, unsigned int c)
{
  QPainter painter(&m_image);
  painter.setPen(ui2rgb(c));
  if (x1 > x2) std::swap(x1, x2);
  if (y2 > y1) std::swap(y2, y1);
  painter.drawRect(x1, y1, x2 - x1, y2 - y1);
}
void CanvasArea::line(int x1, int y1, int x2, int y2, unsigned int c)
{
  QPainter painter(&m_image);
  painter.setPen(ui2rgb(c));
  painter.drawLine(x1, y1, x2, y2);
}
void CanvasArea::clear()
{
  m_image.fill(ui2rgb(15));
}




int main(int argc, char *argv[])
{
  QCoreApplication::setApplicationName("Prolog-D");
  QCoreApplication::setOrganizationName("Celyabinsk SU");
  QApplication app(argc, argv);

  setlocale(LC_ALL, "ru");

  // loadTranslation
  QSettings settings (qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
  const QString ru_language = settings.value("language", "Russian").toString();
  QTranslator translator;
  bool tt = translator.load(":/" + ru_language);
  if (tt) {
    qApp->installTranslator(&translator);
  }

  MainWindow wnd;
  wnd.setWindowIcon(QIcon(":/images/prolog1.png"));
  wnd.show();
  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      wnd.loadFileToTopTabWidget(argv[i]);
    }
  }
  return app.exec();
}
