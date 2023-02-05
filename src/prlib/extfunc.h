#ifndef extfunch
#define extfunch

#include "pstructs.h"

unsigned argnull(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned prout();
bool bpred(unsigned name, unsigned narg);
unsigned argone(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argtwo(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argthree(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argfour(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argfive(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
void phon(int a, int b);
IntegerType occ(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);     //��������� ������ ���������� � ����������
FloatType occf(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);  //��������� ������ ���������� � ����������

unsigned zap1(IntegerType num, unsigned arg, TScVar *ScVar, TClVar *ClVar,
              array *heap);  //���������� ������ num � arg ���������� ���������

//���������� ������ num1 � arg1 ���������� ���������
//���������� ������ num2 � arg2 ���������� ���������
unsigned zap2(IntegerType num1, IntegerType num2, unsigned arg1, unsigned arg2, TScVar *ScVar, TClVar *ClVar, array *heap);

unsigned zap1f(FloatType num, unsigned arg, TScVar *ScVar, TClVar *ClVar,
               array *heap);  //���������� float num � arg ���������� ���������

unsigned outfile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);

unsigned prstint(unsigned sw, TScVar *ScVar, TClVar *ClVar,
                 array *heap);  //������
unsigned priocod(unsigned sw, TClVar *ClVar);
unsigned prrdint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);  //���� ������   !!!����� ���� ������
unsigned prmem(unsigned sw);                                               //�� ������� ��� ���� ����� ����� �-���
unsigned prrdsym(unsigned sw, TScVar *ScVar, TClVar *Clvar,
                 array *heap);  //��������
unsigned prskol(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned print(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);    //�������������� � �����
unsigned prfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);  //�������������� � float
unsigned prterm(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);   //�� ����� ��� ���� ��� ������� 5
unsigned argone(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned prgt(TScVar *ScVar, TClVar *ClVar, array *heap);
//=========== ������� ����� ==============
int Inputstring(char *buf, int size, char *caption = 0);
int InputSymbol(char *buf);
int InputInt(IntegerType *n, const char *caption);
int InputFloat(FloatType *n, const char *caption);

// out for debug
void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);
void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);

#endif
