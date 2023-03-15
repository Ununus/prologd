#pragma once

#include "pstructs.h"

// ���������, ��� ���������� ���������� �� ��������� ��������� 'name' ����� 'narg'
bool bpred(size_t name, size_t narg);

PredicateState argnull(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argone(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argtwo(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argthree(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfour(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfive(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);

// ��������� ������ ���������� � ����������
IntegerType occ(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������� ������ ���������� � ����������
FloatType occf(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� ������ num � arg ���������� ���������
PredicateState zap1(IntegerType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� ����� num1 � num2 � arg1 � arg2 ������������� ����������� ���������
PredicateState zap2(IntegerType num1, IntegerType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������� float num � arg ���������� ���������
PredicateState zap1f(FloatType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������ ������ � ���������� ��� � arg
PredicateState zap3(const char *str, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);

// ���
PredicateState prcall(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prrandom(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���
PredicateState prwait(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������_�
PredicateState outfile(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������_��
PredicateState infile(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState priocod(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prrdint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prrdfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
PredicateState prrdsym(size_t sw, TScVar *ScVar, TClVar *Clvar, array *heap);
// ������
PredicateState prstint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
PredicateState prstfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prstlst(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
PredicateState printfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����; �����
PredicateState whatisit(size_t sw, bool (*f)(char), size_t i, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prdel(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������
PredicateState prskol(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������������� � �����
PredicateState print(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �������������� � float
PredicateState prfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prterm(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ������
PredicateState prgt(TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
PredicateState prpaint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ��������
PredicateState pradd(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���
PredicateState prassrt(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����
PredicateState prapp(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prset(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������
PredicateState prmul3(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ���������
PredicateState prmul(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ����������
PredicateState prcircl(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prcopy(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prclaus(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// �����
PredicateState prger(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
//=========== ������� ����� ==============
int Inputstring(char *buf, size_t size, char *caption = 0);
int Inputline(char *buf, size_t size, char *caption = 0);
int InputSymbol(char *buf);
int InputInt(IntegerType *n, const char *caption);
int InputFloat(FloatType *n, const char *caption);

// out for debug
void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);
void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);