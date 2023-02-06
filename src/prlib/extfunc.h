#ifndef extfunch
#define extfunch

#include "pstructs.h"

// ���������, ��� ���������� ���������� �� ��������� ��������� 'name' ����� 'narg'
bool bpred(unsigned name, unsigned narg);

unsigned argnull(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argone(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argtwo(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argthree(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argfour(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argfive(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);

// ��������� ������ ���������� � ����������
IntegerType occ(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������� ������ ���������� � ����������
FloatType occf(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� ������ num � arg ���������� ���������
unsigned zap1(IntegerType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� ����� num1 � num2 � arg1 � arg2 ������������� ����������� ���������
unsigned zap2(IntegerType num1, IntegerType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� float num � arg ���������� ���������
unsigned zap1f(FloatType num, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������ ������ � ���������� ��� � arg
unsigned zap3(const char *str, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);

// ���
int prcall(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
unsigned prrandom(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���
unsigned prwait(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������_�
unsigned outfile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������_��
unsigned infile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
unsigned prstint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
unsigned priocod(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
unsigned prrdint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
unsigned prrdfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
unsigned prrdsym(unsigned sw, TScVar *ScVar, TClVar *Clvar, array *heap);
// �������
unsigned prskol(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������������� � �����
unsigned print(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������������� � float
unsigned prfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
unsigned prterm(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
unsigned prgt(TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
unsigned prpaint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
unsigned pradd(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���
unsigned prassrt(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
unsigned prapp(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
unsigned prset(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������
unsigned prmul3(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������
unsigned prmul(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����������
unsigned prcircl(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
unsigned prcopy(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
unsigned prclaus(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
unsigned prger(unsigned long sw, TScVar *ScVar, TClVar *ClVar, array *heap);
  //=========== ������� ����� ==============
int Inputstring(char *buf, int size, char *caption = 0);
int InputSymbol(char *buf);
int InputInt(IntegerType *n, const char *caption);
int InputFloat(FloatType *n, const char *caption);

// out for debug
void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);
void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);

#endif
