#include "err.h"
#include <stdio.h>

const char *errors[] = {
  "������ �� ����������",                                // 0
  "������������ ������",                                 // 1
  "������������ ��������� ������",                       // 2
  "�������� ������ ������",                              // 3
  "�������� ������ �����",                               // 4
  "��� �����������",                                     // 5
  "����������� \"-\"",                                   // 6
  "����������� \")\" ��� ��� \":-(<-)\"",                // 7
  "�������� ������������� \":-(<-)\"",                   // 8
  "������ �� ����� ���� ����������",                     // 9
  "�������� ������������� \"?\"",                        // 10
  "�������� ������������� \"_\"",                        // 11
  "�������� ������������� \"!\"",                        // 12
  "����� ������� �����������(������������)",             // 13
  "�� ������ ������ ��� ��������� ������",               // 14
  "������������ ������",                                 // 15
  "�� ������ ������ ��� ��������� �������",              // 16
  "����� \"(\" ������ ���� �������",                     // 17 ������ ���� issymbol
  "���� �� � �������������� ���������",                  // 18
  "������ � ��������� ��������������� ���������",        // 19
  "������ ��� ������� �����������",                      // 20
  "�� ���� ������� ���� '*.OUT'",                        // 21
  "�� ���� ������� �������",                             // 22
  "������� ����� ���������� ��������",                   // 23
  "����������� ������",                                  // 24
  "�� ������������� ��������",                           // 25
  "�� ������ ����� ���������� �� ���������� ���������",  // 26
  "���������� ��������",                                 // 27
  "����������� �����",                                   // 28
  "� '���������' ������������� ������� ������� �����",   // 29
  "������������ '���������'(�������� ���������)",        // 30
  "�� ���������� �������� '��������'",                   // 31
  "������ � �������������� ���������",                   // 32
  "������ �����/������",                                 // 33
  "�������� 'mod' ��������� � ������������� ��������",   // 34
  "�������� 'div' ��������� � ������������� ��������",   // 35
  "������ �� ���������� ��������� 'int'",                // 36
  "������ �� ���������� ��������� 'float'",              // 37
  "������ ��� ������� �������",                          // 38
  "������������ ������ ��� ������ ���������",            // 39
  "������������ ����� ��� ���������� �����. ���������",  // 40
  "������� �� 0 ��� ���������� �����. ���������",        // 41
  "������������ ������ ���� ���������� ��������",        // 42
  "�������� ���������� � �������� ����������",           // 43
  "������� ������� ������"                               // 44
};

char *GetPrErrText(int err) {
  static char ErrBuf[255];
  char *p = const_cast<char *>("");
  if (err < 0 || err > 43)
    sprintf(ErrBuf, "Unknown error (code = %d)", err);
  else
    sprintf(ErrBuf, "%s (%d)", errors[err], err);
  p = ErrBuf;
  return p;
}
