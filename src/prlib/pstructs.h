#pragma once
#include <fstream>
#include <string>
#include <stdio.h>
#include <memory>
#include <vector>
#include "err.h"
#include "pdefs.h"

using FloatType = double;
using IntegerType = long long;

// ������������ � control
enum class PredicateState {
  Error = 1,
  PrepereNewTarget = 2,
  Yes = 3,
  ControlStep = 4,
  No = 5,
  Builtin = 6
};

// �������������� ����� ���������� �������
enum class ArgType : unsigned char {
  None = 0,       // ��������� �� ���������
  Anonim = 1,     // ��������
  EmptyList = 2,  // ������ ������
  List = 3,       // ������
  QStr = 4,       // "������"
  Var = 5,        // ����������
  Float = 6,      // ������������
  Int = 7,        // �����
  Str = 9         // ������
};

static inline ArgType DEBUGArgTypeFromInt(unsigned sw) {
  switch (sw) {
  case 1: return ArgType::Anonim;
  case 2: return ArgType::EmptyList;
  case 3: return ArgType::List;
  case 4: return ArgType::QStr;
  case 5: return ArgType::Var;
  case 6: return ArgType::Float;
  case 7: return ArgType::Int;
  case 9: return ArgType::Str;

  default: break;
  }
  return ArgType::None;
}

using Arg1 = std::tuple<ArgType>;
using Arg2 = std::tuple<ArgType, ArgType>;
using Arg3 = std::tuple<ArgType, ArgType, ArgType>;
using Arg4 = std::tuple<ArgType, ArgType, ArgType, ArgType>;
using Arg5 = std::tuple<ArgType, ArgType, ArgType, ArgType, ArgType>;

// TODO: Exception
struct WrongArgumentType {
  ArgType arg;
  size_t position;
  // ��, ���������. ����� ����� �����?
  std::vector<ArgType> expectedArgs;

  explicit WrongArgumentType(ArgType arg, size_t position)
    : arg(arg)
    , position(position) {}
};

/// @brief  ��� ������� ������� ���������� WrongArgumentType,
///         ���� � args �� I ������� ��������� �� ��� Arg.
///         ������:
///         ExpectArgType<0, ArgType::Float, ArgType::Int>(args) -
///             ������� ��� � args �� 0 ������� ����� ���� Float, ���� Int,
///             ����� �������� ����������.
/// @tparam I - ������� ���������
/// @tparam Arg - ��������� ���
/// @tparam ArgTuple - Arg1, Arg2, Arg3, Arg4 ��� Arg5
/// @param args - ����������� ���������
template<size_t I, ArgType Arg, class ArgTuple>
void ExpectArgType(const ArgTuple &args) {
  if (std::get<I>(args) == Arg) {
    return;
  }
  WrongArgumentType err(std::get<I>(args), I);
  err.expectedArgs.emplace_back(Arg);
  throw err;
}
template<size_t I, ArgType Arg1, ArgType Arg2, ArgType... Args, class ArgTuple>
void ExpectArgType(const ArgTuple &args) {
  if (std::get<I>(args) == Arg1) {
    return;
  }
  try {
    ExpectArgType<I, Arg2, Args...>(args);
  } catch (WrongArgumentType &err) {
    err.expectedArgs.emplace_back(Arg1);
    throw;
  }
}
/// @brief  ��� ������� ���������� true, ���� � args �� I ������� ��������� ��� Arg.
///         ������:
///         IsArgType<0, ArgType::Float, ArgType::Int>(args) -
///             ����� true, ���� � args �� 0 ������� ����� ���� Float, ���� Int.
/// @tparam I - ������� ���������
/// @tparam Arg - ��������� ���
/// @tparam ArgTuple - Arg1, Arg2, Arg3, Arg4 ��� Arg5
/// @param args - ����������� ���������
template<size_t I, ArgType Arg, class ArgTuple>
bool IsArgType(const ArgTuple &args) {
  return std::get<I>(args) == Arg;
}
template<size_t I, ArgType Arg1, ArgType Arg2, ArgType... Args, class ArgTuple>
bool IsArgType(const ArgTuple &args) {
  if (std::get<I>(args) == Arg1) {
    return true;
  }
  return IsArgType<I, Arg2, Args...>(args);
}

// ������ ���� ��������
struct baserecord {
  unsigned int ident;
};

// ��������� ���������
struct recordsconst : public baserecord {
  // char *ptr;          //��������� � ����� �� ���������� �������������
  unsigned int ptrsymb;  // ������ � heap ���� �������������
  unsigned int length;   // �����
  unsigned int begin;    // ������ � heap �� ������ �����. ��� ��� ����� ����������
  recordsconst(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
};

struct recordstring : public baserecord {
  // char *ptr;           //��������� �� ������ � ������
  unsigned int ptrsymb;  // ������ � heap ���� �������������
  unsigned int length;   // ����� ������ �� ������ " � '\0'
  unsigned int begin;    // ������ ��� ������������� � recordsconst ������ =0;
  recordstring(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
  // ���� begin!=0 �� ��� ������ ��� ������ ������� ��� ���������� ���������
  // �������� zap3();
  // ������� � ��� ��������� �� � ������ ��������� � ����� ������
  // ������������� �� ����� ���������� ��� ������ �� ���������� ���������
  //  ����� ��� � ������ ��������� � ����������� ��� �������� ���� � �������
  //~recordstring();
};

// ��������� ������ ����������
struct recordvar : public baserecord {
  // char *ptr;         //��������� �� ����� ��������� ����������
  unsigned int ptrsymb;  // ������ � heap ���� �������������
  unsigned int length;   // ����� ����������
  unsigned int num;      // ����� ���������� � ����������� ������� � 0
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
  // unsigned int *ptr;//��������� �� �������� �������� ������
  // ������ � ��-�� ��������� ������� �� 1 ������ ��� ��-�� ���������
  unsigned int length;   // ����� ������
  unsigned int precord;  // ������ �������� ������
  recordexpression(unsigned int LENGTH, unsigned int PRECORD);
};

struct recordfunction : public baserecord {
  unsigned int narg;    // ����� ���������� � �������
  unsigned int func;    // ��������� �� ����� ������
  unsigned int ptrarg;  // ������ ������� � �����������
  // unsigned int *ptrarg; //��������� �� ������ �������� � ������� heap
  // ��� ������������� ��������� �� ������� ����� ���������
  // � ������� ����  ����� ���������� � narg
  // ������ ������� � �� 1 ������ ��� ������ �������
  recordfunction(unsigned char NARG, unsigned int FUNC, unsigned int PTRARG);
};

struct recordlist : public baserecord {
  unsigned int head;  // ��������� �� ������
  unsigned int link;  // ��������� �� �����
  recordlist(unsigned int HEAD, unsigned int LINK = 0);
};

struct recordclause : public baserecord {
  unsigned int next;       // ����� � ������� heap ���������� ������������ �����������
  unsigned int nvars;      // ����� ���������� � �����������
  unsigned int head;       // ������ � heap (�� issymbol???) ������ �����������
  unsigned int ptrtarget;  // ������ ������ �����
  // unsigned int *ptrtarget;   //��������� �� ������ �������� � ������� heap
  // ��������� ����� ����� ����� ��������� ����� �������� �� 1 ������ �������
  // �����������
  recordclause(unsigned char IDENT, unsigned int NEXT, unsigned int NVARS, unsigned int HEAD, unsigned int PTRTARGET);
};
//-----------------------------------------------
// ������ ��� ���������� � ���������� ���������
// ������� �������� ������� � main()
struct array {
protected:
  unsigned char *heaps;  // ��������� �� ������
public:
  unsigned int freeheap;                           // ���-�� ��-��� �� ������� ���������
  unsigned int size;                               // ���-�� ��-��� ����� /���� unsigned int/
  unsigned int last;                               // ������ ������� ����������� �� ������ ��������� ��
  unsigned int query;                              // ������ ����������� - �������.
  unsigned int apend(void *, unsigned int count);  // ���������� � �����
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

  recordclause *pnclause;    // ��� newclause
  recordclause *ptclause;    // ��� trylause
  recordclause *phclause;    // ��� head
  recordclause *paclause;    // �� �� ������ ��� atomp
  unsigned int *pacltarget,  // ��������� �� ���� ����������� aclause
                             //(aclause-1)
    *ptcltarget, *pncltarget;
};

constexpr int bruteExpand = 8;
// ��������� ��� scaner
constexpr int _maxbptr_ = bruteExpand * 1024;
constexpr int _maxgptr_ = bruteExpand * 150;
constexpr int _maxvar_ = bruteExpand * 512;
constexpr int _maxsymbol_ = bruteExpand * 4096;
constexpr int _maxarray_ = bruteExpand * 5000;

struct TScVar {
  unsigned int *buf;
  unsigned int *goal;
  unsigned int *tat;  // ��������� ������������� ��������
  unsigned int *tvar; // ������� ����������
  unsigned int bptr;
  unsigned int gptr;
  unsigned int nosymbol;
  unsigned int novar; // ������ ��������� ����� � tvar
  bool right;
  bool EndOfClause;
  bool Query;
  unsigned int exprip;   // ����� �� ����� ? !!!
  unsigned int hpunkn;   // ��������� �������� � buf
  unsigned int hpempty;  // ��������� ������� ������

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
// ��������� ��� �������� ������ - ���������
constexpr int _vmaxstack_ = bruteExpand2 * 1000;
constexpr int _maxbf_ = bruteExpand2 * 2048;

struct TClVar {
  PredicateState stat;
  unsigned int vmaxstack;
  unsigned int *st_con;
  unsigned int *st_vr1;
  unsigned int *st_vr2;
  unsigned int *st_trail;
  unsigned int *bf;    // ������ ����� ����������
  unsigned int varqu;  // ���-�� ��������

  unsigned int newclause,                // ������ � ������� ����������� �� �����������
    nclause,                             // ������ ��� newclause
    aclause,                             //           atomp
    hclause,                             //           head
    tclause,                             //           tryclause
    svptr,                               //
    oldsvptr,                            //
    scptr,                               // ������� ����� ����������
    frame2,                              // ��������� � ����� � ����� ����������
    frame1,                              //
    parent,                              // ��������� �� ������������ ����� � ����� ����������
    bipr,                                // ��������� �� ������� ���������
    head,                                // ����� ������ � ���� �� ����������� ����
    oldtptr,                             // ������� ����� ����� �� ����������
    tptr;                                // ������� ����� �����
  ErrorCode err{ ErrorCode::NoErrors };  // ������
  bool ntro;
  int atomp;  // ��������� �� ����
  int tryclause;
  bool ex, flag;

  // ��� unify
  unsigned int bp, term1, term2, fr1, fr2, numb;
  /*
    recordclause * pnclause;//��� newclause
    recordclause * ptclause;//��� trylause
    recordclause * phclause;//��� head
    recordclause * paclause;//�� �� ������ ��� atomp
    unsigned int
            * pacltarget,   //��������� �� ���� ����������� aclause
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

  unsigned int *BPT;  // ��� ������ �������
  unsigned int *bpt;  // ��������� � ������� BPT

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

// ������� � ������� heap   ��������� �� ������ ����������

/* ����� mod � ���� */
constexpr size_t hpmod = 0 + sizeof("mod") - 1;

/* ����� ���� � ���� */
constexpr size_t hpfail = hpmod + sizeof(recordsconst) + sizeof("����") - 1;

/* ����� ������ � ���� */
constexpr size_t hptrace = hpfail + sizeof(recordsconst) + sizeof("������") - 1;

/* ����� ���_������ � ���� */
constexpr size_t hpnottr = hptrace + sizeof(recordsconst) + sizeof("���_������") - 1;

/* ����� ! � ���� */
constexpr size_t hpcut = hpnottr + sizeof(recordsconst) + sizeof("!") - 1;

/* ����� ��� � ���� */
constexpr size_t hpcall = hpcut + sizeof(recordsconst) + sizeof("���") - 1;

/* ����� �������� � ���� */
constexpr size_t hprdsym = hpcall + sizeof(recordsconst) + sizeof("��������") - 1;

/* ����� ������� � ���� */
constexpr size_t hprdint = hprdsym + sizeof(recordsconst) + sizeof("�������") - 1;

/* ����� ������� � ���� */
constexpr size_t hprdfloat = hprdint + sizeof(recordsconst) + sizeof("�������") - 1;

/* ����� ����� � ���� */
constexpr size_t hpiocod = hprdfloat + sizeof(recordsconst) + sizeof("�����") - 1;

/* ����� ������_�� � ���� */
constexpr size_t hpsee = hpiocod + sizeof(recordsconst) + sizeof("������_��") - 1;

/* ����� ������_� � ���� */
constexpr size_t hptell = hpsee + sizeof(recordsconst) + sizeof("������_�") - 1;

// ����� ��� � ����
constexpr size_t hpvar = hptell + sizeof(recordsconst) + sizeof("���") - 1;

// ����� ��� � ����
constexpr size_t hpint = hpvar + sizeof(recordsconst) + sizeof("���") - 1;

// ����� ��� � ����
constexpr size_t hpfloat = hpint + sizeof(recordsconst) + sizeof("���") - 1;

/* ����� ���� � ���� */
constexpr size_t hpsym = hpfloat + sizeof(recordsconst) + sizeof("����") - 1;

/* ����� ������ � ���� */
constexpr size_t hplst = hpsym + sizeof(recordsconst) + sizeof("������") - 1;

/* ����� ����� � ���� */
constexpr size_t hpout = hplst + sizeof(recordsconst) + sizeof("�����") - 1;

/* ����� ������ � ���� */
constexpr size_t hpgt = hpout + sizeof(recordsconst) + sizeof("������") - 1;

/* ����� ������ � ���� */
constexpr size_t hpstint = hpgt + sizeof(recordsconst) + sizeof("������") - 1;

/* ����� ������ � ���� */
constexpr size_t hpstfloat = hpstint + sizeof(recordsconst) + sizeof("������") - 1;

/* ����� ������� � ���� */
constexpr size_t hpstlst = hpstfloat + sizeof(recordsconst) + sizeof("�������") - 1;

/* ����� ������ � ���� */
constexpr size_t hpintfloat = hpstlst + sizeof(recordsconst) + sizeof("������") - 1;

/* ����� ����� � ���� */
constexpr size_t hplettr = hpintfloat + sizeof(recordsconst) + sizeof("�����") - 1;

/* ����� ����� � ���� */
constexpr size_t hpdigit = hplettr + sizeof(recordsconst) + sizeof("�����") - 1;

/* ����� ���� � ���� */
constexpr size_t hpterm = hpdigit + sizeof(recordsconst) + sizeof("����") - 1;

/* ����� �������� � ���� */
constexpr size_t hpdel = hpterm + sizeof(recordsconst) + sizeof("��������") - 1;

/* ����� ������� � ���� */
constexpr size_t hpskol = hpdel + sizeof(recordsconst) + sizeof("�������") - 1;

/* ����� ����� � ���� */
constexpr size_t hpset = hpskol + sizeof(recordsconst) + sizeof("�����") - 1;

/* ����� ���� � ���� */
constexpr size_t hpapp = hpset + sizeof(recordsconst) + sizeof("����") - 1;

/* ����� ��� � ���� */
constexpr size_t hpassrt = hpapp + sizeof(recordsconst) + sizeof("���") - 1;

/* ����� ��������� � ���� */
constexpr size_t hpmul = hpassrt + sizeof(recordsconst) + sizeof("���������") - 1;

/* ����� ���������� � ���� */
constexpr size_t hpcircl = hpmul + sizeof(recordsconst) + sizeof("����������") - 1;

/* ����� �������� � ���� */
constexpr size_t hppaint = hpcircl + sizeof(recordsconst) + sizeof("��������") - 1;

/* ����� ����� � ���� */
constexpr size_t hpcopy = hppaint + sizeof(recordsconst) + sizeof("�����") - 1;

/* ����� ����� � ���� */
constexpr size_t hpclaus = hpcopy + sizeof(recordsconst) + sizeof("�����") - 1;

/* ����� �������(�����) � ���� */
constexpr size_t hpline = hpclaus + sizeof(recordsconst) + sizeof("�����") - 1;

// ����� ���� � ����
constexpr size_t hprand = hpline + sizeof(recordsconst) + sizeof("����") - 1;

// ����� ��������
constexpr size_t hpadd = hprand + sizeof(recordsconst) + sizeof("��������") - 1;

// ����� ���
constexpr size_t hpwait = hpadd + sizeof(recordsconst) + sizeof("���") - 1;

// ����� div
constexpr size_t hpdiv = hpwait + sizeof(recordsconst) + sizeof("div") - 1;

// ����� int
constexpr size_t hp_int = hpdiv + sizeof(recordsconst) + sizeof("int") - 1;

// ����� float
constexpr size_t hp_float = hp_int + sizeof(recordsconst) + sizeof("float") - 1;