#pragma once

#include "pstructs.h"

// Пользовательские функции для переопределения интерфейсом
void Rectangle(IntegerType x1, IntegerType y1, IntegerType x2, IntegerType y2, unsigned color);
void MoveTo_LineTo(IntegerType x1, IntegerType y1, IntegerType x2, IntegerType y2, unsigned color);
void FloodFill(IntegerType x, IntegerType y, unsigned color);
void vertical(IntegerType x1, IntegerType y, IntegerType x2, IntegerType color);
void horisontal(IntegerType x, IntegerType y1, IntegerType y2, IntegerType color);
void ClearView(unsigned color = 15);
void Ellipse(IntegerType x1, IntegerType y1, IntegerType x2, IntegerType y2, unsigned color);
void SetPixel(IntegerType x, IntegerType y, unsigned color);
unsigned GetPixel(IntegerType x, IntegerType y);
int InputStringFromDialog(char *Buf, size_t BufSize, char *pCaption);
// Вывод предиката (ДА, НЕТ)
void prdout(bool value);
// Пользовательский вывод (предикатом ВЫВОД)
void usrout(const char *str);
// Прологовский вывод
void pldout(const char *str);
// Вывод ошибок
void errout(const char *str);

//======================
// Подключение встроенных предикатов
ErrorCode buildin(TScVar *ScVar, array *heap);
// Инициализация. Все приводится к исходному состоянию
ErrorCode InitProlog();
// Освобождение всех буферов
ErrorCode FreeProlog();
// Функции для построения элементов программы пролога
ErrorCode tokbb(TScVar *ScVar, array *heap);
ErrorCode clause(TScVar *ScVar, array *heap);
ErrorCode strings(char *&p, TScVar *ScVar, array *heap);
ErrorCode num(char *&p, TScVar *ScVar, array *heap);
ErrorCode implic(char *&p, TScVar *ScVar, array *heap);
ErrorCode expbeg(TScVar *ScVar, array *heap);
ErrorCode lbeg(char *&p, TScVar *ScVar, array *heap);
ErrorCode list(TScVar *ScVar, array *heap);
ErrorCode arsgn(TScVar *ScVar, unsigned int i);
//==============================================
bool isalpharus(char symb);
bool isalphaang(char symb);
bool isalphanumang(char symb);
bool isdigitrus(char symb);
bool issvar(char *&p, unsigned int &len);
bool issconst(char *&p, unsigned int &len);
//=====функции для записи ========================================
ErrorCode variabletable(char *&p, unsigned int len, TScVar *ScVar, array *heap);
ErrorCode wrsconst(char *&p, unsigned int len, TScVar *ScVar, array *heap);
