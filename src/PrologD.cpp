// Encoding CP-1251
#include "PrologD.h"

#include <QApplication>
#include <QSettings>
#include <QTextCodec>
#include <QTranslator>
#include "gui/MainWindow.h"

#include <stdio.h>
#include <stdlib.h>

#include "prlib/control.h"
#include "prlib/err.h"
#include "prlib/functions.h"
#include "prlib/pstructs.h"
#include "prlib/scaner.h"
#include <charconv>

#include <chrono>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#include <QPainter>
#include <QDebug>

const QRgb PROLOG_COLOR_BLACK = QColor(0, 0, 0).rgb();
const QRgb PROLOG_COLOR_BLUE = QColor(0, 0, 255).rgb();
const QRgb PROLOG_COLOR_GEEN = QColor(0, 255, 0).rgb();
const QRgb PROLOG_COLOR_LBLUE = QColor(0, 191, 255).rgb();
const QRgb PROLOG_COLOR_BROWN = QColor(150, 75, 0).rgb();
const QRgb PROLOG_COLOR_VIOLET = QColor(90, 0, 157).rgb();
const QRgb PROLOG_COLOR_DYELLOW = QColor(150, 150, 0).rgb();
const QRgb PROLOG_COLOR_GRAY = QColor(170, 170, 170).rgb();
const QRgb PROLOG_COLOR_DGRAY = QColor(85, 85, 85).rgb();
const QRgb PROLOG_COLOR_CYAN = QColor(125, 125, 255).rgb();
const QRgb PROLOG_COLOR_LGREEN = QColor(125, 255, 125).rgb();
const QRgb PROLOG_COLOR_LCYAN = QColor(50, 230, 255).rgb();
const QRgb PROLOG_COLOR_RED = QColor(125, 125, 255).rgb();
const QRgb PROLOG_COLOR_LVIOLET = QColor(159, 20, 255).rgb();
const QRgb PROLOG_COLOR_YELLOW = QColor(255, 255, 0).rgb();
const QRgb PROLOG_COLOR_WHITE = QColor(255, 255, 255).rgb();

QRgb ui2rgb(unsigned int c) {
  switch (c) {
  case 0: return PROLOG_COLOR_BLACK;     //������
  case 1: return PROLOG_COLOR_BLUE;      //�����
  case 2: return PROLOG_COLOR_GEEN;      //�������
  case 3: return PROLOG_COLOR_LBLUE;     //�������
  case 4: return PROLOG_COLOR_BROWN;     //����������
  case 5: return PROLOG_COLOR_VIOLET;    //����������
  case 6: return PROLOG_COLOR_DYELLOW;   //�����-������
  case 7: return PROLOG_COLOR_GRAY;      //�����
  case 8: return PROLOG_COLOR_DGRAY;     //�������-�����
  case 9: return PROLOG_COLOR_CYAN;      //������-�����
  case 10: return PROLOG_COLOR_LGREEN;   //������-�������
  case 11: return PROLOG_COLOR_LCYAN;    //������-�������
  case 12: return PROLOG_COLOR_RED;      //�������
  case 13: return PROLOG_COLOR_LVIOLET;  //���������
  case 14: return PROLOG_COLOR_YELLOW;   //������
  }
  return PROLOG_COLOR_WHITE;  //�����
}
unsigned int rgb2ui(QRgb color) {
  if (color == PROLOG_COLOR_BLACK)
    return 0;
  if (color == PROLOG_COLOR_BLUE)
    return 1;
  if (color == PROLOG_COLOR_GEEN)
    return 2;
  if (color == PROLOG_COLOR_LBLUE)
    return 3;
  if (color == PROLOG_COLOR_BROWN)
    return 4;
  if (color == PROLOG_COLOR_VIOLET)
    return 5;
  if (color == PROLOG_COLOR_DYELLOW)
    return 6;
  if (color == PROLOG_COLOR_GRAY)
    return 7;
  if (color == PROLOG_COLOR_DGRAY)
    return 8;
  if (color == PROLOG_COLOR_CYAN)
    return 9;
  if (color == PROLOG_COLOR_LGREEN)
    return 10;
  if (color == PROLOG_COLOR_LCYAN)
    return 11;
  if (color == PROLOG_COLOR_RED)
    return 12;
  if (color == PROLOG_COLOR_LVIOLET)
    return 13;
  if (color == PROLOG_COLOR_YELLOW)
    return 14;
  return 15;
}

static int Nstr;
static int Ninp;
static PrologDWorker *prd = nullptr;

QString decode_cp1251_to_utf8(const char *str) {
  std::string res;
  for (const char *c = str; *c; ++c) {
    if (*c == '�') {
      res.push_back(-47);
      res.push_back(-111);
    } else if (*c == '�') {
      res.push_back(-48);
      res.push_back(-127);
    } else if (*c >= '�' && *c <= '�') {
      auto v = static_cast<unsigned char>(*c - '�') + 1040u;
      unsigned char c1 = 192u + (v >> 6u);
      unsigned char c2 = (v & 63u) + 128u;
      res.push_back(c1);
      res.push_back(c2);
    } else {
      res.push_back(static_cast<unsigned char>(*c & 127u));
    }
  }
  return QString(res.c_str());
}

std::string decode_utf8_to_cp1251(QString qstr) {
  std::string str{ qstr.toUtf8().toStdString() };
  if (str.empty())
    return str;
  size_t si = 0;
  for (size_t i = 0; i < str.size();) {
    if (i + 1 < str.size() && str[i] == -48 && str[i + 1] == -127) {
      str[si++] = '�';
      i += 2;
    } else if (i + 1 < str.size() && str[i] == -47 && str[i + 1] == -111) {
      str[si++] = '�';
      i += 2;
    } else if ((str[i] & 128u) == 0) {
      str[si++] = str[i++];
    } else if ((str[i] & 224u) == 192u && i + 1 != str.size()) {
      str[si++] = ((str[i] ^ 192u) << 6u) + (str[i + 1] & 63u) - 1040u - 64u;
      i += 2;
    } else {
      throw std::runtime_error("Decoding error");
    }
  }
  str.resize(si);
  return str;
}
void prdout(bool value) {
  emit prd->signalPredicatOut(value);
}
void pldout(const char *str) {
  emit prd->signalPrologOut(decode_cp1251_to_utf8(str));
}
void usrout(const char *str) {
  emit prd->signalUserOut(decode_cp1251_to_utf8(str));
}
void errout(const char *str) {
  char number[8];
  memset(number, 0, sizeof(char) * 8);
  std::to_chars(number, number + 8, Nstr + 1);
  emit prd->signalErrorOut(decode_cp1251_to_utf8("������ #") + decode_cp1251_to_utf8(number) + ". " + decode_cp1251_to_utf8(str));
}
int InputStringFromDialog(char *buf, size_t size, const char *caption) {
  // emit prd->signalStdOut("<font color=\"#126799\">" + decode_cp1251_to_utf8(caption));
  std::string line;
  while (Ninp < prd->inputList.size() && prd->inputList[Ninp].isEmpty()) {
    ++Ninp;
  }
  if (Ninp < prd->inputList.size()) {
    line = decode_utf8_to_cp1251(prd->inputList[Ninp]);
    ++Ninp;
  } else {
    prd->haveInput = false;
    emit prd->signalWantInput(decode_cp1251_to_utf8(caption));
    while (!prd->haveInput && prd->EnableRunning) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (prd->inputStr.isEmpty()) {
      prd->EnableRunning = false;
      return 1;
    }
    // out(caption);
    line = decode_utf8_to_cp1251(prd->inputStr);
  }
  if (size <= 0) {
    return 1;
  }
  int to = size - 1, lnsz = static_cast<int>(line.size());
  if (lnsz < to) {
    to = lnsz;
  }
  for (int i = 0; i < to; ++i) {
    buf[i] = line[i];
  }
  buf[to] = 0;
  return 0;
}
unsigned int GetPixel(IntegerType x, IntegerType y) {
  // qDebug() << "GetPixel" << x << y;
  int iw = prd->canvas().image().width();
  int ih = prd->canvas().image().height();
  if (x < 0 || x >= iw || y < 0 || y >= ih) {
    errout("������ �� ������� ������");
    return 0;
  }
  return rgb2ui(prd->canvas().image().pixel(x, y));
}
void ClearView(unsigned int c) {
  // qDebug() << "ClearView" << c;
  // emit prd->signalClearView(c);
  prd->canvas().clearView(c);
  // ���������� ���������� ��� ��������������� ����� ���� �� ���������
  // update(x-w, y-h, x+w, y+h);
  emit prd->signalCanvasUpdated();
}
void Ellipse(IntegerType x, IntegerType y, IntegerType w, IntegerType h, unsigned int c) {
  // qDebug() << "Ellipse" << x << y << w*2<<h*2 << c;
  // emit prd->signalEllipse(x-w, y-h, w*2, h*2, c);
  prd->canvas().ellipse(x - w, y - h, w * 2, h * 2, c);
  emit prd->signalCanvasUpdated();
}
void Rectangle(IntegerType x1, IntegerType y1, IntegerType x2, IntegerType y2, unsigned int c) {
  // qDebug() << "Rectangle" << x1 << y1 << x2 << y2 << c;
  // emit prd->signalRectangle(x1, y1, x2, y2, c);
  prd->canvas().rectangle(x1, y1, x2, y2, c);
  emit prd->signalCanvasUpdated();
}
void FloodFill(IntegerType x, IntegerType y, unsigned int c) {
  // qDebug() << "FloodFill" << x << y << c;
  // emit prd->signalFloodFill(x, y, c);
  int iw = prd->canvas().image().width();
  int ih = prd->canvas().image().height();
  if (x < 0 || x >= iw || y < 0 || y >= ih) {
    errout("��������� �� ��������� ������");
    return;
  }
  prd->canvas().floodFill(x, y, c);
  emit prd->signalCanvasUpdated();
}
void MoveTo_LineTo(IntegerType x1, IntegerType y1, IntegerType x2, IntegerType y2, unsigned int c) {
  // qDebug() << "MoveTo_LineTo" << x1 << y1 << x2 << y2 << c;
  // emit prd->signalMoveTo_LineTo(x1, y1, x2, y2, c);
  prd->canvas().line(x1, y1, x2, y2, c);
  emit prd->signalCanvasUpdated();
}
void SetPixel(IntegerType x, IntegerType y, unsigned int c) {
  // qDebug() << "SetPixel" << x << y << c;
  // emit prd->signalSetPixel(x, y, c);
  int iw = prd->canvas().image().width();
  int ih = prd->canvas().image().height();
  if (x < 0 || x >= iw || y < 0 || y >= ih) {
    errout("��������� �� ��������� ������");
    return;
  }
  prd->canvas().setPixel(x, y, c);
  emit prd->signalCanvasUpdated();
}
void horisontal(IntegerType x1, IntegerType y1, IntegerType x2, IntegerType y2) {
  errout("���������� �� ����������");
  // qDebug() << "horisontal" << x1 << y1 << x2 << y2;
}
void vertical(IntegerType x1, IntegerType y1, IntegerType x2, IntegerType y2) {
  errout("���������� �� ����������");
  // qDebug() << "vertical" << x1 << y1 << x2 << y2;
}

PrologDWorker::PrologDWorker(CanvasArea *canvas, QObject *parent)
  : QObject(parent)
  , m_canvas(canvas) {}
void PrologDWorker::run(const QStringList &program, const QStringList &input) try {
  inputList = input;
  prd = this;
  ErrorCode serr = ErrorCode::NoErrors, cerr = ErrorCode::NoErrors;
  std::unique_ptr<array> heap;
  std::unique_ptr<TScVar> ScVar;
  std::unique_ptr<TClVar> ClVar;

  heap = std::unique_ptr<array>(new array(_maxarray_));
  ScVar = std::unique_ptr<TScVar>(new TScVar);
  ClVar = std::unique_ptr<TClVar>(new TClVar);
  ClVar->PrSetting = std::unique_ptr<TPrSetting>(new TPrSetting);
  EnableRunning = true;
  serr = buildin(ScVar.get(), heap.get());
  if (serr != ErrorCode::NoErrors) {
    throw std::runtime_error(GetPrErrText(serr));
  }
  Ninp = 0;
  for (Nstr = 0; EnableRunning && Nstr < program.size(); Nstr++) {
    // ���������� ���������
    QString lise = program[Nstr];
    lise.replace('\t', ' ');
    std::string line = decode_utf8_to_cp1251(lise);

    char *p = const_cast<char *>(line.c_str());  // ������� ������

    serr = scaner(p, ScVar.get(), heap.get());
    if (serr != ErrorCode::NoErrors) {
      throw std::runtime_error(GetPrErrText(serr));
    }
    if (ScVar->Query && ScVar->EndOfClause)  //���� ����� ����������� � ������ ��
    {
      if (m_outQuestion)  //����� �������
      {
        pldout(p);
      }
      cerr = control(ScVar.get(), ClVar.get(), heap.get(), &EnableRunning);
      if (cerr != ErrorCode::NoErrors) {
        throw std::runtime_error(GetPrErrText(cerr));
      }
      ScVar->Query = ScVar->EndOfClause = false;  //�� ����������
    }
  }
  emit signalWorkEnded();
} catch (const std::bad_alloc &er) {
  errout(er.what());
  emit signalWorkEnded();
  return;
} catch (const std::runtime_error &er) {
  errout(er.what());
  emit signalWorkEnded();
  return;
} catch (...) {
  errout("Prolog failure");
  emit signalWorkEnded();
  return;
}
void CanvasArea::resize(int w, int h) {
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
void CanvasArea::ellipse(int x, int y, int w, int h, unsigned int c) {
  QPainter painter(&m_image);
  painter.setPen(ui2rgb(c));
  painter.drawEllipse(x, y, w, h);
}
void CanvasArea::floodFill(int x, int y, unsigned int c) {
  if (x < 0 || y < 0 || x >= m_image.width() || y >= m_image.height()) {
    return;
  }
  std::queue<std::pair<int, int>> q;
  QRgb col = m_image.pixel(x, y);
  QRgb newcol = ui2rgb(c);
  if (col == newcol)
    return;
  q.push({ x, y });
  const int dx[] = { 1, -1, 0, 0 };
  const int dy[] = { 0, 0, 1, -1 };
  m_image.setPixel(x, y, newcol);
  while (!q.empty()) {
    int cx, cy;
    std::tie(cx, cy) = q.front();
    q.pop();
    for (int i = 0; i < 4; ++i) {
      int nx = cx + dx[i], ny = cy + dy[i];
      if (nx >= 0 && nx < m_image.width() && ny >= 0 && ny < m_image.height()) {
        if (m_image.pixel(nx, ny) == col) {
          q.push({ nx, ny });
          m_image.setPixel(nx, ny, newcol);
        }
      }
    }
  }
}
void CanvasArea::setPixel(int x, int y, unsigned int c) {
  m_image.setPixel(x, y, ui2rgb(c));
}
void CanvasArea::clearView(unsigned int c) {
  m_image.fill(ui2rgb(c));
}
void CanvasArea::rectangle(int x1, int y1, int x2, int y2, unsigned int c) {
  QPainter painter(&m_image);
  painter.setPen(ui2rgb(c));
  if (x1 > x2)
    std::swap(x1, x2);
  if (y2 > y1)
    std::swap(y2, y1);
  painter.drawRect(x1, y1, x2 - x1, y2 - y1);
}
void CanvasArea::line(int x1, int y1, int x2, int y2, unsigned int c) {
  QPainter painter(&m_image);
  painter.setPen(ui2rgb(c));
  painter.drawLine(x1, y1, x2, y2);
}
void CanvasArea::clear() {
  m_image.fill(ui2rgb(15));
}

int main(int argc, char *argv[]) {
  QCoreApplication::setApplicationName("Prolog-D");
  QCoreApplication::setOrganizationName("Celyabinsk SU");
  QApplication app(argc, argv);

  // setlocale(LC_ALL, "ru");
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

  // loadTranslation
  QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
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
