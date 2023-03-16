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

//=======�������������� ����������
const char *NamesOfPredicates[] = { "mod",    "����",      "������",    "���_������", "!",        "���",   "��������", "�������",  "�������", "�������",
                                    "�����",  "������_��", "������_�",  "���",        "���",      "���",   "����",     "������",   "�����",   "������",
                                    "������", "������",    "�������",   "������",     "�����",    "�����", "����",     "��������", "�������", "�����",
                                    "����",   "���",       "���������", "����������", "��������", "�����", "�����",    "�����",    "����",    "��������",
                                    "���",    "div",       "int",       "float",      "����",     "������" };
const size_t MAX_BUILD_PRED = sizeof(NamesOfPredicates) / sizeof(char *);

const char *Build_in_Libriary[] = {
  "��(x)<-���(x),!,����;",
  "��(_);",
  "���(x,_)<-���(x),!;",
  "���(_,x)<-���(x);",
  "�����(x,x);",
  "������(x,y)<-������(y,x);",
  "��������(x,y,z)<-���������(1,x,y,z);",
  "��<-�����(10);",
  "�����(x,y)<-�����(x,1,y,x);",
};
const size_t LEN_BUILD_LIBR = sizeof(Build_in_Libriary) / sizeof(char *);

// ����������� ���������� ����������
ErrorCode buildin(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  for (size_t i = 0; i < MAX_BUILD_PRED; i++) {
    size_t len = strlen(NamesOfPredicates[i]);
    auto index = heap->append<char>(0, len);
    memcpy(heap->GetPchar(index), NamesOfPredicates[i], sizeof(char) * len);
    ScVar->vtat[i] = heap->append(recordsconst(index, len));
  }
  // freesymbol = MAX_BUILD_PRED;
  ScVar->nosymbol = MAX_BUILD_PRED;
  ScVar->hpunkn = heap->append(recordunknown());     // ��������� �������� � heap
  ScVar->hpempty = heap->append(recordemptylist());  // ��������� ������� ������
  heap->freeheap = heap->last;

  // extern char*Build_in_Libriary[] ;
  for (size_t i = 0; i < LEN_BUILD_LIBR && err == ErrorCode::NoErrors; ++i) {
    char *p = (char *)Build_in_Libriary[i];
    err = scaner(p, ScVar, heap);
  }
  return err;
}

// �������������. ��� ���������� � ��������� ���������
ErrorCode InitProlog() {
  ErrorCode err = ErrorCode::NoErrors;  // ��� ������
  return err;
}

ErrorCode FreeProlog() {
  return ErrorCode::NoErrors;
}

// ������� ��� ���������� ��������� ��������� �������
ErrorCode tokbb(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  auto &bptr = ScVar->bptr;
  auto &buf = ScVar->vbuf;
  auto &goal = ScVar->vgoal;
  auto &gptr = ScVar->gptr;
  if (gptr == 0 || gptr >= ScVar->vgoal.size() || buf[goal[gptr - 1]] != isbbeg) {
    return ErrorCode::UnpairedBracketsInFunctionProcessing;  // 16; //��� ���� ������ ��� �� issymbol
  }
  --gptr;
  auto i = goal[gptr];  // ������ � ������� buf ����������� �� (
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
  auto n = (bptr - i) / 2;  // ����� ����������
  std::vector<size_t> ptrargs(n);
  for (size_t j = 0; j < n; j++) {
    if ((ptrargs[j] = buf[i + j * 2]) > isbase) {
      err = ErrorCode::ErrorWhileParsingFunction;  // 38;
    }
  }
  // ��������� � ������
  auto index = heap->append<size_t>(0, n);
  memcpy(heap->GetPunsigned(index), ptrargs.data(), sizeof(size_t) * n);
  i -= 2;
  if (err != ErrorCode::NoErrors) {
    return err;
  }
  index = heap->append(recordfunction(n, buf[i], index));
  bptr = i;
  if (bptr + 1 >= buf.size()) {
    buf.resize(std::max(buf.size() * 2, bptr + 2));
  }
  buf[bptr++] = index;
  return err;
}

ErrorCode clause(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  bool &Query = ScVar->Query;
  bool &EndOfClause = ScVar->EndOfClause;
  auto &gptr = ScVar->gptr;
  auto &buf = ScVar->vbuf;
  auto &novar = ScVar->novar;
  auto id = Query ? isclauseq : isclause;
  if (gptr != NULL) {
    err = ErrorCode::UnpairedBracketsInFunctionProcessing;  // 16;
  }
  //!!! ����� ����� �������� �� ������ ������
  baserecord *tp;
  size_t ntarget = 1;  // ����� �����
  int i = 1, j = 0;
  int n = 0;
  while (buf[n] != isend) {
    n++;
  }
  // �� ������ ����� ������ ���� ���� ��������� ��� func
  if (buf[0] >= heap->last) {
    err = ErrorCode::ErrorWhileParsingSentence;  // 20;  //������ ��� ������� �����
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
    auto index = heap->append<size_t>(0, (ntarget + 1));
    size_t *_ptarget = heap->GetPunsigned(index);
    recordclause *ptrpred = 0;
    index = heap->append(recordclause(id, 0, novar, buf[0], index));
    recordclause *ptr = heap->GetPrecordclause(index);
    // ������ ��������� ����
    i = 0;
    j = 0;
    while (j < (int)ntarget) {
      *(_ptarget + j++) = buf[i];
      i += 2;
    }
    *(_ptarget + j) = NULL;
    // �8������� ���������� ����������� �����������
    if (!Query) {
      // i=j=heap->indexOf(ptr)-1;j++;i--;//j-������ ������� ����������� � heap
      // � ptr - ��������� �� ����������� � index ������ ������� �����������
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
  // ������ ������ ������ ����� ��� ���� + -
  ErrorCode err = ErrorCode::NoErrors;
  auto &bptr = ScVar->bptr;
  auto &buf = ScVar->vbuf;
  // char bufnum[1024]{};
  std::string bufnum;
  FloatType valuef = 0;
  IntegerType valuei = 0;
  std::ptrdiff_t punkt = 0, e = 0;
  std::ptrdiff_t i = 0;
  if (*p == '+' || *p == '-') {
    //bufnum[i++] = *p;
    bufnum.push_back(*p);
    ++i;
  }
  for (; ((isdigitrus(*(p + i))) || (*(p + i) == 'e' && punkt) || (*(p + i) == 'E' && punkt) || (*(p + i) == '-' && e) || (*(p + i) == '+' && e) ||
          (*(p + i) == '.'));
       i++) {
    bufnum.push_back(*(p + i));
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
    //bufnum[i] = 0;
    if (punkt || e) {
      // std::from_chars(bufnum, bufnum + sizeof(bufnum), valuef); // � gcc �� �����������
      valuef = stod(bufnum);
    } else {
      // std::from_chars(bufnum, bufnum + sizeof(bufnum), valuei);
      // valuei = atoll(bufnum);
      valuei = IntegerType(bufnum);
    }
    if ((valuef == valuei.convert_to<FloatType>()) && (i > 1 || (i == 1 && (*p != '0')))) {
      err = ErrorCode::InvalidNumberFormat;  // 4;  // �� ������ ������ �����
    } else {
      if (punkt || e) {
        auto index = heap->append(recordfloat(valuef));
        if (bptr + 1 >= buf.size()) {
          buf.resize(std::max(buf.size() * 2, bptr + 2));
        }
        buf[bptr++] = index;
      } else {
        auto index = heap->append(recordinteger(valuei));
        if (bptr + 1 >= buf.size()) {
          buf.resize(std::max(buf.size() * 2, bptr + 2));
        }
        buf[bptr++] = index;
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
  if ((symb >= '�' && symb <= '�') || (symb >= '�' && symb <= '�') || symb == '�' || symb == '�') {
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
    ScVar->vbuf[ScVar->bptr++] = isimpl;
  }
  return err;
}
//===========
size_t prioritet(size_t ch) {
  size_t value = 0;
  switch (ch) {
  case ismod:  // ������� �� �������
  case isdiv:  // ������������� �������
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
  ScVar->bptr = ScVar->exprip;
  //========�������� ���������� � �� ���
  int sk = 0;  // ������
  while (ScVar->vbuf[ScVar->bptr] != isnil && err == ErrorCode::NoErrors) {
    size_t pel = ScVar->vbuf[ScVar->bptr - 1];  // ���������� ������� � ��-� ���-��
    switch (ScVar->vbuf[ScVar->bptr]) {
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
      if (ScVar->vbuf[ScVar->bptr] < isbase &&
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
  std::vector<size_t> vobuf;
  std::vector<size_t> vstac;
  vstac.push_back(0);
  vstac.push_back(ScVar->vbuf[ScVar->bptr++]);
  size_t n = 0, p;  // ����� ��, ���������
  while (ScVar->vbuf[ScVar->bptr] != isnil && err == ErrorCode::NoErrors) {
    if (ScVar->vbuf[ScVar->bptr] == isbend && vstac.back() == isbbeg) {
      vstac.pop_back();
      ScVar->bptr++;
      continue;
    }  // ��������� �� ������ ������
    if ((p = prioritet(ScVar->vbuf[ScVar->bptr])) == 0) {
      vobuf.push_back(ScVar->vbuf[ScVar->bptr++]);
      n++;
    } else {
      if (p <= prioritet(vstac.back()) && p > 1) {
        while (n > 1 && p <= prioritet(vstac.back()) && vstac.back()) {
          vobuf.push_back(vstac.back());
          vstac.pop_back();
          n--;
        }
        if (p == 2 && vstac.back() == isbbeg) {
          vstac.pop_back();
          ScVar->bptr++;
        } else {
          vstac.push_back(ScVar->vbuf[ScVar->bptr++]);
        }
      } else {
        vstac.push_back(ScVar->vbuf[ScVar->bptr++]);
      }
    }
  }
  if (err != ErrorCode::NoErrors) {
    return err;
  }
  auto index = heap->append<size_t>(0, vobuf.size());
  memcpy(heap->GetPunsigned(index), vobuf.data(), sizeof(size_t) * vobuf.size());
  index = heap->append(recordexpression(vobuf.size(), index));
  ScVar->bptr = ScVar->exprip - 1;
  ScVar->vbuf[ScVar->bptr++] = index;
  return err;
}

//==============
ErrorCode expbeg(TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;  // ��������� ����������� � ������
  if (!ScVar->exprip) {
    ScVar->vbuf[ScVar->bptr++] = isexpress;
    ScVar->vbuf[ScVar->exprip = ScVar->bptr++] = isbbeg;
  } else {
    ScVar->vbuf[ScVar->bptr++] = isbend;
    ScVar->vbuf[ScVar->bptr] = isnil;
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
      if (ScVar->bptr + 1 >= ScVar->vbuf.size()) {
        ScVar->vbuf.resize(std::max(ScVar->vbuf.size() * 2, ScVar->bptr + 2));
      }
      ScVar->vbuf[ScVar->bptr++] = ScVar->hpempty;
      p++;
    } else {
      ScVar->vgoal[ScVar->gptr++] = ScVar->bptr;
      ScVar->vbuf[ScVar->bptr++] = islbeg;
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
  if (ScVar->gptr <= 0 || ScVar->gptr >= ScVar->vgoal.size() || ScVar->vbuf[ScVar->vgoal[ScVar->gptr]] != islbeg) {
    err = ErrorCode::UnpairedBracketsInListProcessing;  // 14;
  } else {
    ScVar->bptr = ScVar->vgoal[ScVar->gptr];
    ScVar->vbuf[ScVar->bptr] = heap->last;
    do {
      if (ScVar->vbuf[++ScVar->bptr] > isbase) {
        err = ErrorCode::WrongList;  // 15;  //�� ���������� ������
      }
      recordlist pl(ScVar->vbuf[ScVar->bptr],
                    heap->last + sizeof(recordlist));  //!!! ��� �� ��������
      index = heap->append(pl);
    } while (ScVar->vbuf[++ScVar->bptr] == iscomma && err == ErrorCode::NoErrors);
    if (err == ErrorCode::NoErrors) {
      switch (ScVar->vbuf[ScVar->bptr]) {
      case islend: {
        indexlast = index;
        index = heap->append(recordemptylist());
        recordlist *prl = heap->GetPrecordlist(indexlast);
        prl->link = index;
      } break;
      case isstick:
        if (ScVar->vbuf[++ScVar->bptr] > isbase || ScVar->vbuf[ScVar->bptr + 1] != islend) {
          err = ErrorCode::WrongList;  // 15;  // �� ������ ��������� ������
        } else {
          ptr = heap->GetPrecordlist(index);
          // recordlist prl(ScVar->buf[ScVar->bptr], ScVar->hpempty);
          // index = heap->apend(&prl, sizeof(recordlist));
          ptr->link = ScVar->vbuf[ScVar->bptr];
        }
        break;
      }
    }
  }
  ScVar->bptr = ScVar->vgoal[ScVar->gptr] + 1;
  return err;
}

//========

ErrorCode arsgn(TScVar *ScVar, size_t i) {
  ErrorCode err = ErrorCode::NoErrors;
  if (!ScVar->exprip) {
    err = ErrorCode::SignNotInArithmeticExpression;  // 18;
  } else {
    ScVar->vbuf[ScVar->bptr++] = i;
  }
  return err;
}

//=====������� ��� ������ ========================================
//  ��� ����������� �������� � �����
ErrorCode variabletable(char *&p, size_t len, TScVar *ScVar, array *heap) {
  recordvar *ptr;
  size_t k;
  auto &novar = ScVar->novar;
  auto &tvar = ScVar->vtvar;
  auto &buf = ScVar->vbuf;
  auto &bptr = ScVar->bptr;
  for (k = 0; k < novar; k++) {
    ptr = heap->GetPrecordvar(tvar[k]);
    char *name = heap->GetPchar(ptr->ptrsymb);
    if ((len == ptr->length) && (!strncmp(name, p, len))) {
      break;
    }
  }
  if (k == novar)  // ����� ���������� ���
  {
    auto index = heap->append<char>(0, len);
    memcpy(heap->GetPchar(index), p, sizeof(char) * len);
    // ������ ���������� ����� � 1
    index = heap->append(recordvar(index, len, novar));
    tvar[novar++] = index;
  }
  if (bptr + 1 >= buf.size()) {
    buf.resize(std::max(buf.size() * 2, bptr + 2));
  }
  buf[bptr++] = tvar[k];
  return ErrorCode::NoErrors;
}

// ����� ���������� � ���� ������
ErrorCode wrsconst(char *&p, size_t len, TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  auto &bptr = ScVar->bptr;
  auto &buf = ScVar->vbuf;
  auto &nosymbol = ScVar->nosymbol;
  auto &tat = ScVar->vtat;
  recordsconst *ptr;
  size_t k;
  for (k = 0; k < nosymbol; k++) {
    ptr = heap->GetPrecordsconst(tat[k]);
    char *name = heap->GetPchar(ptr->ptrsymb);
    if ((len == ptr->length) && (!strncmp(name, p, len)))
      break;
  }
  if (k == nosymbol)  // ����� sconst ���
  {
    auto index = heap->append<char>(0, len);
    memcpy(heap->GetPchar(index), p, sizeof(char) * len);
    index = heap->append(recordsconst(index, len));
    if (nosymbol + 1 >= tat.size()) {
      tat.resize(std::max(tat.size() * 2, nosymbol + 2));
    }
    tat[nosymbol++] = index;
  }
  if (err == ErrorCode::NoErrors) {
    if (bptr + 1 >= buf.size()) {
      buf.resize(std::max(buf.size() * 2, bptr + 2));
    }
    buf[bptr++] = (tat[k] == hpmod) ? ismod : (tat[k] == hpdiv) ? isdiv : tat[k];
  }
  return err;
}
