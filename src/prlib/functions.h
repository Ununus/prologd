#pragma once

#include "pstructs.h"

// ���������������� ������� ��� ��������������� �����������
void Rectangle(long long x1, long long y1, long long x2, long long y2, unsigned color);
void MoveTo_LineTo(long long x1, long long y1, long long x2, long long y2, unsigned color);
void FloodFill(long long x, long long y, unsigned color);
void vertical(long long x1, long long y, long long x2, long long color);
void horisontal(long long x, long long y1, long long y2, long long color);
void ClearView(unsigned color = 15);
void Ellipse(long long x1, long long y1, long long x2, long long y2, unsigned color);
void SetPixel(long long x, long long y, unsigned color);
unsigned GetPixel(long long x, long long y);
// ����
int InputStringFromDialog(char *buf, size_t size, const char *caption, bool splitSpace);
// ����� ��������� (��, ���)
void prdout(bool value);
// ���������������� ����� (���������� �����)
void usrout(const char *str);
// ������������ �����
void pldout(const char *str);
// ����� ������
void errout(const char *str);

//======================
// ����������� ���������� ����������
ErrorCode buildin(TScVar *ScVar, array *heap);
// �������������. ��� ���������� � ��������� ���������
ErrorCode InitProlog();
// ������������ ���� �������
ErrorCode FreeProlog();
// ������� ��� ���������� ��������� ��������� �������
ErrorCode tokbb(TScVar *ScVar, array *heap);
ErrorCode clause(TScVar *ScVar, array *heap);
ErrorCode strings(char *&p, TScVar *ScVar, array *heap);
ErrorCode num(char *&p, TScVar *ScVar, array *heap);
ErrorCode implic(char *&p, TScVar *ScVar, array *heap);
ErrorCode expbeg(TScVar *ScVar, array *heap);
ErrorCode lbeg(char *&p, TScVar *ScVar, array *heap);
ErrorCode list(TScVar *ScVar, array *heap);
ErrorCode arsgn(TScVar *ScVar, size_t i);
//==============================================
bool isalpharus(char symb);
bool isalphaang(char symb);
bool isalphanumang(char symb);
bool isdigitrus(char symb);
bool issvar(char *&p, size_t &len);
bool issconst(char *&p, size_t &len);
//=====������� ��� ������ ========================================
ErrorCode variabletable(char *&p, size_t len, TScVar *ScVar, array *heap);
ErrorCode wrsconst(char *&p, size_t len, TScVar *ScVar, array *heap);
