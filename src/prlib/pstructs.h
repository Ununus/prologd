#pragma once
#include <fstream>
#include <string>
#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <type_traits>
#include "err.h"
#include "pdefs.h"
#include <boost/multiprecision/cpp_int.hpp>

constexpr size_t kInitialBptr = 4 * 1024;
constexpr size_t kInitialGptr = 4 * 150;
constexpr size_t kInitialVar = 4 * 512;
constexpr size_t kInitialSymbol = 4 * 2048;

constexpr size_t kInitialHeapSize = 16 * 16384;
constexpr size_t kInitialStackSize = 16 * 4096;
constexpr size_t kInitialUniBufSize = 4 * 100;
constexpr size_t kInitialBptSize = 4 * 256;

using FloatType = double;
using IntegerType = boost::multiprecision::cpp_int;

// используется в control
enum class PredicateState { Error = 1, PrepereNewTarget = 2, Yes = 3, ControlStep = 4, No = 5, Builtin = 6 };

// предок всех структур
struct baserecord {
  size_t ident;
  baserecord(size_t ident)
    : ident(ident) {}
  const char *name() {
    switch (ident) {
    case isvar: return "ПЕРМЕННАЯ";
    case issymbol: return "СИМВ. КОНСТАНТА";
    case isinteger: return "ЦЕЛОЕ";
    case isfloat: return "ВЕЩЕСТВЕННОЕ";
    case isexpression: return "ВЫРАЖЕНИЕ";
    case isunknown: return "АНОНИМКА";
    case isemptylist: return "[]";
    case islist: return "СПИСОК";
    case isfunction: return "ФУНКЦИЯ";
    case isstring: return "СТРОКА";
    case isclause: return "ПРАВИЛО";
    case isclauseq: return "ПРАВИЛО_";
    case iscut: return "CUT";
    default: return "НЕИЗВЕСТНЫЙ";
    }
  }
  virtual ~baserecord() {}
};

// строковая константа
struct recordsconst final : public baserecord {
  // char *ptr;    // указатель в текст на символьное представление
  size_t ptrsymb;  // индекс в heap симв представления
  size_t length;   // длина
  size_t begin;    // индекс в heap на первое предл. где она стоит предикатом
  recordsconst(size_t PTRsymb, size_t LENGTH, size_t BEGIN = 0)
    : baserecord(issymbol)
    , ptrsymb(PTRsymb)
    , length(LENGTH)
    , begin(BEGIN) {}
};

// "строка"
struct recordstring final : public baserecord {
  // char *ptr;    // указатель на строку в тексте
  size_t ptrsymb;  // индекс в heap симв представления
  size_t length;   // длина строки не считая " и '\0'
  size_t begin;    // введен для совместимости с recordsconst всегда =0;
  recordstring(size_t PTRsymb, size_t LENGTH, size_t BEGIN = 0)
    : baserecord(isstring)
    , ptrsymb(PTRsymb)
    , length(LENGTH)
    , begin(BEGIN) {}
};

// структура записи переменной
struct recordvar final : public baserecord {
  // char *ptr;    // указатель на текст представл переменной
  size_t ptrsymb;  // индекс в heap симв представления
  size_t length;   // длина переменной
  size_t num;      // номер переменный в предложении начиная с 0
  recordvar(size_t PTRsymb, size_t LENGTH, size_t NUM = 0)
    : baserecord(isvar)
    , ptrsymb(PTRsymb)
    , length(LENGTH)
    , num(NUM) {}
};

struct recordinteger final : public baserecord {
  IntegerType value;
  recordinteger(IntegerType VALUE)
    : baserecord(isinteger)
    , value(VALUE) {}
};

struct recordfloat final : public baserecord {
  FloatType value;
  recordfloat(FloatType VALUE)
    : baserecord(isfloat)
    , value(VALUE) {}
};

struct recordunknown final : public baserecord {
  recordunknown()
    : baserecord(isunknown) {}
};

struct recordcut final : public baserecord {
  size_t func;
  recordcut(size_t hpcut)
    : baserecord(iscut)
    , func(hpcut) {}
};

struct recordemptylist final : public baserecord {
  recordemptylist()
    : baserecord(isemptylist) {}
};

struct recordexpression final : public baserecord {
  size_t length;   // длина записи
  size_t precord;  // индекс польской записи
  recordexpression(size_t LENGTH, size_t PRECORD)
    : baserecord(isexpression)
    , length(LENGTH)
    , precord(PRECORD) {}
};

struct recordfunction final : public baserecord {
  size_t narg;    // число аргументов в функции
  size_t func;    // указатель на функц символ
  size_t ptrarg;  // индекс массива с аргументами
  recordfunction(size_t NARG, size_t FUNC, size_t PTRARG)
    : baserecord(isfunction)
    , narg(NARG)
    , func(FUNC)
    , ptrarg(PTRARG) {}
};

struct recordlist final : public baserecord {
  size_t head;  // указатель на голову
  size_t link;  // указатель на хвост
  explicit recordlist(size_t HEAD, size_t LINK = 0)
    : baserecord(islist)
    , head(HEAD)
    , link(LINK) {}
};

struct recordclause final : public baserecord {
  size_t next;       // инекс в массиве heap следующего одноименного предложения
  size_t nvars;      // число переменных в предложении
  size_t head;       // индекс в heap (на issymbol???) голову предложения
  size_t ptrtarget;  // индекс начала целей
  recordclause(size_t IDENT, size_t NEXT, size_t NVARS, size_t HEAD, size_t PTRTARGET)
    : baserecord(IDENT)
    , next(NEXT)
    , nvars(NVARS)
    , head(HEAD)
    , ptrtarget(PTRTARGET) {}
};
//-----------------------------------------------
// массив для трансляции и исполнения программы
// пролога описания функций в main()
struct array {
protected:
  // unsigned char *heaps;  // указатель на массив
  std::vector<unsigned char> vheap;

public:
  size_t freeheap;  // кол-во эл-тов до разбора программы
  size_t last;      // индекс массива указывающий на первый свободный эл
  size_t query;     // индекс предложения - вопроса.
  template<class T>
  size_t append(const T &record, size_t count = 1);  // добавление в конец

  array(size_t SIZE = kInitialHeapSize);
  void clear();
  void cleanUp(size_t idx);
  ~array();
  baserecord *GetPbaserecord(size_t index);
  recordsconst *GetPrecordsconst(size_t index);
  recordstring *GetPrecordstring(size_t index);
  recordvar *GetPrecordvar(size_t index);
  recordinteger *GetPrecordinteger(size_t index);
  recordfloat *GetPrecordfloat(size_t index);
  recordunknown *GetPrecordunknown(size_t index);
  recordcut *GetPrecordcut(size_t index);
  recordemptylist *GetPrecordemptylist(size_t index);
  recordexpression *GetPrecordexpression(size_t index);
  recordfunction *GetPrecordfunction(size_t index);
  recordlist *GetPrecordlist(size_t index);
  recordclause *GetPrecordclause(size_t index);
  size_t *GetPunsigned(size_t index);
  char *GetPchar(size_t index);

  size_t ipnclause;    // для newclause
  size_t iptclause;    // для trylause
  size_t iphclause;    // для head
  size_t ipaclause;    // ук на структ для atomp
  size_t ipacltarget;  // указатель на цели предложения aclause
  size_t iptcltarget;
  size_t ipncltarget;

  std::vector<size_t> recordintegersInHeap;
};

template<class T>
size_t array::append(const T &record, size_t count) {
  auto baklast = last;
  auto sz = sizeof(record) * count;
  if (last + sz + 1 > vheap.size()) {
    vheap.resize(std::max(vheap.size() * 2, last + sz + 2));
  }
  for (size_t i = 0; i < count; ++i) {
    new (vheap.data() + last + i * sizeof(record)) T(record);
    if constexpr (std::is_same<T, recordinteger>()) {
      recordintegersInHeap.emplace_back(last + i * sizeof(record));
    }
  }
  last += sz;
  return baklast;
  // return heaps + last;
}

// структура для scaner
struct TScVar {
  // size_t *buf;
  // size_t *goal;
  // size_t *tat;   // текстовое представление констант
  // size_t *tvar;  // таблица переменных
  std::vector<size_t> vbuf;
  std::vector<size_t> vgoal;
  std::vector<size_t> vtat;   // текстовое представление констант
  std::vector<size_t> vtvar;  // таблица переменных
  size_t bptr;
  size_t gptr;
  size_t nosymbol;
  size_t novar;  // первое свободное место в tvar
  bool right;
  bool EndOfClause;
  bool Query;
  size_t exprip;   // нужна ли здесь ? !!!
  size_t hpunkn;   // положенме анонимки в buf
  size_t hpempty;  // положение пустого списка

  TScVar();
  ~TScVar();
  void Init();
  void Clear();
};

struct TPrSetting {
  std::ifstream in;
  std::ofstream out;
  std::string name_in_file;
  std::string name_out_file;
  bool Trace{ false };
};

// структуры для хранения данных - программы
struct TClVar {
  PredicateState stat;
  std::string outBuff;
  size_t vmaxstack;
  std::vector<size_t> vst_con;
  std::vector<size_t> vst_vr1;
  std::vector<size_t> vst_vr2;
  std::vector<size_t> vst_trail;
  std::vector<size_t> vbf;  // массив стека унификации
  size_t varqu;             // кол-во вопросов

  size_t newclause;  // индекс в массиве указывающий на предложения
  size_t nclause;    // индекс для newclause
  size_t aclause;    //            atomp
  size_t hclause;    //            head
  size_t tclause;    //            tryclause
  size_t svptr;      //
  size_t oldsvptr;   //
  size_t scptr;      // вершина стека управления
  size_t frame2;     // указатель к кадру с стеке переменных
  size_t frame1;     //
  size_t parent;     // указатель на родительскую среду в стеке управления
  size_t bipr;       // указатель на функтор предиката
  size_t head;       // место записи в куче об исполняемой цели
  size_t oldtptr;    // вершина стека следа до исполнения
  size_t tptr;       // вершина стека следа

  ErrorCode err{ ErrorCode::NoErrors };  // ошибка
  bool ntro;
  size_t atomp;  // указатель на цель
  size_t tryclause;
  bool ex, flag;
  bool quiet{ false };  // Для предиката ТИХО

  // для unify
  size_t bp, term1, term2, fr1, fr2, numb;

  recordsconst *precordsconst;
  recordvar *precordvar;
  recordinteger *precordinteger;
  recordfloat *precordfloat;
  recordstring *precordstring;
  recordunknown *precordunknown;
  recordemptylist *precordemtylist;
  recordexpression *precordexpression;
  recordfunction *precordfunction;
  recordlist *precordlist;
  recordclause *precordclause;
  recordcut *precordcut;

  std::vector<size_t> vBPT;  // для вывода пролога
  size_t ibpt{};             // указатель в массиве BPT

  std::unique_ptr<TPrSetting> PrSetting{};

  TClVar();
  ~TClVar();
  void Init();
  void Clear();
};

inline std::string toString(const FloatType &value) {
  auto str = std::to_string(value);
  while (str.size() > 1 && str[str.size() - 2] != '.' && str.back() == '0') {
    str.pop_back();
  }
  return str;
}

// индексы в массиве heap   указатели на фреймы переменных

/* место mod в куче */
constexpr size_t hpmod = 0 + sizeof("mod") - 1;

/* место ЛОЖЬ в куче */
constexpr size_t hpfail = hpmod + sizeof(recordsconst) + sizeof("ЛОЖЬ") - 1;

/* место ТРАССА в куче */
constexpr size_t hptrace = hpfail + sizeof(recordsconst) + sizeof("ТРАССА") - 1;

/* место НЕТ_ТРАССЫ в куче */
constexpr size_t hpnottr = hptrace + sizeof(recordsconst) + sizeof("НЕТ_ТРАССЫ") - 1;

/* место ! в куче */
constexpr size_t hpcut = hpnottr + sizeof(recordsconst) + sizeof("!") - 1;

/* место ВЫП в куче */
constexpr size_t hpcall = hpcut + sizeof(recordsconst) + sizeof("ВЫП") - 1;

/* место ВВОДСИМВ в куче */
constexpr size_t hprdsym = hpcall + sizeof(recordsconst) + sizeof("ВВОДСИМВ") - 1;

// место ВВОДСТР
constexpr size_t hprdstr = hprdsym + sizeof(recordsconst) + sizeof("ВВОДСТР") - 1;

/* место ВВОДЦЕЛ в куче */
constexpr size_t hprdint = hprdstr + sizeof(recordsconst) + sizeof("ВВОДЦЕЛ") - 1;

/* место ВВОДВЕЩ в куче */
constexpr size_t hprdfloat = hprdint + sizeof(recordsconst) + sizeof("ВВОДВЕЩ") - 1;

/* место ВВКОД в куче */
constexpr size_t hpiocod = hprdfloat + sizeof(recordsconst) + sizeof("ВВКОД") - 1;

/* место ЧТЕНИЕ_ИЗ в куче */
constexpr size_t hpsee = hpiocod + sizeof(recordsconst) + sizeof("ЧТЕНИЕ_ИЗ") - 1;

/* место ЗАПИСЬ_В в куче */
constexpr size_t hptell = hpsee + sizeof(recordsconst) + sizeof("ЗАПИСЬ_В") - 1;

// место ПЕР в куче
constexpr size_t hpvar = hptell + sizeof(recordsconst) + sizeof("ПЕР") - 1;

// место ЦЕЛ в куче
constexpr size_t hpint = hpvar + sizeof(recordsconst) + sizeof("ЦЕЛ") - 1;

// место ВЕЩ в куче
constexpr size_t hpfloat = hpint + sizeof(recordsconst) + sizeof("ВЕЩ") - 1;

/* место СИМВ в куче */
constexpr size_t hpsym = hpfloat + sizeof(recordsconst) + sizeof("СИМВ") - 1;

/* место СПИСОК в куче */
constexpr size_t hplst = hpsym + sizeof(recordsconst) + sizeof("СПИСОК") - 1;

/* место ВЫВОД в куче */
constexpr size_t hpout = hplst + sizeof(recordsconst) + sizeof("ВЫВОД") - 1;

/* место БОЛЬШЕ в куче */
constexpr size_t hpgt = hpout + sizeof(recordsconst) + sizeof("БОЛЬШЕ") - 1;

/* место СТРЦЕЛ в куче */
constexpr size_t hpstint = hpgt + sizeof(recordsconst) + sizeof("СТРЦЕЛ") - 1;

/* место СТРВЕЩ в куче */
constexpr size_t hpstfloat = hpstint + sizeof(recordsconst) + sizeof("СТРВЕЩ") - 1;

/* место СТРСПИС в куче */
constexpr size_t hpstlst = hpstfloat + sizeof(recordsconst) + sizeof("СТРСПИС") - 1;

/* место ЦЕЛВЕЩ в куче */
constexpr size_t hpintfloat = hpstlst + sizeof(recordsconst) + sizeof("ЦЕЛВЕЩ") - 1;

/* место БУКВА в куче */
constexpr size_t hplettr = hpintfloat + sizeof(recordsconst) + sizeof("БУКВА") - 1;

/* место ЦИФРА в куче */
constexpr size_t hpdigit = hplettr + sizeof(recordsconst) + sizeof("ЦИФРА") - 1;

/* место ТЕРМ в куче */
constexpr size_t hpterm = hpdigit + sizeof(recordsconst) + sizeof("ТЕРМ") - 1;

/* место УДАЛЕНИЕ в куче */
constexpr size_t hpdel = hpterm + sizeof(recordsconst) + sizeof("УДАЛЕНИЕ") - 1;

/* место СКОЛЬКО в куче */
constexpr size_t hpskol = hpdel + sizeof(recordsconst) + sizeof("СКОЛЬКО") - 1;

/* место ТОЧКА в куче */
constexpr size_t hpset = hpskol + sizeof(recordsconst) + sizeof("ТОЧКА") - 1;

/* место СЦЕП в куче */
constexpr size_t hpapp = hpset + sizeof(recordsconst) + sizeof("СЦЕП") - 1;

/* место ДОБ в куче */
constexpr size_t hpassrt = hpapp + sizeof(recordsconst) + sizeof("ДОБ") - 1;

/* место УМНОЖЕНИЕ в куче */
constexpr size_t hpmul = hpassrt + sizeof(recordsconst) + sizeof("УМНОЖЕНИЕ") - 1;

/* место ОКРУЖНОСТЬ в куче */
constexpr size_t hpcircl = hpmul + sizeof(recordsconst) + sizeof("ОКРУЖНОСТЬ") - 1;

/* место ЗАКРАСКА в куче */
constexpr size_t hppaint = hpcircl + sizeof(recordsconst) + sizeof("ЗАКРАСКА") - 1;

/* место КОПИЯ в куче */
constexpr size_t hpcopy = hppaint + sizeof(recordsconst) + sizeof("КОПИЯ") - 1;

/* место ПРЕДЛ в куче */
constexpr size_t hpclaus = hpcopy + sizeof(recordsconst) + sizeof("ПРЕДЛ") - 1;

/* место ОТРЕЗОК(ЛИНИЯ) в куче */
constexpr size_t hpline = hpclaus + sizeof(recordsconst) + sizeof("ЛИНИЯ") - 1;

// место СЛУЧ в куче
constexpr size_t hprand = hpline + sizeof(recordsconst) + sizeof("СЛУЧ") - 1;

// место СЛОЖЕНИЕ
constexpr size_t hpadd = hprand + sizeof(recordsconst) + sizeof("СЛОЖЕНИЕ") - 1;

// место ЖДИ
constexpr size_t hpwait = hpadd + sizeof(recordsconst) + sizeof("ЖДИ") - 1;

// место div
constexpr size_t hpdiv = hpwait + sizeof(recordsconst) + sizeof("div") - 1;

// место int
constexpr size_t hp_int = hpdiv + sizeof(recordsconst) + sizeof("int") - 1;

// место float
constexpr size_t hp_float = hp_int + sizeof(recordsconst) + sizeof("float") - 1;

// место ТИХО
constexpr size_t hpquiet = hp_float + sizeof(recordsconst) + sizeof("ТИХО") - 1;

// место ВЕРСИЯ
constexpr size_t hpversion = hpquiet + sizeof(recordsconst) + sizeof("ВЕРСИЯ") - 1;
