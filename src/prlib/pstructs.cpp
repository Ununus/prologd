#include "pstructs.h"
#include "pdefs.h"
#include "functions.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

void TScVar::Init() {
  buf = nullptr;
  goal = nullptr;
  tvar = nullptr;
  tat = nullptr;
  bptr = 0;
  gptr = 0;
  novar = 0;
  nosymbol = 0;
  right = false;
  EndOfClause = false;
  Query = false;
  exprip = 0;
  buf = new size_t[_maxbptr_];
  goal = new size_t[_maxgptr_];
  tvar = new size_t[_maxvar_];
  tat = new size_t[_maxsymbol_];
}

void TScVar::Clear() {
  if (buf)
    delete[] buf;
  if (goal)
    delete[] goal;
  if (tat)
    delete[] tat;
  if (tvar)
    delete[] tvar;
}

TScVar::TScVar() {
  Init();
}

TScVar::~TScVar() {
  Clear();
}

void TClVar::Init() {
  vmaxstack = _vmaxstack_;
  st_con = new size_t[_vmaxstack_];
  st_vr1 = new size_t[_vmaxstack_];
  st_vr2 = new size_t[_vmaxstack_];
  st_trail = new size_t[_vmaxstack_];
  bf = new size_t[_maxbf_];
  BPT = new size_t[_maxbptr_];
  bpt = BPT;
}

void TClVar::Clear() {
  if (st_con)
    delete[] st_con;
  if (st_vr1)
    delete[] st_vr1;
  if (st_vr2)
    delete[] st_vr2;
  if (st_trail)
    delete[] st_trail;
  if (BPT)
    delete[] BPT;
  outBuff.clear();
}

TClVar::TClVar() {
  Init();
}
TClVar::~TClVar() {
  Clear();
}

//============array=============
void array::clear() {
  freeheap = last = 0;
  pnclause = nullptr;
  ptclause = nullptr;
  paclause = nullptr;
  phclause = nullptr;
  pacltarget = nullptr;
  ptcltarget = nullptr;
  pncltarget = nullptr;
}

// heap->last = bakindex;
void array::cleanUp(size_t idx) {
  while (!recordintegersInHeap.empty() && recordintegersInHeap.back() >= idx) {
    // std::cout << "Int destr call " << idx << std::endl;
    GetPrecordinteger(recordintegersInHeap.back())->value.~IntegerType();
    recordintegersInHeap.pop_back();
  }
  last = idx;
}
array::array(size_t SIZE) {
  heaps = new unsigned char[SIZE];
  size = (heaps != 0) ? SIZE : 0;
  clear();
}

array::~array() {
  if (heaps) {
    for (size_t idx : recordintegersInHeap) {
      // std::cout << "Int destr call " << idx << std::endl;
      GetPrecordinteger(idx)->value.~IntegerType();
    }
    recordintegersInHeap.clear();
    delete[] heaps;
  }
}

void array::expand() {
  size_t sizenew = size << 1;
  unsigned char *heapsnew = new (std::nothrow) unsigned char[sizenew];
  if (!heapsnew) {
    throw std::runtime_error("Not enoth memory");
  }
  memset(heapsnew, 0, sizenew);
  for (unsigned i = 0; i < size; i++) {
    heapsnew[i] = heaps[i];
  }
  if (pnclause)
    pnclause = (recordclause *)&heapsnew[(unsigned char *)pnclause - heaps];
  if (ptclause)
    ptclause = (recordclause *)&heapsnew[(unsigned char *)ptclause - heaps];
  if (paclause)
    paclause = (recordclause *)&heapsnew[(unsigned char *)paclause - heaps];
  if (phclause)
    phclause = (recordclause *)&heapsnew[(unsigned char *)phclause - heaps];
  if (pacltarget)
    pacltarget = (size_t *)&heapsnew[(unsigned char *)pacltarget - heaps];
  if (ptcltarget)
    ptcltarget = (size_t *)&heapsnew[(unsigned char *)ptcltarget - heaps];
  if (pncltarget)
    pncltarget = (size_t *)&heapsnew[(unsigned char *)pncltarget - heaps];
  delete[] heaps;
  heaps = heapsnew;
  size = sizenew;
  //    maxarray = size;
}

baserecord *array::GetPbaserecord(size_t index) {
  return (baserecord *)&heaps[index];
}

recordsconst *array::GetPrecordsconst(size_t index) {
  return (recordsconst *)GetPbaserecord(index);
}

recordstring *array::GetPrecordstring(size_t index) {
  return (recordstring *)GetPbaserecord(index);
}

recordvar *array::GetPrecordvar(size_t index) {
  return (recordvar *)GetPbaserecord(index);
}

recordinteger *array::GetPrecordinteger(size_t index) {
  return (recordinteger *)GetPbaserecord(index);
}

recordfloat *array::GetPrecordfloat(size_t index) {
  return (recordfloat *)GetPbaserecord(index);
}

recordunknown *array::GetPrecordunknown(size_t index) {
  return (recordunknown *)GetPbaserecord(index);
}

recordcut *array::GetPrecordcut(size_t index) {
  return (recordcut *)GetPbaserecord(index);
}

recordemptylist *array::GetPrecordemptylist(size_t index) {
  return (recordemptylist *)GetPbaserecord(index);
}

recordexpression *array::GetPrecordexpression(size_t index) {
  return (recordexpression *)GetPbaserecord(index);
}

recordfunction *array::GetPrecordfunction(size_t index) {
  return (recordfunction *)GetPbaserecord(index);
}

recordlist *array::GetPrecordlist(size_t index) {
  return (recordlist *)GetPbaserecord(index);
}

recordclause *array::GetPrecordclause(size_t index) {
  return (recordclause *)GetPbaserecord(index);
}

size_t *array::GetPunsigned(size_t index) {
  return (size_t *)GetPbaserecord(index);
}

char *array::GetPchar(size_t index) {
  return (char *)GetPbaserecord(index);
}

//============конец array========

#ifdef PROLOG_DEBUG
void PrintSconst(char **Buf, size_t *BufSize, recordsconst *rsc, TScVar *ScVar, array *heap);
void PrintVar(char **Buf, size_t *BufSize, recordvar *pbr, TScVar *ScVar, array *heap);
void PrintInteger(char **Buf, size_t *BufSize, recordinteger *pri, TScVar *ScVar, array *heap);
void PrintUnknown(char **Buf, size_t *BufSize, recordunknown *pru, TScVar *ScVar, array *heap);
void PrintList(char **Buf, size_t *BufSize, recordlist *pru, TScVar *ScVar, array *heap);
void PrintExpression(char **Buf, size_t *BufSize, recordexpression *pbr, TScVar *ScVar, array *heap);
void PrintEmptyList(char **Buf, size_t *BufSize, recordemptylist *pr, TScVar *ScVar, array *heap);

void PrintFunction(char **Buf, size_t *BufSize, recordfunction *prf, TScVar *ScVar, array *heap) {
  char pBuf[255];
  char *p = *Buf;
  baserecord *pbr = heap->GetPbaserecord(prf->func);
  if (pbr->ident == issymbol) {
    char *p = *Buf;
    PrintSconst(Buf, BufSize, (recordsconst *)pbr, ScVar, heap);
    //*Buf = 0;
    pldout(const_cast<char *>("finction:"));
    char Func[1024];
    auto len = (*Buf - p);
    strncpy(Func, p, len);
    Func[len] = 0;
    pldout(const_cast<const char *>(Func));
  }
  auto *ptrarg = heap->GetPunsigned(prf->ptrarg);
  if (prf->narg && (*BufSize) > 2) {
    sprintf(*Buf, "(");
    (*Buf)++;
    (*BufSize)--;

    for (int i = 0; i < static_cast<int>(prf->narg); i++) {
      if (i && (*BufSize) > 2) {
        sprintf(*Buf, ",");
        (*Buf)++;
        (*BufSize)--;
      }
      baserecord *pbr = heap->GetPbaserecord(ptrarg[i]);
      char sarg[255];
      sprintf(sarg, "argument index: %zd", ptrarg[i]);
      pldout(const_cast<const char *>(sarg));
      switch (pbr->ident) {
      case issymbol: PrintSconst(Buf, BufSize, (recordsconst *)pbr, ScVar, heap); break;
      case isvar: PrintVar(Buf, BufSize, (recordvar *)pbr, ScVar, heap); break;
      case isinteger: PrintInteger(Buf, BufSize, (recordinteger *)pbr, ScVar, heap); break;
      case isunknown: PrintUnknown(Buf, BufSize, (recordunknown *)pbr, ScVar, heap); break;
      case islist: PrintList(Buf, BufSize, (recordlist *)pbr, ScVar, heap); break;
      case isexpression: PrintExpression(Buf, BufSize, (recordexpression *)pbr, ScVar, heap); break;
      case isemptylist: PrintEmptyList(Buf, BufSize, (recordemptylist *)pbr, ScVar, heap); break;
      default:
        auto len = *Buf - p;
        if (len > 0) {
          strncpy(pBuf, p, len);
          pBuf[len] = 0;
          pldout(pBuf);
        }
        sprintf(pBuf, "PrintFunction: unknown ident: %zd", pbr->ident);
        pldout(pBuf);
      }
    }
    if (*BufSize > 2) {
      sprintf(*Buf, ")");
      (*BufSize)--;
      (*Buf)++;
    }
  }
}
void PrintUnknown(char **Buf, size_t *BufSize, recordunknown *pru, TScVar *ScVar, array *heap) {
  if (2 < *BufSize) {
    strncpy(*Buf, "_", 1);
    *(BufSize) -= 1;
    (*Buf) += 1;
    return;
  }
  pldout(const_cast<char *>("Buffer for unknown out is too small"));
}

void PrintExpression(char **Buf, size_t *BufSize, recordexpression *pex, TScVar *ScVar, array *heap) {
  if (2 > *BufSize) {
    pldout(const_cast<char *>("Buffer for unknown out is too small"));
    return;
  }
  strncpy(*Buf, "#", 1);
  *(BufSize) -= 1;
  (*Buf) += 1;

  if (2 > *BufSize) {
    pldout(const_cast<char *>("Buffer for unknown out is too small"));
    return;
  }
  strncpy(*Buf, "#", 1);
  *(BufSize) -= 1;
  (*Buf) += 1;
}

void PrintList(char **Buf, size_t *BufSize, recordlist *rl, TScVar *ScVar, array *heap) {
  char pBuf[255]{};
  char *p = *Buf;
  if (2 > *BufSize) {
    pldout(const_cast<char *>("Buffer for unknown out is too small"));
    return;
  }
  strncpy(*Buf, "[", 1);
  *(BufSize) -= 1;
  (*Buf) += 1;
  // baserecord * pbr = (baserecord *)&heap->heaps[rl->head];
  int count = 0;
  while (rl && rl->ident == islist) {
    if (count++) {
      if (2 > *BufSize) {
        pldout(const_cast<char *>("Buffer for unknown out is too small"));
        return;
      }
      strncpy(*Buf, ",", 1);
      *(BufSize) -= 1;
      (*Buf) += 1;
    }
    baserecord *pbr = heap->GetPbaserecord(rl->head);
    switch (pbr->ident) {
    case issymbol: PrintSconst(Buf, BufSize, (recordsconst *)pbr, ScVar, heap); break;
    case isvar: PrintVar(Buf, BufSize, (recordvar *)pbr, ScVar, heap); break;
    case isinteger: PrintInteger(Buf, BufSize, (recordinteger *)pbr, ScVar, heap); break;
    case isunknown: PrintUnknown(Buf, BufSize, (recordunknown *)pbr, ScVar, heap); break;
    case islist: PrintList(Buf, BufSize, (recordlist *)pbr, ScVar, heap); break;
    case isemptylist: break;
    default:
      auto len = *Buf - p;
      if (len > 0) {
        strncpy(pBuf, p, len);
        pBuf[len] = 0;
        pldout(pBuf);
      }
      sprintf(pBuf, "PrintList: unknown ident: %zd", pbr->ident);
      pldout(pBuf);
    }
    auto rl_link = rl->link;
    auto ident = pbr->ident;
    if (rl->link != NULL && rl->link != isnil && pbr->ident != isemptylist) {
      rl = heap->GetPrecordlist(rl->link);
    } else {
      rl = (recordlist *)0;
    }
  }

  if (2 > *BufSize) {
    pldout(const_cast<char *>("Buffer for unknown out is too small"));
    return;
  }
  strncpy(*Buf, "]", 1);
  *(BufSize) -= 1;
  (*Buf) += 1;
}

void PrintSconst(char **Buf, size_t *BufSize, recordsconst *rsc, TScVar *ScVar, array *heap) {
  char *pc = heap->GetPchar(rsc->ptrsymb);
  if (rsc->length + 1 < *BufSize) {
    strncpy(*Buf, pc, rsc->length);
    *(BufSize) -= rsc->length;
    (*Buf) += rsc->length;
    return;
  }
  pldout(const_cast<char *>("Buffer for sconst out is too small"));
}

void PrintVar(char **Buf, size_t *BufSize, recordvar *prv, TScVar *ScVar, array *heap) {
  char *pc = heap->GetPchar(prv->ptrsymb);
  if (prv->length + 1 < *BufSize) {
    strncpy(*Buf, pc, prv->length);
    *(BufSize) -= prv->length;
    (*Buf) += prv->length;
    return;
  }
  pldout(const_cast<char *>("Buffer for var out is too small"));
}

void PrintEmptyList(char **Buf, size_t *BufSize, recordemptylist *pr, TScVar *ScVar, array *heap) {
  char pBuf[255];
  sprintf(pBuf, "[]");
  size_t Length = strlen(pBuf);
  if (Length + 1 < *BufSize) {
    strcpy(*Buf, pBuf);
    *(BufSize) -= Length;
    (*Buf) += Length;
    return;
  }
  pldout(const_cast<char *>("Buffer for Integer out is too small"));
}

void PrintInteger(char **Buf, size_t *BufSize, recordinteger *pri, TScVar *ScVar, array *heap) {
  char pBuf[255];
  sprintf(pBuf, "%s", pri->value.str().c_str());
  size_t Length = strlen(pBuf);
  if (Length + 1 < *BufSize) {
    strcpy(*Buf, pBuf);
    *(BufSize) -= Length;
    (*Buf) += Length;
    return;
  }
  pldout(const_cast<char *>("Buffer for Integer out is too small"));
}

void PrintClause(recordclause *rc, TScVar *ScVar, array *heap) {
  char _Buf[1024];
  char *p = _Buf;
  size_t BufSize = sizeof(_Buf);
  if (rc->ident == isclauseq) {
    *(p++) = '?';
    BufSize--;
  }
  auto *ptarget = heap->GetPunsigned(rc->ptrtarget);
  for (int i = 0; *(ptarget + i); i++) {
    if (i == 1 && BufSize > 3) {
      *(p++) = ':';
      *(p++) = '-';
      BufSize -= 2;
    } else if (i && BufSize > 2) {
      *(p++) = ',';
      BufSize--;
    }
    recordfunction *rf = heap->GetPrecordfunction(*(ptarget + i));
    switch (rf->ident) {
    case isfunction: PrintFunction(&p, &BufSize, rf, ScVar, heap); break;
    case issymbol: PrintSconst(&p, &BufSize, (recordsconst *)rf, ScVar, heap); break;
    default: sprintf(_Buf, "PrintClause: unknown ident: %zd", rf->ident);
    }
  }
  if (BufSize > 2)
    sprintf(p, ".");
  pldout(_Buf);
}

void PrintClauses(recordclause *rc, TScVar *ScVar, array *heap) {
  while (1) {
    PrintClause(rc, ScVar, heap);
    if (rc->next == NULL || rc->next == isnil)
      return;
    rc = heap->GetPrecordclause(rc->next);
  }
}

void PrintProgram(TScVar *ScVar, array *heap) {
#if 1
  return;
#else

  char _Buf[1024];
  pldout(const_cast<char *>("=========================================================="));
  pldout(const_cast<char *>("Print program .."));
  int i = 0;
  unsigned int &nosymbol = ScVar->nosymbol;
  unsigned int *tat = ScVar->tat;
  for (; i < static_cast<int>(nosymbol); i++) {
    // recordsconst * ptr =
    // heap->GetPrecordsconst(*heap->GetPunsigned(tat[i]));
    recordsconst *ptr = heap->GetPrecordsconst(tat[i]);
    //(recordsconst *)&heap->heaps[tat[i]];
    char *name = heap->GetPchar(ptr->ptrsymb);
    int Length = ptr->length;
    if (Length > sizeof(_Buf))
      Length = sizeof(_Buf) - 1;
    strncpy(_Buf, name, Length);
    _Buf[Length] = 0;
    pldout(_Buf);
    if (ptr->begin) {
      recordclause *rc = heap->GetPrecordclause(ptr->begin);
      //(recordclause *)&heap->heaps[ptr->begin];
      PrintClauses(rc, ScVar, heap);
    }
  }
#endif
}
#endif