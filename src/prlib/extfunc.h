#pragma once

#include "pstructs.h"

// Проверяет, что количество аргументов во строенном предикате 'name' равно 'narg'
bool bpred(size_t name, size_t narg);

PredicateState argnull(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argone(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argtwo(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argthree(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfour(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);
PredicateState argfive(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap);

// получение целого связанного с переменной
IntegerType occ(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap);
// получение вещесв связанного с переменной
FloatType occf(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация целого num с arg аргументом предиката
PredicateState zap1(IntegerType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация целых num1 и num2 с arg1 и arg2 соотвественно аргументами предиката
PredicateState zap2(IntegerType num1, IntegerType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap);
// унификация float num с arg аргументом предиката
PredicateState zap1f(FloatType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);
// запись строки и унификация его с arg
PredicateState zap3(const char *str, int arg, TScVar *ScVar, TClVar *ClVar, array *heap);

// ВЫП
PredicateState prcall(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЛУЧ
PredicateState prrandom(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЖДИ
PredicateState prwait(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЗАПИСЬ_В
PredicateState outfile(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЧТЕНИЕ_ИЗ
PredicateState infile(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВКОД
PredicateState priocod(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДЦЕЛ
PredicateState prrdint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДВЕЩ
PredicateState prrdfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ВВОДСИМВ
PredicateState prrdsym(size_t sw, TScVar *ScVar, TClVar *Clvar, array *heap);
// СТРЦЕЛ
PredicateState prstint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СТРВЕЩ
PredicateState prstfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СТРСПИС
PredicateState prstlst(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЦЕЛВЕЩ
PredicateState printfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// БУКВА; ЦИФРА
PredicateState whatisit(size_t sw, bool (*f)(char), size_t i, TScVar *ScVar, TClVar *ClVar, array *heap);
// УДАЛ
PredicateState prdel(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СКОЛЬКО
PredicateState prskol(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// преобразование в целое
PredicateState print(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// преобразование в float
PredicateState prfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ТЕРМ
PredicateState prterm(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// БОЛЬШЕ
PredicateState prgt(TScVar *ScVar, TClVar *ClVar, array *heap);
// ЗАКРАСКА
PredicateState prpaint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЛОЖЕНИЕ
PredicateState pradd(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ДОБ
PredicateState prassrt(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// СЦЕП
PredicateState prapp(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ТОЧКА
PredicateState prset(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// УМНОЖЕНИЕ
PredicateState prmul3(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// УМНОЖЕНИЕ
PredicateState prmul(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ОКРУЖНОСТЬ
PredicateState prcircl(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// КОПИЯ
PredicateState prcopy(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ПРЕДЛ
PredicateState prclaus(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
// ЛИНИЯ
PredicateState prger(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap);
//=========== функции ввода ==============
int Inputstring(char *buf, size_t size, char *caption = 0);
int Inputline(char *buf, size_t size, char *caption = 0);
int InputSymbol(char *buf);
int InputInt(IntegerType *n, const char *caption);
int InputFloat(FloatType *n, const char *caption);

// out for debug
void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);
void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap);