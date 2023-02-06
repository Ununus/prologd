#pragma once
//#include <limits.h>
//идентификаторы структур элементов
constexpr size_t isvar = 0;
constexpr size_t issymbol = 1;
constexpr size_t isinteger = 2;
constexpr size_t isfloat = 3;
constexpr size_t isexpression = 4;
constexpr size_t isunknown = 5;
constexpr size_t isemptylist = 6;
constexpr size_t islist = 7;
constexpr size_t isfunction = 8;
constexpr size_t isstring = 9;
constexpr size_t isclause = 10;
constexpr size_t isclauseq = 11;
constexpr size_t iscut = 12;

constexpr size_t maxarity = 5;  // пока max арность встроенного предиката

//константы для записи в массив buf
#define isnil INT_MAX            //не значит ничего
#define isbase INT_MAX - 17      //
#define iscomma INT_MAX - 16     //     ,
#define isstick INT_MAX - 15     //     |
#define isbbeg INT_MAX - 14      //     (
#define isbend INT_MAX - 13      //     )
#define isimpl INT_MAX - 12      // :-  <-
#define isend INT_MAX - 11       // . ;
#define isexpress INT_MAX - 10   // #
#define islbeg INT_MAX - 9       // [
#define islend INT_MAX - 8       // ]
#define isunitminus INT_MAX - 7  // -
#define isminus INT_MAX - 6      // -
#define isslash INT_MAX - 5      // /
#define ismult INT_MAX - 4       // *
#define isplus INT_MAX - 3       // +
#define ismod INT_MAX - 2        // mod
#define isdiv INT_MAX - 1        // div

#define maxlinelen 1024  //длина строки редактора,и всякие промежуточные буфера

#define maxbf 65536
#define maxstaccalc 3200
//#define maxbf 2048       //размер стека унификации
//#define maxstaccalc 100  //стек для вычисления ар выр
#define maxgrx 1024  // max координата для скроллера графики X
#define maxgry 768   //                                     Y