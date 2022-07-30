#ifndef defsh
#define defsh
//#include <limits.h>
//идентификаторы структур элементов
#define isvar 0
#define issymbol 1
#define isinteger 2
#define isfloat 3
#define isexpression 4
#define isunknown 5
#define isemptylist 6
#define islist 7
#define isfunction 8
#define isstring 9
#define isclause 10
#define isclauseq 11  // gue
#define iscut 12
// для опций вывода
#define tscreen 0
#define gscreen 1
#define printer 2
#define outfiles 3
#define userfiles 4
// для опций исполнения
#define tracce 1    //трасса
#define outque 2    //вывод вопроса
#define autosave 4  //эти определения для опций исполнения
#define maxarity 5  //пока max арность встроенного предиката
//=================================

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
#define no_memory_out 1          //нет памяти для вывода
#define no_memory_stac 2         //переполнение стека программы

#define maxlinelen 1024  //длина строки редактора,и всякие промежуточные буфера

#define maxbf 65536
#define maxstaccalc 3200
//#define maxbf 2048       //размер стека унификации
//#define maxstaccalc 100  //стек для вычисления ар выр
#define maxgrx 1024  // max координата для скроллера графики X
#define maxgry 768   //                                     Y

//==============идернтификаторы графических функций ======
#define pbakground 1  //фон
#define ppixel 2      //точка
#define pline 3       //линия
#define pcirkle 4     //окружность
#define ppaint 5      //закраска
#endif
