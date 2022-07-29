#ifndef pstructsh
#define pstructsh
#include <fstream>
#include <string>
#include <stdio.h>
#include <memory>
//#include <limits.h>
//#include "pdefs.h"

extern void out(const char *str);
extern void errout(const char *str);

struct baserecord  //предок всех структур
{
  unsigned int ident;
};

struct recordsconst
  : public baserecord  //строковая константа
{
  // char *ptr;                 //указатель в текст на символьное представление
  unsigned int ptrsymb;  //индекс в heap симв представления
  unsigned int length;   //длина
  unsigned int begin;    //индекс в heap на первое предл. где она стоит предикатом
  recordsconst(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
};

struct recordstring : public baserecord {
  // char *ptr; //указатель на строку в тексте
  unsigned int ptrsymb;  //индекс в heap симв представления
  unsigned int length;   //длина строки не считая " и '\0'
  unsigned int begin;    //введен для совместимости с recordsconst всегда =0;
  recordstring(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
  //если begin!=0 то это значит что строка создана при исполнении программы
  //функцией zap3();
  //пролога и она находится не в тексте редактора а динам памяти
  //следовательно ее нужно уничточить при выходи из исполнения программы
  // иначе это в тексте редактора и уничтожится при закрытии окна с текстом
  //~recordstring();
};

struct recordvar
  : public baserecord  //структура записи переменной
{
  // char *ptr;         //указатель на текст представл переменной
  unsigned int ptrsymb;  //индекс в heap симв представления
  unsigned int length;   //длина переменной
  unsigned int num;      //номер переменный в предложении начиная с 0
  recordvar(unsigned PTRsymb, unsigned char LENGTH, unsigned int NUM = 0);
};

struct recordinteger : public baserecord {
  int value;
  recordinteger(int VALUE);
};

struct recordfloat : public baserecord {
  float value;
  recordfloat(float VALUE);
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
  //запись в эл-те указатель которой на 1 меньше чем эл-та структуры
  unsigned int length;   //длина записи
  unsigned int precord;  //индекс польской записи
  recordexpression(unsigned int LENGTH, unsigned int PRECORD);
};

struct recordfunction : public baserecord {
  unsigned int narg;    //число аргументов в функции
  unsigned int func;    //указатель на функц символ
  unsigned int ptrarg;  //индекс массива с аргументами
  // unsigned int *ptrarg; //указатель на массив индексов в массиве heap
  //при использовании указатели из массива нужно приводить
  //к нужному типу  число указателей в narg
  //индекс массива в на 1 меньше чем индекс функции
  recordfunction(unsigned char NARG, unsigned int FUNC, unsigned int PTRARG);
};

struct recordlist : public baserecord {
  unsigned int head;  //указатель на голову
  unsigned int link;  //указатель на хвост
  recordlist(unsigned int HEAD, unsigned int LINK = 0);
};

struct recordclause : public baserecord {
  unsigned int next;       //инекс в массиве heap следующего одноименного предложения
  unsigned int nvars;      //число переменных в предложении
  unsigned int head;       //индекс в heap (на issymbol???) голову предложения
  unsigned int ptrtarget;  //индекс начала целей
  // unsigned int *ptrtarget;   //указатель на массив индексов в массиве heap
  //аргументы можно найти череы указатель инекс которого на 1 меньше индекса
  //предложения
  recordclause(unsigned char IDENT, unsigned int NEXT, unsigned int NVARS, unsigned int HEAD, unsigned int PTRTARGET);
};
//-----------------------------------------------
struct array  //массив для трансляции и исполнения программы
              //пролога описания функций в main()
{
protected:
  unsigned char *heaps;  //указатель на массив
public:
  unsigned int freeheap;                           //кол-во эл-тов до разбора программы
  unsigned int size;                               //кол-во эл-тов всего /типа unsigned int/
  unsigned int last;                               //индекс массива указывающий на первый свободный эл
  unsigned int query;                              //индекс предложения - вопроса.
  unsigned int apend(void *, unsigned int count);  //добавление в конец
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

  recordclause *pnclause;    //для newclause
  recordclause *ptclause;    //для trylause
  recordclause *phclause;    //для head
  recordclause *paclause;    //ук на структ для atomp
  unsigned int *pacltarget,  //указатель на цели предложения aclause
                             //(aclause-1)
    *ptcltarget, *pncltarget;
};

// структура для scaner
constexpr int _maxbptr_ = 1024;
constexpr int _maxgptr_ = 150;
constexpr int _maxvar_ = 512;
constexpr int _maxsymbol_ = 4096;
constexpr int _maxarray_ = 5000;

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
  unsigned int exprip;   //нужна ли здесь ? !!!
  unsigned int hpunkn;   //положенме анонимки в buf
  unsigned int hpempty;  //положение пустого списка

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

//структуры для хранения данных - программы
constexpr int _vmaxstack_ = 1000;
constexpr int _maxbf_ = 2048;
struct TClVar {
  unsigned int stat;
  unsigned int vmaxstack;
  unsigned int *st_con;
  unsigned int *st_vr1;
  unsigned int *st_vr2;
  unsigned int *st_trail;
  unsigned int *bf;    // массив стека унификации
  unsigned int varqu;  //кол-во вопросов

  unsigned int newclause,  //индекс в массиве указывающий на предложения
    nclause,               //индекс для newclause
    aclause,               //           atomp
    hclause,               //           head
    tclause,               //           tryclause
    svptr,                 //
    oldsvptr,              //
    scptr,                 //вершина стека управления
    frame2,                //указатель к кадру с стеке переменных
    frame1,                //
    parent,                //указатель на родительскую среду в стеке управления
    bipr,                  //указатель на функтор предиката
    head,                  //место записи в куче об исполняемой цели
    oldtptr,               //вершина стека следа до исполнения
    tptr,                  //вершина стека следа
    err;                   //ошибка
  bool ntro;
  int atomp;  //указатель на цель
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

//индексы в массиве heap   указатели на фреймы переменных
/* место mod в куче */
#define hpmod 0 + sizeof("mod") - 1

/* место con: в куче */
//#define hpcon    hpmod + sizeof(recordsconst) + sizeof("con:") - 1

/* место grp: в куче */
//#define hpgrp    hpcon + sizeof(recordsconst) + sizeof("grp:") - 1

/* место prn: в куче */
//#define hpprn    hpgrp+sizeof(recordsconst) + sizeof("prn:") - 1

/* место ЛОЖЬ в куче */
#define hpfail hpmod + sizeof(recordsconst) + sizeof("ЛОЖЬ") - 1

/* место ТРАССА в куче */
#define hptrace hpfail + sizeof(recordsconst) + sizeof("ТРАССА") - 1

/* место НЕТ_ТРАССЫ в куче */
#define hpnottr hptrace + sizeof(recordsconst) + sizeof("НЕТ_ТРАССЫ") - 1

/* место ! в куче */
#define hpcut hpnottr + sizeof(recordsconst) + sizeof("!") - 1

/* место ВЫП в куче */
#define hpcall hpcut + sizeof(recordsconst) + sizeof("ВЫП") - 1

/* место ВВОДСИМВ в куче */
#define hprdsym hpcall + sizeof(recordsconst) + sizeof("ВВОДСИМВ") - 1

/* место ВВОДЦЕЛ в куче */
#define hprdint hprdsym + sizeof(recordsconst) + sizeof("ВВОДЦЕЛ") - 1

/* место ВВОДВЕЩ в куче */
#define hprdfloat hprdint + sizeof(recordsconst) + sizeof("ВВОДВЕЩ") - 1

/* место ВВКОД в куче */
#define hpiocod hprdfloat + sizeof(recordsconst) + sizeof("ВВКОД") - 1

/* место ЧТЕНИЕ_ИЗ в куче */
#define hpsee hpiocod + sizeof(recordsconst) + sizeof("ЧТЕНИЕ_ИЗ") - 1

/* место ЗАПИСЬ_В в куче */
#define hptell hpsee + sizeof(recordsconst) + sizeof("ЗАПИСЬ_В") - 1

// место ПЕР в куче
#define hpvar hptell + sizeof(recordsconst) + sizeof("ПЕР") - 1

// место ЦЕЛ в куче
#define hpint hpvar + sizeof(recordsconst) + sizeof("ЦЕЛ") - 1

// место ВЕЩ в куче
#define hpfloat hpint + sizeof(recordsconst) + sizeof("ВЕЩ") - 1

/* место СИМВ в куче */
#define hpsym hpfloat + sizeof(recordsconst) + sizeof("СИМВ") - 1

/* место ВЫВОД в куче */
#define hpout hpsym + sizeof(recordsconst) + sizeof("ВЫВОД") - 1

/* место БОЛЬШЕ в куче */
#define hpgt hpout + sizeof(recordsconst) + sizeof("БОЛЬШЕ") - 1

/* место СТРЦЕЛ в куче */
#define hpstint hpgt + sizeof(recordsconst) + sizeof("СТРЦЕЛ") - 1

/* место СТРВЕЩ в куче */
#define hpstfloat hpstint + sizeof(recordsconst) + sizeof("СТРВЕЩ") - 1

/* место СТРСПИС в куче */
#define hpstlst hpstfloat + sizeof(recordsconst) + sizeof("СТРСПИС") - 1

/* место ЦЕЛВЕЩ в куче */
#define hpintfloat hpstlst + sizeof(recordsconst) + sizeof("ЦЕЛВЕЩ") - 1

/* место БУКВА в куче */
#define hplettr hpintfloat + sizeof(recordsconst) + sizeof("БУКВА") - 1

/* место ЦИФРА в куче */
#define hpdigit hplettr + sizeof(recordsconst) + sizeof("ЦИФРА") - 1

/* место ТЕРМ в куче */
#define hpterm hpdigit + sizeof(recordsconst) + sizeof("ТЕРМ") - 1

/* место УДАЛЕНИЕ в куче */
#define hpdel hpterm + sizeof(recordsconst) + sizeof("УДАЛЕНИЕ") - 1

/* место СКОЛЬКО в куче */
#define hpskol hpdel + sizeof(recordsconst) + sizeof("СКОЛЬКО") - 1

/* место ТОЧКА в куче */
#define hpset hpskol + sizeof(recordsconst) + sizeof("ТОЧКА") - 1

/* место СЦЕП в куче */
#define hpapp hpset + sizeof(recordsconst) + sizeof("СЦЕП") - 1

/* место ДОБ в куче */
#define hpassrt hpapp + sizeof(recordsconst) + sizeof("ДОБ") - 1

/* место УМНОЖЕНИЕ в куче */
#define hpmul hpassrt + sizeof(recordsconst) + sizeof("УМНОЖЕНИЕ") - 1

/* место ОКРУЖНОСТЬ в куче */
#define hpcircl hpmul + sizeof(recordsconst) + sizeof("ОКРУЖНОСТЬ") - 1

/* место ЗАКРАСКА в куче */
#define hppaint hpcircl + sizeof(recordsconst) + sizeof("ЗАКРАСКА") - 1

/* место КОПИЯ в куче */
#define hpcopy hppaint + sizeof(recordsconst) + sizeof("КОПИЯ") - 1

/* место ПРЕДЛ в куче */
#define hpclaus hpcopy + sizeof(recordsconst) + sizeof("ПРЕДЛ") - 1

/* место ОТРЕЗОК(ЛИНИЯ) в куче */
#define hpline hpclaus + sizeof(recordsconst) + sizeof("ЛИНИЯ") - 1

// место СЛУЧ в куче
#define hprand hpline + sizeof(recordsconst) + sizeof("СЛУЧ") - 1

//место СЛОЖЕНИЕ
#define hpadd hprand + sizeof(recordsconst) + sizeof("СЛОЖЕНИЕ") - 1

//место ЖДИ
#define hpwait hpadd + sizeof(recordsconst) + sizeof("ЖДИ") - 1

//место div
#define hpdiv hpwait + sizeof(recordsconst) + sizeof("div") - 1

//место int
#define hp_int hpdiv + sizeof(recordsconst) + sizeof("int") - 1

//место float
#define hp_float hp_int + sizeof(recordsconst) + sizeof("float") - 1

#endif
