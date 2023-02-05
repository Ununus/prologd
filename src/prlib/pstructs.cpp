#include "pstructs.h"
#include "pdefs.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

void TScVar::Init() {
  buf = (unsigned int *)0;
  goal = (unsigned int *)0;
  tvar = (unsigned int *)0;
  tat = (unsigned int *)0;
  bptr = 0;
  gptr = 0;
  novar = 0;
  nosymbol = 0;
  right = false;
  EndOfClause = false;
  Query = false;
  exprip = 0;
  buf = new unsigned int[_maxbptr_];
  goal = new unsigned int[_maxgptr_];
  tvar = new unsigned int[_maxvar_];
  tat = new unsigned int[_maxsymbol_];
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
  st_con = new unsigned int[_vmaxstack_];
  st_vr1 = new unsigned int[_vmaxstack_];
  st_vr2 = new unsigned int[_vmaxstack_];
  st_trail = new unsigned int[_vmaxstack_];
  bf = new unsigned int[_maxbf_];
  BPT = new unsigned int[_maxbptr_];
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
}

TClVar::TClVar() {
  Init();
}
TClVar::~TClVar() {
  Clear();
}

recordsconst::recordsconst(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN) {
  ident = issymbol;
  ptrsymb = PTRsymb;
  length = LENGTH;
  begin = BEGIN;
}

recordstring::recordstring(unsigned PTRsymb, unsigned char LENGTH, unsigned int BEGIN) {
  ident = isstring;
  ptrsymb = PTRsymb;
  length = LENGTH;
  begin = BEGIN;
}
//если begin!=0 то это значит что строка создана при исполнении программы
//функцией zap3();
//пролога и она находится не в тексте редактора а динам памяти
//следовательно ее нужно уничточить при выходи из исполнения программы
// иначе это в тексте редактора и уничтожится при закрытии окна с текстом
/*
recordstring::~recordstring()
{   if (begin) delete (char*)ptr;
}
*/
recordvar::recordvar(unsigned PTRsymb, unsigned char LENGTH, unsigned int NUM) {
  ident = isvar;
  ptrsymb = PTRsymb;
  length = LENGTH;
  num = NUM;
}

recordinteger::recordinteger(IntegerType VALUE) {
  ident = isinteger;
  value = VALUE;
}

recordfloat::recordfloat(FloatType VALUE) {
  ident = isfloat;
  value = VALUE;
}

recordunknown::recordunknown() {
  ident = isunknown;
}

recordcut::recordcut() {
  ident = iscut;
  func = hpcut;
}

recordemptylist::recordemptylist() {
  ident = isemptylist;
}

recordexpression::recordexpression(unsigned int LENGTH,
                                   unsigned int PRECORD)  //(unsigned int *PTR,unsigned int LENGTH)
{
  ident = isexpression;
  length = LENGTH;
  precord = PRECORD;
}  // ptr=PTR;length=LENGTH;}

recordfunction::recordfunction(unsigned char NARG,
                               unsigned int FUNC,
                               unsigned int PTRARG)  //,unsigned int *PTRARG)
{
  ident = isfunction;
  narg = NARG;
  func = FUNC;
  ptrarg = PTRARG;
}  //;ptrarg=PTRARG;}

recordlist::recordlist(unsigned int HEAD, unsigned int LINK) {
  ident = islist;
  head = HEAD;
  link = LINK;
}

recordclause::recordclause(unsigned char IDENT,
                           unsigned int NEXT,
                           unsigned int NVARS,
                           unsigned int HEAD,
                           unsigned int PTRTARGET)  //,unsigned int *PTRTARGET)
{
  ident = IDENT;
  next = NEXT;
  nvars = NVARS;
  head = HEAD;
  ptrtarget = PTRTARGET;
}  // ptrtarget=PTRTARGET;}

//============array=============
void array::clear() {
  freeheap = last = 0;
  pnclause = (recordclause *)0;
  ptclause = (recordclause *)0;
  paclause = (recordclause *)0;
  phclause = (recordclause *)0;
  pacltarget = (unsigned *)0;
  ptcltarget = (unsigned *)0;
  pncltarget = (unsigned *)0;
}
array::array(unsigned int SIZE) {
  heaps = new unsigned char[SIZE];
  size = (heaps != 0) ? SIZE : 0;
  clear();
}

array::~array() {
  if (heaps)
    delete[] heaps;
}

int array::expand() {
#ifdef _DEBUG_
  pldout("array::expand");
#endif
  unsigned sizenew = size << 1;
  unsigned char *heapsnew = new unsigned char[sizenew];
  if (!heapsnew) {
    // throw(T_Exception(ExErrAndMessage, LgError,
    //	"can`t allocate memory for array expand"));
    return 44;
  }
  memset(heapsnew, 0, sizenew);
  for (unsigned i = 0; i < size; i++)
    heapsnew[i] = heaps[i];
  if (pnclause)
    pnclause = (recordclause *)&heapsnew[(unsigned char *)pnclause - heaps];
  if (ptclause)
    ptclause = (recordclause *)&heapsnew[(unsigned char *)ptclause - heaps];
  if (paclause)
    paclause = (recordclause *)&heapsnew[(unsigned char *)paclause - heaps];
  if (phclause)
    phclause = (recordclause *)&heapsnew[(unsigned char *)phclause - heaps];
  if (pacltarget)
    pacltarget = (unsigned *)&heapsnew[(unsigned char *)pacltarget - heaps];
  if (ptcltarget)
    ptcltarget = (unsigned *)&heapsnew[(unsigned char *)ptcltarget - heaps];
  if (pncltarget)
    pncltarget = (unsigned *)&heapsnew[(unsigned char *)pncltarget - heaps];
  delete[] heaps;
  heaps = heapsnew;
  size = sizenew;
  //    maxarray = size;
  return 0;
}
unsigned int array::apend(void *ptr, unsigned int count) {
  unsigned int baklast = last;
  if (!ptr)
    return -1;
  if (last + count + 1 > size && expand() != 0)
    return -1;
  unsigned char *d = (unsigned char *)&heaps[last];
  memcpy(d, ptr, count);
  last += count;
  //   for(;count; *d++ = *s++, count--);
  return baklast;
}

baserecord *array::GetPbaserecord(unsigned int index) {
  // if (index < 0 || index >= last)
  //	throw(T_Exception(ExErrAndMessage, LgError,
  //		"GetPbaserecord from array: incorrect index"));
  return (baserecord *)&heaps[index];
}

recordsconst *array::GetPrecordsconst(unsigned int index) {
  return (recordsconst *)GetPbaserecord(index);
}

recordstring *array::GetPrecordstring(unsigned int index) {
  return (recordstring *)GetPbaserecord(index);
}

recordvar *array::GetPrecordvar(unsigned int index) {
  return (recordvar *)GetPbaserecord(index);
}

recordinteger *array::GetPrecordinteger(unsigned int index) {
  return (recordinteger *)GetPbaserecord(index);
}

recordfloat *array::GetPrecordfloat(unsigned int index) {
  return (recordfloat *)GetPbaserecord(index);
}

recordunknown *array::GetPrecordunknown(unsigned int index) {
  return (recordunknown *)GetPbaserecord(index);
}

recordcut *array::GetPrecordcut(unsigned int index) {
  return (recordcut *)GetPbaserecord(index);
}

recordemptylist *array::GetPrecordemptylist(unsigned int index) {
  return (recordemptylist *)GetPbaserecord(index);
}

recordexpression *array::GetPrecordexpression(unsigned int index) {
  return (recordexpression *)GetPbaserecord(index);
}

recordfunction *array::GetPrecordfunction(unsigned int index) {
  return (recordfunction *)GetPbaserecord(index);
}

recordlist *array::GetPrecordlist(unsigned int index) {
  return (recordlist *)GetPbaserecord(index);
}

recordclause *array::GetPrecordclause(unsigned int index) {
  return (recordclause *)GetPbaserecord(index);
}

unsigned *array::GetPunsigned(unsigned index) {
  return (unsigned *)GetPbaserecord(index);
}

char *array::GetPchar(unsigned index) {
  return (char *)GetPbaserecord(index);
}

//============конец array========

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
  //(baserecord *)&heap->heaps[prf->func];
  if (pbr->ident == issymbol) {
    char *p = *Buf;
    PrintSconst(Buf, BufSize, (recordsconst *)pbr, ScVar, heap);
    //*Buf = 0;
    pldout(const_cast<char *>("finction:"));
    char Func[1024];
    int len = (*Buf - p);
    strncpy(Func, p, len);
    Func[len] = 0;
    pldout(const_cast<const char *>(Func));
  }
  unsigned *ptrarg = heap->GetPunsigned(prf->ptrarg);

  //(unsigned *)&heap->heaps[prf->ptrarg];
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
      //(baserecord *)&heap->heaps[ptrarg[i]];
      char sarg[255];
      sprintf(sarg, "argument index: %d", ptrarg[i]);
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
        int len = *Buf - p;
        if (len > 0) {
          strncpy(pBuf, p, len);
          pBuf[len] = 0;
          pldout(pBuf);
        }
        sprintf(pBuf, "PrintFunction: unknown ident: %d", pbr->ident);
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
  char pBuf[255];
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
    //(baserecord *)&heap->heaps[rl->head];
    switch (pbr->ident) {
    case issymbol: PrintSconst(Buf, BufSize, (recordsconst *)pbr, ScVar, heap); break;
    case isvar: PrintVar(Buf, BufSize, (recordvar *)pbr, ScVar, heap); break;
    case isinteger: PrintInteger(Buf, BufSize, (recordinteger *)pbr, ScVar, heap); break;
    case isunknown: PrintUnknown(Buf, BufSize, (recordunknown *)pbr, ScVar, heap); break;
    case islist: PrintList(Buf, BufSize, (recordlist *)pbr, ScVar, heap); break;
    case isemptylist: break;
    default:
      int len = *Buf - p;
      if (len > 0) {
        strncpy(pBuf, p, len);
        pBuf[len] = 0;
        pldout(pBuf);
      }
      sprintf(pBuf, "PrintList: unknown ident: %d", pbr->ident);
      pldout(pBuf);
    }
    unsigned rl_link = rl->link;
    unsigned ident = pbr->ident;
    if (rl->link != NULL && rl->link != isnil && pbr->ident != isemptylist)
      rl = heap->GetPrecordlist(rl->link);
    //(recordlist *)&heap->heaps[rl->link];
    else
      rl = (recordlist *)0;
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
  //(char *)&heap->heaps[rsc->ptrsymb];
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
  //(char *)&heap->heaps[prv->ptrsymb];
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
  sprintf(pBuf, "%d", pri->value);
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
  unsigned *ptarget = heap->GetPunsigned(rc->ptrtarget);
  //(unsigned *)&heap->heaps[rc->ptrtarget];
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
    //(recordfunction *)&heap->heaps[*(ptarget + i)];
    switch (rf->ident) {
    case isfunction: PrintFunction(&p, &BufSize, rf, ScVar, heap); break;
    case issymbol: PrintSconst(&p, &BufSize, (recordsconst *)rf, ScVar, heap); break;
    default: sprintf(_Buf, "PrintClause: unknown ident: %d", rf->ident);
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
    //(recordclause *)&heap->heaps[rc->next];
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
