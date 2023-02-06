#pragma once

#include "pstructs.h"

// ���������, ��� ���������� ���������� �� ��������� ��������� 'name' ����� 'narg'
bool bpred(unsigned name, unsigned narg);

PredicateState argnull(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argone(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argtwo(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argthree(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfour(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfive(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);

// ��������� ������ ���������� � ����������
IntegerType occ(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������� ������ ���������� � ����������
FloatType occf(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� ������ num � arg ���������� ���������
PredicateState zap1(IntegerType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� ����� num1 � num2 � arg1 � arg2 ������������� ����������� ���������
PredicateState zap2(IntegerType num1, IntegerType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� float num � arg ���������� ���������
PredicateState zap1f(FloatType num, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������ ������ � ���������� ��� � arg
PredicateState zap3(const char *str, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);

// ���
PredicateState prcall(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prrandom(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���
PredicateState prwait(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������_�
PredicateState outfile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������_��
PredicateState infile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState priocod(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prrdint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prrdfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
PredicateState prrdsym(unsigned sw, TScVar *ScVar, TClVar *Clvar, array *heap);
// ������
PredicateState prstint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
PredicateState prstfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prstlst(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
PredicateState printfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����; �����
PredicateState whatisit(unsigned sw, bool (*f)(char), unsigned i, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prdel(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prskol(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������������� � �����
PredicateState print(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������������� � float
PredicateState prfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prterm(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
PredicateState prgt(TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
PredicateState prpaint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
PredicateState pradd(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���
PredicateState prassrt(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prapp(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prset(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������
PredicateState prmul3(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������
PredicateState prmul(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����������
PredicateState prcircl(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prcopy(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prclaus(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prger(unsigned long sw, TScVar *ScVar, TClVar *ClVar, array *heap);
//=========== ������� ����� ==============
int Inputstring(char *buf, int size, char *caption = 0);
int InputSymbol(char *buf);
int InputInt(IntegerType *n, const char *caption);
int InputFloat(FloatType *n, const char *caption);

// out for debug
void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);
void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);