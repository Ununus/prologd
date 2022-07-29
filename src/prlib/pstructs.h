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

struct baserecord  //������ ���� ��������
{
  unsigned int ident;
};

struct recordsconst
  : public baserecord  //��������� ���������
{
  // char *ptr;                 //��������� � ����� �� ���������� �������������
  unsigned int ptrsymb;  //������ � heap ���� �������������
  unsigned int length;   //�����
  unsigned int begin;    //������ � heap �� ������ �����. ��� ��� ����� ����������
  recordsconst(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
};

struct recordstring : public baserecord {
  // char *ptr; //��������� �� ������ � ������
  unsigned int ptrsymb;  //������ � heap ���� �������������
  unsigned int length;   //����� ������ �� ������ " � '\0'
  unsigned int begin;    //������ ��� ������������� � recordsconst ������ =0;
  recordstring(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN = 0);
  //���� begin!=0 �� ��� ������ ��� ������ ������� ��� ���������� ���������
  //�������� zap3();
  //������� � ��� ��������� �� � ������ ��������� � ����� ������
  //������������� �� ����� ���������� ��� ������ �� ���������� ���������
  // ����� ��� � ������ ��������� � ����������� ��� �������� ���� � �������
  //~recordstring();
};

struct recordvar
  : public baserecord  //��������� ������ ����������
{
  // char *ptr;         //��������� �� ����� ��������� ����������
  unsigned int ptrsymb;  //������ � heap ���� �������������
  unsigned int length;   //����� ����������
  unsigned int num;      //����� ���������� � ����������� ������� � 0
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
  // unsigned int *ptr;//��������� �� �������� �������� ������
  //������ � ��-�� ��������� ������� �� 1 ������ ��� ��-�� ���������
  unsigned int length;   //����� ������
  unsigned int precord;  //������ �������� ������
  recordexpression(unsigned int LENGTH, unsigned int PRECORD);
};

struct recordfunction : public baserecord {
  unsigned int narg;    //����� ���������� � �������
  unsigned int func;    //��������� �� ����� ������
  unsigned int ptrarg;  //������ ������� � �����������
  // unsigned int *ptrarg; //��������� �� ������ �������� � ������� heap
  //��� ������������� ��������� �� ������� ����� ���������
  //� ������� ����  ����� ���������� � narg
  //������ ������� � �� 1 ������ ��� ������ �������
  recordfunction(unsigned char NARG, unsigned int FUNC, unsigned int PTRARG);
};

struct recordlist : public baserecord {
  unsigned int head;  //��������� �� ������
  unsigned int link;  //��������� �� �����
  recordlist(unsigned int HEAD, unsigned int LINK = 0);
};

struct recordclause : public baserecord {
  unsigned int next;       //����� � ������� heap ���������� ������������ �����������
  unsigned int nvars;      //����� ���������� � �����������
  unsigned int head;       //������ � heap (�� issymbol???) ������ �����������
  unsigned int ptrtarget;  //������ ������ �����
  // unsigned int *ptrtarget;   //��������� �� ������ �������� � ������� heap
  //��������� ����� ����� ����� ��������� ����� �������� �� 1 ������ �������
  //�����������
  recordclause(unsigned char IDENT, unsigned int NEXT, unsigned int NVARS, unsigned int HEAD, unsigned int PTRTARGET);
};
//-----------------------------------------------
struct array  //������ ��� ���������� � ���������� ���������
              //������� �������� ������� � main()
{
protected:
  unsigned char *heaps;  //��������� �� ������
public:
  unsigned int freeheap;                           //���-�� ��-��� �� ������� ���������
  unsigned int size;                               //���-�� ��-��� ����� /���� unsigned int/
  unsigned int last;                               //������ ������� ����������� �� ������ ��������� ��
  unsigned int query;                              //������ ����������� - �������.
  unsigned int apend(void *, unsigned int count);  //���������� � �����
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

  recordclause *pnclause;    //��� newclause
  recordclause *ptclause;    //��� trylause
  recordclause *phclause;    //��� head
  recordclause *paclause;    //�� �� ������ ��� atomp
  unsigned int *pacltarget,  //��������� �� ���� ����������� aclause
                             //(aclause-1)
    *ptcltarget, *pncltarget;
};

// ��������� ��� scaner
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
  unsigned int exprip;   //����� �� ����� ? !!!
  unsigned int hpunkn;   //��������� �������� � buf
  unsigned int hpempty;  //��������� ������� ������

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

//��������� ��� �������� ������ - ���������
constexpr int _vmaxstack_ = 1000;
constexpr int _maxbf_ = 2048;
struct TClVar {
  unsigned int stat;
  unsigned int vmaxstack;
  unsigned int *st_con;
  unsigned int *st_vr1;
  unsigned int *st_vr2;
  unsigned int *st_trail;
  unsigned int *bf;    // ������ ����� ����������
  unsigned int varqu;  //���-�� ��������

  unsigned int newclause,  //������ � ������� ����������� �� �����������
    nclause,               //������ ��� newclause
    aclause,               //           atomp
    hclause,               //           head
    tclause,               //           tryclause
    svptr,                 //
    oldsvptr,              //
    scptr,                 //������� ����� ����������
    frame2,                //��������� � ����� � ����� ����������
    frame1,                //
    parent,                //��������� �� ������������ ����� � ����� ����������
    bipr,                  //��������� �� ������� ���������
    head,                  //����� ������ � ���� �� ����������� ����
    oldtptr,               //������� ����� ����� �� ����������
    tptr,                  //������� ����� �����
    err;                   //������
  bool ntro;
  int atomp;  //��������� �� ����
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

//������� � ������� heap   ��������� �� ������ ����������
/* ����� mod � ���� */
#define hpmod 0 + sizeof("mod") - 1

/* ����� con: � ���� */
//#define hpcon    hpmod + sizeof(recordsconst) + sizeof("con:") - 1

/* ����� grp: � ���� */
//#define hpgrp    hpcon + sizeof(recordsconst) + sizeof("grp:") - 1

/* ����� prn: � ���� */
//#define hpprn    hpgrp+sizeof(recordsconst) + sizeof("prn:") - 1

/* ����� ���� � ���� */
#define hpfail hpmod + sizeof(recordsconst) + sizeof("����") - 1

/* ����� ������ � ���� */
#define hptrace hpfail + sizeof(recordsconst) + sizeof("������") - 1

/* ����� ���_������ � ���� */
#define hpnottr hptrace + sizeof(recordsconst) + sizeof("���_������") - 1

/* ����� ! � ���� */
#define hpcut hpnottr + sizeof(recordsconst) + sizeof("!") - 1

/* ����� ��� � ���� */
#define hpcall hpcut + sizeof(recordsconst) + sizeof("���") - 1

/* ����� �������� � ���� */
#define hprdsym hpcall + sizeof(recordsconst) + sizeof("��������") - 1

/* ����� ������� � ���� */
#define hprdint hprdsym + sizeof(recordsconst) + sizeof("�������") - 1

/* ����� ������� � ���� */
#define hprdfloat hprdint + sizeof(recordsconst) + sizeof("�������") - 1

/* ����� ����� � ���� */
#define hpiocod hprdfloat + sizeof(recordsconst) + sizeof("�����") - 1

/* ����� ������_�� � ���� */
#define hpsee hpiocod + sizeof(recordsconst) + sizeof("������_��") - 1

/* ����� ������_� � ���� */
#define hptell hpsee + sizeof(recordsconst) + sizeof("������_�") - 1

// ����� ��� � ����
#define hpvar hptell + sizeof(recordsconst) + sizeof("���") - 1

// ����� ��� � ����
#define hpint hpvar + sizeof(recordsconst) + sizeof("���") - 1

// ����� ��� � ����
#define hpfloat hpint + sizeof(recordsconst) + sizeof("���") - 1

/* ����� ���� � ���� */
#define hpsym hpfloat + sizeof(recordsconst) + sizeof("����") - 1

/* ����� ����� � ���� */
#define hpout hpsym + sizeof(recordsconst) + sizeof("�����") - 1

/* ����� ������ � ���� */
#define hpgt hpout + sizeof(recordsconst) + sizeof("������") - 1

/* ����� ������ � ���� */
#define hpstint hpgt + sizeof(recordsconst) + sizeof("������") - 1

/* ����� ������ � ���� */
#define hpstfloat hpstint + sizeof(recordsconst) + sizeof("������") - 1

/* ����� ������� � ���� */
#define hpstlst hpstfloat + sizeof(recordsconst) + sizeof("�������") - 1

/* ����� ������ � ���� */
#define hpintfloat hpstlst + sizeof(recordsconst) + sizeof("������") - 1

/* ����� ����� � ���� */
#define hplettr hpintfloat + sizeof(recordsconst) + sizeof("�����") - 1

/* ����� ����� � ���� */
#define hpdigit hplettr + sizeof(recordsconst) + sizeof("�����") - 1

/* ����� ���� � ���� */
#define hpterm hpdigit + sizeof(recordsconst) + sizeof("����") - 1

/* ����� �������� � ���� */
#define hpdel hpterm + sizeof(recordsconst) + sizeof("��������") - 1

/* ����� ������� � ���� */
#define hpskol hpdel + sizeof(recordsconst) + sizeof("�������") - 1

/* ����� ����� � ���� */
#define hpset hpskol + sizeof(recordsconst) + sizeof("�����") - 1

/* ����� ���� � ���� */
#define hpapp hpset + sizeof(recordsconst) + sizeof("����") - 1

/* ����� ��� � ���� */
#define hpassrt hpapp + sizeof(recordsconst) + sizeof("���") - 1

/* ����� ��������� � ���� */
#define hpmul hpassrt + sizeof(recordsconst) + sizeof("���������") - 1

/* ����� ���������� � ���� */
#define hpcircl hpmul + sizeof(recordsconst) + sizeof("����������") - 1

/* ����� �������� � ���� */
#define hppaint hpcircl + sizeof(recordsconst) + sizeof("��������") - 1

/* ����� ����� � ���� */
#define hpcopy hppaint + sizeof(recordsconst) + sizeof("�����") - 1

/* ����� ����� � ���� */
#define hpclaus hpcopy + sizeof(recordsconst) + sizeof("�����") - 1

/* ����� �������(�����) � ���� */
#define hpline hpclaus + sizeof(recordsconst) + sizeof("�����") - 1

// ����� ���� � ����
#define hprand hpline + sizeof(recordsconst) + sizeof("����") - 1

//����� ��������
#define hpadd hprand + sizeof(recordsconst) + sizeof("��������") - 1

//����� ���
#define hpwait hpadd + sizeof(recordsconst) + sizeof("���") - 1

//����� div
#define hpdiv hpwait + sizeof(recordsconst) + sizeof("div") - 1

//����� int
#define hp_int hpdiv + sizeof(recordsconst) + sizeof("int") - 1

//����� float
#define hp_float hp_int + sizeof(recordsconst) + sizeof("float") - 1

#endif
