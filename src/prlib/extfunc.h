#pragma once

#include "pstructs.h"

// Проверяет, что количество аргументов во строенном предикате 'name' равно 'narg'
bool bpred(unsigned name, unsigned narg);

PredicateState argnull(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argone(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argtwo(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argthree(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfour(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfive(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap);

// получение целого связанного с переменной
IntegerType occ(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// получение вещесв связанного с переменной
FloatType occf(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация целого num с arg аргументом предиката
PredicateState zap1(IntegerType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация целых num1 и num2 с arg1 и arg2 соотвественно аргументами предиката
PredicateState zap2(IntegerType num1, IntegerType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация float num с arg аргументом предиката
PredicateState zap1f(FloatType num, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// запись строки и унификация его с arg
PredicateState zap3(const char *str, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap);

// ВЫП
PredicateState prcall(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЛУЧ
PredicateState prrandom(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЖДИ
PredicateState prwait(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЗАПИСЬ_В
PredicateState outfile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЧТЕНИЕ_ИЗ
PredicateState infile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВКОД
PredicateState priocod(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДЦЕЛ
PredicateState prrdint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДВЕЩ
PredicateState prrdfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДСИМВ
PredicateState prrdsym(unsigned sw, TScVar *ScVar, TClVar *Clvar, array *heap);
// СТРЦЕЛ
PredicateState prstint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СТРВЕЩ
PredicateState prstfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СТРСПИС
PredicateState prstlst(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЦЕЛВЕЩ
PredicateState printfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// БУКВА; ЦИФРА
PredicateState whatisit(unsigned sw, bool (*f)(char), unsigned i, TScVar *ScVar, TClVar *ClVar, array *heap);
// УДАЛ
PredicateState prdel(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СКОЛЬКО
PredicateState prskol(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// преобразование в целое
PredicateState print(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// преобразование в float
PredicateState prfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ТЕРМ
PredicateState prterm(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// БОЛЬШЕ
PredicateState prgt(TScVar *ScVar, TClVar *ClVar, array *heap);
// ЗАКРАСКА
PredicateState prpaint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЛОЖЕНИЕ
PredicateState pradd(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ДОБ
PredicateState prassrt(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЦЕП
PredicateState prapp(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ТОЧКА
PredicateState prset(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// УМНОЖЕНИЕ
PredicateState prmul3(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// УМНОЖЕНИЕ
PredicateState prmul(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ОКРУЖНОСТЬ
PredicateState prcircl(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// КОПИЯ
PredicateState prcopy(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ПРЕДЛ
PredicateState prclaus(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЛИНИЯ
PredicateState prger(unsigned long sw, TScVar *ScVar, TClVar *ClVar, array *heap);
//=========== функции ввода ==============
int Inputstring(char *buf, int size, char *caption = 0);
int InputSymbol(char *buf);
int InputInt(IntegerType *n, const char *caption);
int InputFloat(FloatType *n, const char *caption);

// out for debug
void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);
void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);