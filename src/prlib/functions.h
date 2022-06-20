#ifndef functionsh
#define functionsh
#include "pstructs.h"
//======================
int buildin(TScVar *ScVar, array *heap);
int InitProlog();  //Инициализация. Все приводится к исходному состоянию
int FreeProlog();  // освобождение всех буферов
//функции для построения элементов программы пролога
int tokbb(TScVar *ScVar, array *heap);

// unsigned int inplic();
int clause(TScVar *ScVar, array *heap);
int strings(char *&p, TScVar *ScVar, array *heap);
int num(char *&p, TScVar *ScVar, array *heap);
bool isalpharus(char symb);
int symvar(char *&p);
int implic(char *&p, TScVar *ScVar, array *heap);
int expbeg(TScVar *ScVar, array *heap);
int lbeg(char *&p, TScVar *ScVar, array *heap);
int list(TScVar *ScVar, array *heap);
int arsgn(TScVar *ScVar, unsigned int i);
//==============================================
bool isalpharus(char symb);
bool isalphaang(char symb);
bool isalphanumang(char symb);
bool isdigitrus(char symb);
bool issvar(char *&p, unsigned int &len);
bool issconst(char *&p, unsigned int &len);

//=====функции для записи ========================================
int variabletable(char *&p, unsigned int len, TScVar *ScVar, array *heap);
int wrsconst(char *&p, unsigned int len, TScVar *ScVar, array *heap);

#endif
