#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "functions.h"
#include "pdefs.h"
// #include "extdecl.h"
#include "scaner.h"
// #include "control.h"
#include "extfunc.h"
// #include <charconv>

//=======Первоначальное заполнение
const char *NamesOfPredicates[] = { "mod",    "ЛОЖЬ",      "ТРАССА",    "НЕТ_ТРАССЫ", "!",        "ВЫП",   "ВВОДСИМВ", "ВВОДСТР",  "ВВОДЦЕЛ", "ВВОДВЕЩ",
                                    "ВВКОД",  "ЧТЕНИЕ_ИЗ", "ЗАПИСЬ_В",  "ПЕР",        "ЦЕЛ",      "ВЕЩ",   "СИМВ",     "СПИСОК",   "ВЫВОД",   "БОЛЬШЕ",
                                    "СТРЦЕЛ", "СТРВЕЩ",    "СТРСПИС",   "ЦЕЛВЕЩ",     "БУКВА",    "ЦИФРА", "ТЕРМ",     "УДАЛЕНИЕ", "СКОЛЬКО", "ТОЧКА",
                                    "СЦЕП",   "ДОБ",       "УМНОЖЕНИЕ", "ОКРУЖНОСТЬ", "ЗАКРАСКА", "КОПИЯ", "ПРЕДЛ",    "ЛИНИЯ",    "СЛУЧ",    "СЛОЖЕНИЕ",
                                    "ЖДИ",    "div",       "int",       "float",      "ТИХО",     "ВЕРСИЯ" };
const size_t MAX_BUILD_PRED = sizeof(NamesOfPredicates) / sizeof(char *);

const char *Build_in_Libriary[] = {
  "НЕ(x)<-ВЫП(x),!,ЛОЖЬ;",
  "НЕ(_);",
  "ИЛИ(x,_)<-ВЫП(x),!;",
  "ИЛИ(_,x)<-ВЫП(x);",
  "РАВНО(x,x);",
  "МЕНЬШЕ(x,y)<-БОЛЬШЕ(y,x);",
  "СЛОЖЕНИЕ(x,y,z)<-УМНОЖЕНИЕ(1,x,y,z);",
  "ПС<-ВВКОД(10);",
  "ДЛИНА(x,y)<-КОПИЯ(x,1,y,x);",
};
const size_t LEN_BUILD_LIBR = sizeof(Build_in_Libriary) / sizeof(char *);

// подключение встроенных предикатов
ErrorCode buildin(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  for (size_t i = 0; i < MAX_BUILD_PRED; i++) {
    size_t len = strlen(NamesOfPredicates[i]);
    auto index = heap->append<char>(0, len);
    memcpy(heap->GetPchar(index), NamesOfPredicates[i], sizeof(char) * len);
    ScVar->tat[i] = heap->append(recordsconst(index, len));
  }
  // freesymbol = MAX_BUILD_PRED;
  ScVar->nosymbol = MAX_BUILD_PRED;
  ScVar->hpunkn = heap->append(recordunknown());     // положение анонимки в heap
  ScVar->hpempty = heap->append(recordemptylist());  // положение пустого списка
  heap->freeheap = heap->last;

  // extern char*Build_in_Libriary[] ;
  for (int i = 0; i < LEN_BUILD_LIBR && err == ErrorCode::NoErrors; i++) {
    char *p = (char *)Build_in_Libriary[i];
    err = scaner(p, ScVar, heap);
  }
  return err;
}

// Инициализация. Все приводится к исходному состоянию
ErrorCode InitProlog() {
  ErrorCode err = ErrorCode::NoErrors;  // нет ошибок
  return err;
}

ErrorCode FreeProlog() {
  return ErrorCode::NoErrors;
}

// функции для построения элементов программы пролога
ErrorCode tokbb(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  auto &bptr = ScVar->bptr;
  auto *buf = ScVar->buf;
  auto *goal = ScVar->goal;
  auto &gptr = ScVar->gptr;
  if (gptr == 0 || gptr >= _maxgptr_ || buf[goal[gptr - 1]] != isbbeg) {
    return ErrorCode::UnpairedBracketsInFunctionProcessing;  // 16; //нет откр скобки или не issymbol
  }
  --gptr;
  auto i = goal[gptr];  // индекс в массиве buf указывающий на (
  recordsconst *ptr;
  if (buf[--i] > isbase) {
    return ErrorCode::AbsentFunctorBeforeOpenBracket;  // 17;
  }
  ptr = heap->GetPrecordsconst(buf[i]);
  if (ptr->ident != issymbol) {
    return ErrorCode::AbsentFunctorBeforeOpenBracket;  // 17;
  }
  i += 2;
  if ((bptr - i) % 2) {
    return ErrorCode::ErrorWhileParsingFunction;  // 38;
  }
  auto n = (bptr - i) / 2;  // число аргументов
  size_t ptrargs[1024]; // TODO: do vector
  if (n > sizeof(ptrargs) / sizeof(size_t)) {
    return ErrorCode::NotEnoughFreeMemory;  // 2;
  }
  for (size_t j = 0; j < n; j++) {
    if ((ptrargs[j] = buf[i + j * 2]) > isbase) {
      err = ErrorCode::ErrorWhileParsingFunction;  // 38;
    }
  }
  // аргументы в массив
  auto index = heap->append<size_t>(0, n);
  memcpy(heap->GetPunsigned(index), ptrargs, sizeof(size_t) * n);
  i -= 2;
  if (err != ErrorCode::NoErrors) {
    return err;
  }
  index = heap->append(recordfunction(n, buf[i], index));
  bptr = i;
  if (bptr + 1 < _maxbptr_) {
    buf[bptr++] = index;
  } else {
    // err = ErrorCode::CannotOpenGraphics;  // 22;
    err = ErrorCode::TooManyCharacterConstants;
  }
  return err;
}

ErrorCode clause(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  bool &Query = ScVar->Query;
  bool &EndOfClause = ScVar->EndOfClause;
  auto &gptr = ScVar->gptr;
  auto *buf = ScVar->buf;
  auto &novar = ScVar->novar;
  auto id = Query ? isclauseq : isclause;
  if (gptr != NULL) {
    err = ErrorCode::UnpairedBracketsInFunctionProcessing;  // 16;
  }
  //!!! здесь нужны проверки на другие ошибки
  baserecord *tp;
  size_t ntarget = 1;  // число целей
  int i = 1, j = 0;
  int n = 0;
  while (buf[n] != isend) {
    n++;
  }
  // на первом месте должна быть симв константа или func
  if (buf[0] >= heap->last) {
    err = ErrorCode::ErrorWhileParsingSentence;  // 20;  //ошибка при разборе предл
  } else {
    tp = heap->GetPbaserecord(buf[0]);
    if (tp->ident != isfunction && tp->ident != issymbol) {
      err = ErrorCode::ErrorWhileParsingSentence;  // 20;
    }
  }
  while (i < n && err == ErrorCode::NoErrors) {
    if (buf[i] == iscomma || (buf[i] == isimpl && i == 1)) {
      // ;
    } else {
      err = ErrorCode::ErrorWhileParsingSentence;  // 20;
    }
    if (buf[i - 1] >= heap->last) {
      err = ErrorCode::ErrorWhileParsingSentence;  // 20;
    } else {
      tp = heap->GetPbaserecord(buf[i - 1]);
      auto ident = tp->ident;
      if (ident != isfunction && ident != issymbol) {
        err = ErrorCode::ErrorWhileParsingSentence;  // 20;
      }
    }
    i += 2;
  }
  i = 1;
  while (buf[i] != isend && err == ErrorCode::NoErrors) {
    if (buf[++i] > isbase) {
      err = ErrorCode::ErrorWhileParsingSentence;  // 20;
    }
    i++;
    ntarget++;
  }
  if (err == ErrorCode::NoErrors) {
    size_t ptarget[1024]; // TODO: do vector
    if ((ntarget + 1) * sizeof(size_t) > sizeof(ptarget)) {
      return ErrorCode::NotEnoughFreeMemory;  // 2;
    }
    auto index = heap->append<size_t>(0, (ntarget + 1));
    // memcpy(heap->GetPunsigned(index), ptarget, sizeof(size_t) * (ntarget + 1));
    size_t *_ptarget = heap->GetPunsigned(index);
    recordclause *ptrpred = 0;
    index = heap->append(recordclause(id, 0, novar, buf[0], index));
    recordclause *ptr = heap->GetPrecordclause(index);
    // теперь перенести цели
    i = 0;
    j = 0;
    while (j < (int)ntarget) {
      *(_ptarget + j++) = buf[i];
      i += 2;
    }
    *(_ptarget + j) = NULL;
    // п8оискать предыдущее одноименное предложение
    if (!Query) {
      // i=j=heap->indexOf(ptr)-1;j++;i--;//j-индекс данного предложения в heap
      // в ptr - указатель на предложение в index индекс данного предложения
      recordfunction *pf = heap->GetPrecordfunction(ptr->head);
      recordsconst *ps;
      if (pf->ident == issymbol) {
        ps = (recordsconst *)pf;
      } else {
        ps = heap->GetPrecordsconst(pf->func);
      }
      if (ps->begin != NULL && ps->begin != isnil) {
        ptrpred = heap->GetPrecordclause(ps->begin);
        while (ptrpred->next != NULL && ptrpred->next != isnil)
          ptrpred = heap->GetPrecordclause(ptrpred->next);
        ptrpred->next = index;
      } else {
        ps->begin = index;
      }
    } else {
      heap->query = index;
    }
  }

  if (err == ErrorCode::NoErrors) {
    EndOfClause = true;
  }

  return err;
}
//=======
ErrorCode strings(char *&p, TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  p++;
  int i;
  for (i = 0; (*(p + i) && *(p + i) != '\"'); i++)
    ;
  if (*(p + i) == '\"') {
    err = wrsconst(p, i, ScVar, heap);  // err=savestring(i);
  } else {
    err = ErrorCode::InvalidStringFormat;  // 3;
  }
  p += i;
  return err;
}

ErrorCode num(char *&p, TScVar *ScVar, array *heap) {
  // должна прийти первая цифра или знак + -
  ErrorCode err = ErrorCode::NoErrors;
  auto &bptr = ScVar->bptr;
  auto *buf = ScVar->buf;
  char bufnum[1024]{};
  FloatType valuef = 0;
  IntegerType valuei = 0;
  std::ptrdiff_t punkt = 0, e = 0;
  std::ptrdiff_t i = 0;
  if (*p == '+' || *p == '-') {
    bufnum[i++] = *p;
  }
  for (; ((isdigitrus(bufnum[i] = *(p + i))) || (*(p + i) == 'e' && punkt) || (*(p + i) == 'E' && punkt) || (*(p + i) == '-' && e) || (*(p + i) == '+' && e) ||
          (*(p + i) == '.'));
       i++) {
    if ((*(p + i) == 'e' || *(p + i) == 'E') && punkt) {
      e++;
    }
    if (*(p + i) == '.') {
      punkt++;
    }
  }
  if (punkt > 1 || e > 1) {
    err = ErrorCode::InvalidNumberFormat;  // 4;
  }
  if (err == ErrorCode::NoErrors) {
    bufnum[i] = 0;
    if (punkt || e) {
      // std::from_chars(bufnum, bufnum + sizeof(bufnum), valuef); // в gcc не реализовано
      valuef = atof(bufnum);
    } else {
      // std::from_chars(bufnum, bufnum + sizeof(bufnum), valuei);
      // valuei = atoll(bufnum);
      valuei = IntegerType(bufnum);
    }
    if ((valuef == valuei.convert_to<FloatType>()) && (i > 1 || (i == 1 && (*p != '0')))) {
      err = ErrorCode::InvalidNumberFormat;  // 4;  // не верный формат числа
    } else {
      if (punkt || e) {
        auto index = heap->append(recordfloat(valuef));
        if (bptr + 1 < _maxbptr_) {
          buf[bptr++] = index;
        } else {
          // err = ErrorCode::CannotOpenGraphics;  // 22;
          err = ErrorCode::TooManyCharacterConstants;
        }
      } else {
        auto index = heap->append(recordinteger(valuei));
        if (bptr + 1 < _maxbptr_) {
          buf[bptr++] = index;
        } else {
          // err = ErrorCode::CannotOpenGraphics;  // 22;
          err = ErrorCode::TooManyCharacterConstants;
        }
      }
    }
  }
  p += i - 1;
  return err;
}

bool isalphaang(char symb) {
  if ((symb >= 'A' && symb <= 'Z') || (symb >= 'a' && symb <= 'z')) {
    return true;
  }
  return false;
}

bool isalphanumang(char symb) {
  if ((symb >= 'A' && symb <= 'Z') || (symb >= 'a' && symb <= 'z') || (symb >= '0' && symb <= '9')) {
    return true;
  }
  return false;
}

bool isdigitrus(char symb) {
  if (symb >= '0' && symb <= '9') {
    return true;
  }
  return false;
}

bool isalpharus(char symb) {
  if ((symb >= 'А' && symb <= 'п') || (symb >= 'р' && symb <= 'я') || symb == 'Ё' || symb == 'ё') {
    return true;
  }
  return false;
}

bool issvar(char *&p, size_t &len) {
  if (isdigitrus(*p)) {
    return false;  // first symbol must be no digit
  }
  for (len = 0; *(p + len) != 0 && (isalphanumang(*(p + len)) || isalpharus(*(p + len)) || *(p + len) == '_'); len++)
    ;
  if (!len) {
    return false;
  }
  if (len == 1) {
    if (*(p + len) == '\'') {
      len++;
    }
    return true;
  } else {
    if (*(p + len) == '\'') {
      len++;
      return true;
    }
  }
  return false;
}

bool issconst(char *&p, size_t &len) {
  len = 0;
  if ((*p >= '0' && *p <= '9') || *p == '-' || *p == '+') {
    return false;
  }
  for (len = 0; *(p + len) != 0 && (isalphanumang(*(p + len)) || isalpharus(*(p + len)) || *(p + len) == '_'); len++)
    ;
  if (!len) {
    return false;
  }
  if (len == 1) {
    if (*(p + len) == '\'') {
      len++;
    }
    return false;
  } else {
    if (*(p + len) == '\'') {
      len++;
      return false;
    }
  }
  return true;
}

ErrorCode implic(char *&p, TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;

  if (*(p + 1) != '-') {
    err = ErrorCode::AbsentMinus;  // 6;
  } else if (ScVar->gptr || ScVar->right) {
    err = ErrorCode::AbsentClosingBraceOrDef;  // 7;
  } else if (ScVar->bptr != 1) {
    err = ErrorCode::IncorrectUseDef;  // 8;
  } else {
    p++;
    ScVar->right = true;
    ScVar->buf[ScVar->bptr++] = isimpl;
  }
  return err;
}
//===========
size_t prioritet(size_t ch) {
  size_t value = 0;
  switch (ch) {
  case ismod:  // остаток от деления
  case isdiv:  // целочисленное деление
  case ismult:
  case isslash: value = 4; break;
  case isbbeg: value = 1; break;
  case isbend: value = 2; break;
  case isplus:
  case isminus: value = 3; break;
  default: value = 0;
  }
  return value;
}

//=============
ErrorCode arexpr(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  size_t obuf[maxlinelen];
  size_t stac[maxlinelen];
  size_t st, n, p;  // индекс в стеке,число эл,приоритет
  ScVar->bptr = ScVar->exprip;
  //========проверка синтаксиса в ар выр
  int sk = 0;  // скобки
  while (ScVar->buf[ScVar->bptr] != isnil && err == ErrorCode::NoErrors) {
    size_t pel = ScVar->buf[ScVar->bptr - 1];  // предыдущий элемент в ар-м выр-ии
    switch (ScVar->buf[ScVar->bptr]) {
    case isbbeg:  // '('
    {
      if (pel == isexpress || pel == isbbeg || pel == ismult || pel == isslash || pel == isplus || pel == isminus || pel == ismod || pel == isdiv) {
        sk++;
      } else {
        err = ErrorCode::ErrorInArithmeticExpression;  // 32;
      }
    } break;
    case isbend:  //')'
    {
      if ((pel == isbend || pel < isbase) && sk)
        sk--;
      else {
        err = ErrorCode::ErrorInArithmeticExpression;  // 32;
      }
    } break;
    case ismod:
    case isdiv:
    case ismult:
    case isslash:
    case isplus:
    case isminus: {
      if (pel != isbend && pel > isbase) {
        err = ErrorCode::ErrorInArithmeticExpression;  // 32;
      }
    } break;
    default:
      if (ScVar->buf[ScVar->bptr] < isbase &&
          (pel == isbbeg || pel == ismult || pel == isslash || pel == isplus || pel == isminus || pel == ismod || pel == isdiv))
        break;
      else {
        err = ErrorCode::ErrorInArithmeticExpression;  // 32;
      }
    }
    ScVar->bptr++;
  }
  if (sk) {
    err = ErrorCode::ErrorInArithmeticExpression;  // 32;
  }
  ScVar->bptr = ScVar->exprip;
  stac[st = 0] = 0;
  st++;
  stac[st++] = ScVar->buf[ScVar->bptr++];
  size_t o = 0;
  n = 0;
  while (ScVar->buf[ScVar->bptr] != isnil && err == ErrorCode::NoErrors) {
    if (ScVar->buf[ScVar->bptr] == isbend && stac[st - 1] == isbbeg) {
      st--;
      ScVar->bptr++;
      continue;
    }  // избавимся от лишних скобок
    if ((p = prioritet(ScVar->buf[ScVar->bptr])) == 0) {
      obuf[o++] = ScVar->buf[ScVar->bptr++];
      n++;
    } else {
      if (p <= prioritet(stac[st - 1]) && p > 1) {
        while (n > 1 && p <= prioritet(stac[st - 1]) && stac[st - 1]) {
          obuf[o++] = stac[--st];
          n--;
        }
        if (p == 2 && stac[st - 1] == isbbeg) {
          st--;
          ScVar->bptr++;
        } else
          stac[st++] = ScVar->buf[ScVar->bptr++];
      } else
        stac[st++] = ScVar->buf[ScVar->bptr++];
    }
  }
  if (err != ErrorCode::NoErrors) {
    return err;
  }
  obuf[o] = 0;
  stac[st] = 0;

  auto index = heap->append<size_t>(0, o);
  memcpy(heap->GetPunsigned(index), obuf, sizeof(size_t) * o);
  index = heap->append(recordexpression(o, index));
  ScVar->bptr = ScVar->exprip - 1;
  ScVar->buf[ScVar->bptr++] = index;
  return err;
}

//==============
ErrorCode expbeg(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;  // выражение заключается в скоики
  if (!ScVar->exprip) {
    ScVar->buf[ScVar->bptr++] = isexpress;
    ScVar->buf[ScVar->exprip = ScVar->bptr++] = isbbeg;
  } else {
    ScVar->buf[ScVar->bptr++] = isbend;
    ScVar->buf[ScVar->bptr] = isnil;
    err = arexpr(ScVar, heap);
    ScVar->exprip = 0;
  }
  return err;
}

ErrorCode lbeg(char *&p, TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  if (ScVar->gptr <= 0) {
    err = ErrorCode::IncorrectUseListAsPredicate;  // 9;
  } else {
    if (*(p + 1) == ']') {
      if (ScVar->bptr + 1 < _maxbptr_) {
        ScVar->buf[ScVar->bptr++] = ScVar->hpempty;
      } else {
        // err = ErrorCode::CannotOpenGraphics;  // 22;
        err = ErrorCode::TooManyCharacterConstants;
      }
      p++;
    } else {
      ScVar->goal[ScVar->gptr++] = ScVar->bptr;
      ScVar->buf[ScVar->bptr++] = islbeg;
    }
  }
  return err;
}

//========
ErrorCode list(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  size_t index;
  size_t indexlast;
  recordlist *ptr;
  ScVar->gptr--;
  if (ScVar->gptr <= 0 || ScVar->gptr >= _maxgptr_ || ScVar->buf[ScVar->goal[ScVar->gptr]] != islbeg) {
    err = ErrorCode::UnpairedBracketsInListProcessing;  // 14;
  } else {
    ScVar->bptr = ScVar->goal[ScVar->gptr];
    ScVar->buf[ScVar->bptr] = heap->last;
    do {
      if (ScVar->buf[++ScVar->bptr] > isbase) {
        err = ErrorCode::WrongList;  // 15;  //не правильный список
      }
      recordlist pl(ScVar->buf[ScVar->bptr],
                    heap->last + sizeof(recordlist));  //!!! это не доделано
      index = heap->append(pl);
    } while (ScVar->buf[++ScVar->bptr] == iscomma && err == ErrorCode::NoErrors);
    if (err == ErrorCode::NoErrors) {
      switch (ScVar->buf[ScVar->bptr]) {
      case islend: {
        indexlast = index;
        index = heap->append(recordemptylist());
        recordlist *prl = heap->GetPrecordlist(indexlast);
        prl->link = index;
      } break;
      case isstick:
        if (ScVar->buf[++ScVar->bptr] > isbase || ScVar->buf[ScVar->bptr + 1] != islend) {
          err = ErrorCode::WrongList;  // 15;  // не верный синтаксис списка
        } else {
          ptr = heap->GetPrecordlist(index);
          // recordlist prl(ScVar->buf[ScVar->bptr], ScVar->hpempty);
          // index = heap->apend(&prl, sizeof(recordlist));
          ptr->link = ScVar->buf[ScVar->bptr];
        }
        break;
      }
    }
  }
  ScVar->bptr = ScVar->goal[ScVar->gptr] + 1;
  return err;
}

//========

ErrorCode arsgn(TScVar *ScVar, size_t i) {
  ErrorCode err = ErrorCode::NoErrors;
  if (!ScVar->exprip) {
    err = ErrorCode::SignNotInArithmeticExpression;  // 18;
  } else {
    ScVar->buf[ScVar->bptr++] = i;
  }
  return err;
}

//=====функции для записи ========================================
//  это неправильно работает с предл
ErrorCode variabletable(char *&p, size_t len, TScVar *ScVar, array *heap) {
  recordvar *ptr;
  size_t k;
  auto &novar = ScVar->novar;
  auto *tvar = ScVar->tvar;
  auto *buf = ScVar->buf;
  auto &bptr = ScVar->bptr;
  for (k = 0; k < novar; k++) {
    ptr = heap->GetPrecordvar(tvar[k]);
    char *name = heap->GetPchar(ptr->ptrsymb);
    if ((len == ptr->length) && (!strncmp(name, p, len))) {
      break;
    }
  }
  if (k == novar)  // такой переменной нет
  {
    auto index = heap->append<char>(0, len);
    memcpy(heap->GetPchar(index), p, sizeof(char) * len);
    // номера переменных будут с 1
    index = heap->append(recordvar(index, len, novar));
    tvar[novar++] = index;
  }
  if (bptr + 1 < _maxbptr_) {
    buf[bptr++] = tvar[k];
  } else {
    return ErrorCode::SentenceOverflow;  // 13;
  }
  return ErrorCode::NoErrors;
}

// Поиск конастанты в Базе Знаний
ErrorCode wrsconst(char *&p, size_t len, TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  auto &bptr = ScVar->bptr;
  auto *buf = ScVar->buf;
  auto &nosymbol = ScVar->nosymbol;
  auto *tat = ScVar->tat;
  recordsconst *ptr;
  size_t k;
  for (k = 0; k < nosymbol; k++) {
    ptr = heap->GetPrecordsconst(tat[k]);
    char *name = heap->GetPchar(ptr->ptrsymb);
    if ((len == ptr->length) && (!strncmp(name, p, len)))
      break;
  }
  if (k == nosymbol)  // такой sconst нет
  {
    auto index = heap->append<char>(0, len);
    memcpy(heap->GetPchar(index), p, sizeof(char) * len);
    index = heap->append(recordsconst(index, len));
    if (nosymbol + 1 < _maxsymbol_) {
      tat[nosymbol++] = index;
    } else {
      err = ErrorCode::TooManyCharacterConstants;  // 23;
    }
  }
  if (err == ErrorCode::NoErrors) {
    if (bptr + 1 < _maxbptr_) {
      buf[bptr++] = (tat[k] == hpmod) ? ismod : (tat[k] == hpdiv) ? isdiv : tat[k];
    } else {
      // err = ErrorCode::CannotOpenGraphics;  // 22;
      err = ErrorCode::TooManyCharacterConstants;
    }
  }
  return err;
}
