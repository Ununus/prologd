#include "scaner.h"
#include "functions.h"
#include "pdefs.h"
#include "pstructs.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

int scaner(char *Str, TScVar *ScVar, array *heap) {  //функция синтаксического разбора строки
  int err = 0;                                       //вызывает функции записи в таблицы
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
  while (*p && !err)  //возвратит 0 или код ошибки в конструкции
  {
    switch (*p) {
    case ',': buf[bptr++] = iscomma; break;  //запись запятой в buf
    case '|': buf[bptr++] = isstick; break;
    case '(': {
      if (!exprip)
        goal[gptr++] = bptr;
      buf[bptr++] = isbbeg;
    } break;  //здесь отличие
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
      }  //эта строка при использовании tfileedit
    } break;
    case '#': err = expbeg(ScVar, heap); break;   //арифм выражение
    case '[': err = lbeg(p, ScVar, heap); break;  //список
    case ']':
      buf[bptr++] = islend;
      err = list(ScVar, heap);
      break;
    case '\"': err = strings(p, ScVar, heap); break;  //строка символов
    case '?':                                         //вопрос
    {
      if (bptr || right)
        err = 10;
      else
        Query = right = true;
    } break;
    case '\n':
    case '\r':
    case ' ': break;  //ни каких действий - переход на следующий символ
    case '_':         //анонимка
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
      //символы арифметических функций должны обработатся в expresse

    default: {
      unsigned int len = 0;
      if (issvar(p, len)) {
        err = variabletable(p, len, ScVar, heap);
        p += len - 1;
      } else {
        if (issconst(p, len)) {
          err = wrsconst(p, len, ScVar, heap);
          p += len - 1;
        } else {  //цифры
          if (*p >= '0' && *p <= '9' || *p == '-' || *p == '+')
            err = num(p, ScVar, heap);
          else
            err = 1;  //символ не возможно обработать
        }
      }
    }
    }
    if (*p)
      p++;  //при выходе из любой процедуры lptr должен указывать
  }         //на последний обработанный символ
  if (err) {
    char Mess[1024];
    // sprintf(Mess, "Ошибка при разборе строки: %s", Str);
    sprintf(Mess, "Ошибка при разборе: %s", Str);
    // out(Mess);
    errout(const_cast<const char *>(Mess));
  }
  return err;
}
