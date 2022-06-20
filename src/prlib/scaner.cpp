#include "scaner.h"
#include "functions.h"
#include "pdefs.h"
#include "pstructs.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

int scaner(char *Str, TScVar *ScVar, array *heap) {  //������� ��������������� ������� ������
  int err = 0;                                       //�������� ������� ������ � �������
  bool &EndOfClause = ScVar->EndOfClause;
  bool &Query = ScVar->Query;
  unsigned int *buf = ScVar->buf;
  unsigned int &bptr = ScVar->bptr;
  unsigned int *goal = ScVar->goal;
  unsigned int &gptr = ScVar->gptr;
  unsigned int &novar = ScVar->novar;
  unsigned int *tat = ScVar->tat;
  unsigned int *tvar = ScVar->tvar;
  unsigned int &nosymbol = ScVar->nosymbol;
  bool &right = ScVar->right;
  unsigned int &exprip = ScVar->exprip;
  EndOfClause = false;
  char *p = Str;
  while (*p && !err)  //��������� 0 ��� ��� ������ � �����������
  {
    switch (*p) {
    case ',': buf[bptr++] = iscomma; break;  //������ ������� � buf
    case '|': buf[bptr++] = isstick; break;
    case '(': {
      if (!exprip)
        goal[gptr++] = bptr;
      buf[bptr++] = isbbeg;
    } break;  //����� �������
    case ')': {
      buf[bptr++] = isbend;
      if (!exprip)
        err = tokbb(ScVar, heap);
    } break;
    case ':':
    case '<': err = implic(p, ScVar, heap); break;
    case '%':
      while (*p != '\n' && *p)
        p++;
      break;
    case ';':
    case '.': {
      buf[bptr++] = isend;
      err = clause(ScVar, heap);
      right = false;
      bptr = gptr = exprip = novar = 0;
      if (Query) {
        if (*p)
          p++;
        return err;
      }  //��� ������ ��� ������������� tfileedit
    } break;
    case '#': err = expbeg(ScVar, heap); break;   //����� ���������
    case '[': err = lbeg(p, ScVar, heap); break;  //������
    case ']':
      buf[bptr++] = islend;
      err = list(ScVar, heap);
      break;
    case '\"': err = strings(p, ScVar, heap); break;  //������ ��������
    case '?':                                         //������
    {
      if (bptr || right)
        err = 10;
      else
        Query = right = true;
    } break;
    case '\n':
    case '\r':
    case ' ': break;  //�� ����� �������� - ������� �� ��������� ������
    case '_':         //��������
    {
      if (gptr <= 0)
        err = 11;
      else if (bptr + 1 < _maxbptr_)
        buf[bptr++] = ScVar->hpunkn;
      else
        err = 22;
    } break;
    case '!':
      if (!right)
        err = 12;
      else if (bptr + 1 < _maxbptr_)
        buf[bptr++] = hpcut;
      else
        err = 22;
      break;
    case '+': {
      err = arsgn(ScVar, isplus);
    } break;
    case '-': {
      if (exprip && bptr &&
          (buf[bptr - 1] != islbeg && buf[bptr - 1] != isexpress && buf[bptr - 1] != isbbeg && buf[bptr - 1] != ismult && buf[bptr - 1] != isslash))
        err = arsgn(ScVar, isminus);
      else
        err = num(p, ScVar, heap);
    } break;
    case '*': err = arsgn(ScVar, ismult); break;
    case '/':
      err = arsgn(ScVar, isslash);
      break;
      //������� �������������� ������� ������ ����������� � expresse

    default: {
      unsigned int len = 0;
      if (issvar(p, len)) {
        err = variabletable(p, len, ScVar, heap);
        p += len - 1;
      } else {
        if (issconst(p, len)) {
          err = wrsconst(p, len, ScVar, heap);
          p += len - 1;
        } else {  //�����
          if (*p >= '0' && *p <= '9' || *p == '-' || *p == '+')
            err = num(p, ScVar, heap);
          else
            err = 1;  //������ �� �������� ����������
        }
      }
    }
    }
    if (*p)
      p++;  //��� ������ �� ����� ��������� lptr ������ ���������
  }         //�� ��������� ������������ ������
  if (err) {
    char Mess[1024];
    // sprintf(Mess, "������ ��� ������� ������: %s", Str);
    sprintf(Mess, "������ ��� �������: %s", Str);
    // out(Mess);
    errout(const_cast<const char *>(Mess));
  }
  return err;
}
