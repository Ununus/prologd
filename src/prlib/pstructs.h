#pragma once
#include <fstream>
#include <string>
#include <stdio.h>
#include <memory>
#include "err.h"
// #include <limits.h>
// #include "pdefs.h"

using FloatType = double;
using IntegerType = long long;

// используется в control
enum class PredicateState {
  Error = 1,
  State_2 = 2,  // Не знаю что за состояние 
  Yes = 3,
  State_4 = 4,  // Не знаю что за состояние
  No = 5,
  Builtin = 6
};

// предок всех структур
struct baserecord {
  unsigned int ident;
};

// строковая константа
struct recordsconst : public baserecord {
  // char *ptr;          //указатель в текст на символьное представление
  unsigned int ptrsymb;  // индекс в heap симв представления
  unsigned int length;   // длина
  unsigned int begin;    // индекс в heap на первое предл. где она стоит предикатом
  recordsconst(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
};

struct recordstring : public baserecord {
  // char *ptr;           //указатель на строку в тексте
  unsigned int ptrsymb;  // индекс в heap симв представления
  unsigned int length;   // длина строки не считая " и '\0'
  unsigned int begin;    // введен для совместимости с recordsconst всегда =0;
  recordstring(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
  // если begin!=0 то это значит что строка создана при исполнении программы
  // функцией zap3();
  // пролога и она находится не в тексте редактора а динам памяти
  // следовательно ее нужно уничточить при выходи из исполнения программы
  //  иначе это в тексте редактора и уничтожится при закрытии окна с текстом
  //~recordstring();
};

// структура записи переменной
struct recordvar : public baserecord {
  // char *ptr;         //указатель на текст представл переменной
  unsigned int ptrsymb;  // индекс в heap симв представления
  unsigned int length;   // длина переменной
  unsigned int num;      // номер переменный в предложении начиная с 0
  recordvar(unsigned PTRsymb, unsigned char LENGTH, unsigned int NUM = 0);
};

struct recordinteger : public baserecord {
  IntegerType value;
  recordinteger(IntegerType VALUE);
};

struct recordfloat : public baserecord {
  FloatType value;
  recordfloat(FloatType VALUE);
};

struct recordunknown : public baserecord {
  recordunknown();
};

struct recordcut : public baserecord {
  unsigned int func;
  recordcut();
};

struct recordemptylist : public baserecord {
  recordemptylist();
};

struct recordexpression : public baserecord {
  // unsigned int *ptr;//указатель на обратную польскую запись
  // запись в эл-те указатель которой на 1 меньше чем эл-та структуры
  unsigned int length;   // длина записи
  unsigned int precord;  // индекс польской записи
  recordexpression(unsigned int LENGTH, unsigned int PRECORD);
};

struct recordfunction : public baserecord {
  unsigned int narg;    // число аргументов в функции
  unsigned int func;    // указатель на функц символ
  unsigned int ptrarg;  // индекс массива с аргументами
  // unsigned int *ptrarg; //указатель на массив индексов в массиве heap
  // при использовании указатели из массива нужно приводить
  // к нужному типу  число указателей в narg
  // индекс массива в на 1 меньше чем индекс функции
  recordfunction(unsigned char NARG, unsigned int FUNC, unsigned int PTRARG);
};

struct recordlist : public baserecord {
  unsigned int head;  // указатель на голову
  unsigned int link;  // указатель на хвост
  recordlist(unsigned int HEAD, unsigned int LINK = 0);
};

struct recordclause : public baserecord {
  unsigned int next;       // инекс в массиве heap следующего одноименного предложения
  unsigned int nvars;      // число переменных в предложении
  unsigned int head;       // индекс в heap (на issymbol???) голову предложения
  unsigned int ptrtarget;  // индекс начала целей
  // unsigned int *ptrtarget;   //указатель на массив индексов в массиве heap
  // аргументы можно найти череы указатель инекс которого на 1 меньше индекса
  // предложения
  recordclause(unsigned char IDENT, unsigned int NEXT, unsigned int NVARS, unsigned int HEAD, unsigned int PTRTARGET);
};
//-----------------------------------------------
// массив для трансляции и исполнения программы
// пролога описания функций в main()
struct array {
protected:
  unsigned char *heaps;  // указатель на массив
public:
  unsigned int freeheap;                           // кол-во эл-тов до разбора программы
  unsigned int size;                               // кол-во эл-тов всего /типа unsigned int/
  unsigned int last;                               // индекс массива указывающий на первый свободный эл
  unsigned int query;                              // индекс предложения - вопроса.
  unsigned int apend(void *, unsigned int count);  // добавление в конец
  array(unsigned int SIZE);
  void clear();
  int expand();
  ~array();
  baserecord *GetPbaserecord(unsigned int index);
  recordsconst *GetPrecordsconst(unsigned int index);
  recordstring *GetPrecordstring(unsigned int index);
  recordvar *GetPrecordvar(unsigned int index);
  recordinteger *GetPrecordinteger(unsigned int index);
  recordfloat *GetPrecordfloat(unsigned int index);
  recordunknown *GetPrecordunknown(unsigned int index);
  recordcut *GetPrecordcut(unsigned int index);
  recordemptylist *GetPrecordemptylist(unsigned int index);
  recordexpression *GetPrecordexpression(unsigned int index);
  recordfunction *GetPrecordfunction(unsigned int index);
  recordlist *GetPrecordlist(unsigned int index);
  recordclause *GetPrecordclause(unsigned int index);
  unsigned *GetPunsigned(unsigned index);
  char *GetPchar(unsigned index);

  recordclause *pnclause;    // для newclause
  recordclause *ptclause;    // для trylause
  recordclause *phclause;    // для head
  recordclause *paclause;    // ук на структ для atomp
  unsigned int *pacltarget,  // указатель на цели предложения aclause
                             //(aclause-1)
    *ptcltarget, *pncltarget;
};

constexpr int bruteExpand = 8;
// структура для scaner
constexpr int _maxbptr_ = bruteExpand * 1024;
constexpr int _maxgptr_ = bruteExpand * 150;
constexpr int _maxvar_ = bruteExpand * 512;
constexpr int _maxsymbol_ = bruteExpand * 4096;
constexpr int _maxarray_ = bruteExpand * 5000;

struct TScVar {
  unsigned int *buf;
  unsigned int *goal;
  unsigned int *tat;
  unsigned int *tvar;
  unsigned int bptr;
  unsigned int gptr;
  unsigned int nosymbol;
  unsigned int novar;
  bool right;
  bool EndOfClause;
  bool Query;
  unsigned int exprip;   // нужна ли здесь ? !!!
  unsigned int hpunkn;   // положенме анонимки в buf
  unsigned int hpempty;  // положение пустого списка

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

constexpr int bruteExpand2 = 32;
// структуры для хранения данных - программы
constexpr int _vmaxstack_ = bruteExpand2 * 1000;
constexpr int _maxbf_ = bruteExpand2 * 2048;

struct TClVar {
  PredicateState stat;
  unsigned int vmaxstack;
  unsigned int *st_con;
  unsigned int *st_vr1;
  unsigned int *st_vr2;
  unsigned int *st_trail;
  unsigned int *bf;    // массив стека унификации
  unsigned int varqu;  // кол-во вопросов

  unsigned int newclause,                // индекс в массиве указывающий на предложения
    nclause,                             // индекс для newclause
    aclause,                             //           atomp
    hclause,                             //           head
    tclause,                             //           tryclause
    svptr,                               //
    oldsvptr,                            //
    scptr,                               // вершина стека управления
    frame2,                              // указатель к кадру с стеке переменных
    frame1,                              //
    parent,                              // указатель на родительскую среду в стеке управления
    bipr,                                // указатель на функтор предиката
    head,                                // место записи в куче об исполняемой цели
    oldtptr,                             // вершина стека следа до исполнения
    tptr;                                // вершина стека следа
  ErrorCode err{ ErrorCode::NoErrors };  // ошибка
  bool ntro;
  int atomp;  // указатель на цель
  int tryclause;
  bool ex, flag;

  // для unify
  unsigned int bp, term1, term2, fr1, fr2, numb;
  /*
    recordclause * pnclause;//для newclause
    recordclause * ptclause;//для trylause
    recordclause * phclause;//для head
    recordclause * paclause;//ук на структ для atomp
    unsigned int
            * pacltarget,   //указатель на цели предложения aclause
                                                          //(aclause-1)
                          * ptcltarget,
                          * pncltarget;
  */
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

  unsigned int *BPT;  // для вывода пролога
  unsigned int *bpt;  // указатель в массиве BPT

  std::unique_ptr<TPrSetting> PrSetting{};

  TClVar();
  ~TClVar();
  void Init();
  void Clear();
};

//-----------------------------------------
void PrintProgram(TScVar *ScVar, array *heap);

struct options {
  unsigned optionsout;
  unsigned optionsrun;
};

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

/* место ВВОДЦЕЛ в куче */
constexpr size_t hprdint = hprdsym + sizeof(recordsconst) + sizeof("ВВОДЦЕЛ") - 1;

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