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

// ������������ � control
enum class PredicateState { Error = 1, PrepereNewTarget = 2, Yes = 3, ControlStep = 4, No = 5, Builtin = 6 };

// ������ ���� ��������
struct baserecord {
  size_t ident;
  baserecord(size_t ident)
    : ident(ident) {}
  const char *name() {
    switch (ident) {
    case isvar: return "���������";
    case issymbol: return "����. ���������";
    case isinteger: return "�����";
    case isfloat: return "������������";
    case isexpression: return "���������";
    case isunknown: return "��������";
    case isemptylist: return "[]";
    case islist: return "������";
    case isfunction: return "�������";
    case isstring: return "������";
    case isclause: return "�������";
    case isclauseq: return "�������_";
    case iscut: return "CUT";
    default: return "�����������";
    }
  }
  virtual ~baserecord() {}
};

// ��������� ���������
struct recordsconst final : public baserecord {
  // char *ptr;    // ��������� � ����� �� ���������� �������������
  size_t ptrsymb;  // ������ � heap ���� �������������
  size_t length;   // �����
  size_t begin;    // ������ � heap �� ������ �����. ��� ��� ����� ����������
  recordsconst(size_t PTRsymb, size_t LENGTH, size_t BEGIN = 0)
    : baserecord(issymbol)
    , ptrsymb(PTRsymb)
    , length(LENGTH)
    , begin(BEGIN) {}
};

// "������"
struct recordstring final : public baserecord {
  // char *ptr;    // ��������� �� ������ � ������
  size_t ptrsymb;  // ������ � heap ���� �������������
  size_t length;   // ����� ������ �� ������ " � '\0'
  size_t begin;    // ������ ��� ������������� � recordsconst ������ =0;
  recordstring(size_t PTRsymb, size_t LENGTH, size_t BEGIN = 0)
    : baserecord(isstring)
    , ptrsymb(PTRsymb)
    , length(LENGTH)
    , begin(BEGIN) {}
};

// ��������� ������ ����������
struct recordvar final : public baserecord {
  // char *ptr;    // ��������� �� ����� ��������� ����������
  size_t ptrsymb;  // ������ � heap ���� �������������
  size_t length;   // ����� ����������
  size_t num;      // ����� ���������� � ����������� ������� � 0
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
  size_t length;   // ����� ������
  size_t precord;  // ������ �������� ������
  recordexpression(size_t LENGTH, size_t PRECORD)
    : baserecord(isexpression)
    , length(LENGTH)
    , precord(PRECORD) {}
};

struct recordfunction final : public baserecord {
  size_t narg;    // ����� ���������� � �������
  size_t func;    // ��������� �� ����� ������
  size_t ptrarg;  // ������ ������� � �����������
  recordfunction(size_t NARG, size_t FUNC, size_t PTRARG)
    : baserecord(isfunction)
    , narg(NARG)
    , func(FUNC)
    , ptrarg(PTRARG) {}
};

struct recordlist final : public baserecord {
  size_t head;  // ��������� �� ������
  size_t link;  // ��������� �� �����
  explicit recordlist(size_t HEAD, size_t LINK = 0)
    : baserecord(islist)
    , head(HEAD)
    , link(LINK) {}
};

struct recordclause final : public baserecord {
  size_t next;       // ����� � ������� heap ���������� ������������ �����������
  size_t nvars;      // ����� ���������� � �����������
  size_t head;       // ������ � heap (�� issymbol???) ������ �����������
  size_t ptrtarget;  // ������ ������ �����
  recordclause(size_t IDENT, size_t NEXT, size_t NVARS, size_t HEAD, size_t PTRTARGET)
    : baserecord(IDENT)
    , next(NEXT)
    , nvars(NVARS)
    , head(HEAD)
    , ptrtarget(PTRTARGET) {}
};
//-----------------------------------------------
// ������ ��� ���������� � ���������� ���������
// ������� �������� ������� � main()
struct array {
protected:
  // unsigned char *heaps;  // ��������� �� ������
  std::vector<unsigned char> vheap;

public:
  size_t freeheap;  // ���-�� ��-��� �� ������� ���������
  size_t last;      // ������ ������� ����������� �� ������ ��������� ��
  size_t query;     // ������ ����������� - �������.
  template<class T>
  size_t append(const T &record, size_t count = 1);  // ���������� � �����

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

  size_t ipnclause;    // ��� newclause
  size_t iptclause;    // ��� trylause
  size_t iphclause;    // ��� head
  size_t ipaclause;    // �� �� ������ ��� atomp
  size_t ipacltarget;  // ��������� �� ���� ����������� aclause
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

// ��������� ��� scaner
struct TScVar {
  // size_t *buf;
  // size_t *goal;
  // size_t *tat;   // ��������� ������������� ��������
  // size_t *tvar;  // ������� ����������
  std::vector<size_t> vbuf;
  std::vector<size_t> vgoal;
  std::vector<size_t> vtat;   // ��������� ������������� ��������
  std::vector<size_t> vtvar;  // ������� ����������
  size_t bptr;
  size_t gptr;
  size_t nosymbol;
  size_t novar;  // ������ ��������� ����� � tvar
  bool right;
  bool EndOfClause;
  bool Query;
  size_t exprip;   // ����� �� ����� ? !!!
  size_t hpunkn;   // ��������� �������� � buf
  size_t hpempty;  // ��������� ������� ������

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

// ��������� ��� �������� ������ - ���������
struct TClVar {
  PredicateState stat;
  std::string outBuff;
  size_t vmaxstack;
  std::vector<size_t> vst_con;
  std::vector<size_t> vst_vr1;
  std::vector<size_t> vst_vr2;
  std::vector<size_t> vst_trail;
  std::vector<size_t> vbf;  // ������ ����� ����������
  size_t varqu;             // ���-�� ��������

  size_t newclause;  // ������ � ������� ����������� �� �����������
  size_t nclause;    // ������ ��� newclause
  size_t aclause;    //            atomp
  size_t hclause;    //            head
  size_t tclause;    //            tryclause
  size_t svptr;      //
  size_t oldsvptr;   //
  size_t scptr;      // ������� ����� ����������
  size_t frame2;     // ��������� � ����� � ����� ����������
  size_t frame1;     //
  size_t parent;     // ��������� �� ������������ ����� � ����� ����������
  size_t bipr;       // ��������� �� ������� ���������
  size_t head;       // ����� ������ � ���� �� ����������� ����
  size_t oldtptr;    // ������� ����� ����� �� ����������
  size_t tptr;       // ������� ����� �����

  ErrorCode err{ ErrorCode::NoErrors };  // ������
  bool ntro;
  size_t atomp;  // ��������� �� ����
  size_t tryclause;
  bool ex, flag;
  bool quiet{ false };  // ��� ��������� ����

  // ��� unify
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

  std::vector<size_t> vBPT;  // ��� ������ �������
  size_t ibpt{};             // ��������� � ������� BPT

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

// ����� �������
constexpr size_t hprdstr = hprdsym + sizeof(recordsconst) + sizeof("�������") - 1;

/* ����� ������� � ���� */
constexpr size_t hprdint = hprdstr + sizeof(recordsconst) + sizeof("�������") - 1;

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

// ����� ����
constexpr size_t hpquiet = hp_float + sizeof(recordsconst) + sizeof("����") - 1;

// ����� ������
constexpr size_t hpversion = hpquiet + sizeof(recordsconst) + sizeof("������") - 1;
