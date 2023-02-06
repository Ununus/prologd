#ifndef extfunch
#define extfunch

#include "pstructs.h"

// Проверяет, что количество аргументов во строенном предикате 'name' равно 'narg'
bool bpred(unsigned name, unsigned narg);

unsigned argnull(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argone(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argtwo(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argthree(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argfour(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
unsigned argfive(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);

// получение целого связанного с переменной
IntegerType occ(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// получение вещесв связанного с переменной
FloatType occf(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация целого num с arg аргументом предиката
unsigned zap1(IntegerType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация целых num1 и num2 с arg1 и arg2 соотвественно аргументами предиката
unsigned zap2(IntegerType num1, IntegerType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация float num с arg аргументом предиката
unsigned zap1f(FloatType num, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// запись строки и унификация его с arg
unsigned zap3(const char *str, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);

// ВЫП
int prcall(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЛУЧ
unsigned prrandom(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЖДИ
unsigned prwait(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЗАПИСЬ_В
unsigned outfile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЧТЕНИЕ_ИЗ
unsigned infile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СТРЦЕЛ
unsigned prstint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВКОД
unsigned priocod(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДЦЕЛ
unsigned prrdint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДВЕЩ
unsigned prrdfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДСИМВ
unsigned prrdsym(unsigned sw, TScVar *ScVar, TClVar *Clvar, array *heap);
// СКОЛЬКО
unsigned prskol(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// преобразование в целое
unsigned print(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// преобразование в float
unsigned prfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ТЕРМ
unsigned prterm(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// БОЛЬШЕ
unsigned prgt(TScVar *ScVar, TClVar *ClVar, array *heap);
// ЗАКРАСКА
unsigned prpaint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЛОЖЕНИЕ
unsigned pradd(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ДОБ
unsigned prassrt(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЦЕП
unsigned prapp(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ТОЧКА
unsigned prset(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// УМНОЖЕНИЕ
unsigned prmul3(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// УМНОЖЕНИЕ
unsigned prmul(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ОКРУЖНОСТЬ
unsigned prcircl(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// КОПИЯ
unsigned prcopy(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ПРЕДЛ
unsigned prclaus(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЛИНИЯ
unsigned prger(unsigned long sw, TScVar *ScVar, TClVar *ClVar, array *heap);
  //=========== функции ввода ==============
int Inputstring(char *buf, int size, char *caption = 0);
int InputSymbol(char *buf);
int InputInt(IntegerType *n, const char *caption);
int InputFloat(FloatType *n, const char *caption);

// out for debug
void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);
void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);

#endif
