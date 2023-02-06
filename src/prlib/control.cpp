#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "pdefs.h"
#include "pstructs.h"
#include "control.h"
#include "extfunc.h"
#include "functions.h"
#include <charconv>

char buff[2048];

void outerror(ErrorCode err) {
  errout(GetPrErrText(err));
}

unsigned menegerbp(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (name == ClVar->head || name == hpcut) {
    return argnull(name, ScVar, ClVar, heap);
  }
  if (name == hpout) {
    return prout(ScVar, ClVar, heap);
  }
  recordfunction *pfunc = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction*)&heap->heaps[ClVar->head];
  if (pfunc->ident == isfunction)
    if (bpred(name, pfunc->narg) == false) {
      outerror(ErrorCode::WrongNumberOfArgumentsInBuiltinPredicate);  // 26
      return 1;
    }
  switch (pfunc->narg) {
  case 1: return argone(name, ScVar, ClVar, heap);
  case 2: return argtwo(name, ScVar, ClVar, heap);
  case 3: return argthree(name, ScVar, ClVar, heap);
  case 4: return argfour(name, ScVar, ClVar, heap);
  case 5: return argfive(name, ScVar, ClVar, heap);
  }
  return 1;
}

// inline void to_stac(unsigned *b,unsigned index,unsigned value)
void to_stac(unsigned *b, unsigned index, unsigned value) {
  *(b + index) = value;
}

inline unsigned from_stac(unsigned *b, unsigned index) {
  return *(b + index);
}

int expand_stack(TClVar *ClVar) {
#ifdef _DEBUG_
  out("expand_stack");
#endif
  unsigned vmaxstacknew = ClVar->vmaxstack << 1;
  unsigned *st_conN;
  unsigned *st_vr1N;
  unsigned *st_vr2N;
  unsigned *st_trailN;
  /*
       st_conN = (unsigned *)malloc(sizeof(unsigned int) * vmaxstacknew);
       st_vr1N = (unsigned *)malloc(sizeof(unsigned int) * vmaxstacknew);
       st_vr2N = (unsigned *)malloc(sizeof(unsigned int) * vmaxstacknew);
       st_trailN = (unsigned *)malloc(sizeof(unsigned int) * vmaxstacknew);
  */
  st_conN = new unsigned int[vmaxstacknew];
  st_vr1N = new unsigned int[vmaxstacknew];
  st_vr2N = new unsigned int[vmaxstacknew];
  st_trailN = new unsigned int[vmaxstacknew];

  if (!st_conN || !st_vr1N || !st_vr2N || !st_trailN) {
    if (st_conN)
      delete[] st_conN;
    if (st_vr1N)
      delete[] st_vr1N;
    if (st_vr2N)
      delete[] st_vr2N;
    if (st_trailN)
      delete[] st_trailN;
    return 28; /*message_no_memory|=no_memory_stac;*/
  }

  unsigned i;

  for (i = 0; i < ClVar->vmaxstack; i++) {
    st_conN[i] = ClVar->st_con[i];
    st_vr1N[i] = ClVar->st_vr1[i];
    st_vr2N[i] = ClVar->st_vr2[i];
    st_trailN[i] = ClVar->st_trail[i];
  }
  for (; i < vmaxstacknew; i++) {
    st_conN[i] = isnil;
    st_vr1N[i] = isnil;
    st_vr2N[i] = isnil;
    st_trailN[i] = isnil;
  }
  delete[] ClVar->st_con;
  ClVar->st_con = st_conN;
  delete[] ClVar->st_vr1;
  ClVar->st_vr1 = st_vr1N;
  delete[] ClVar->st_vr2;
  ClVar->st_vr2 = st_vr2N;
  delete[] ClVar->st_trail;
  ClVar->st_trail = st_trailN;
  ClVar->vmaxstack = vmaxstacknew;
  return 0;
}

void from_control(TClVar *ClVar, array *heap)  // выборка из стека управления
{
  ClVar->newclause = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->atomp = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->aclause = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->parent = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->oldtptr = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->oldsvptr = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->frame2 = from_stac(ClVar->st_con, --ClVar->scptr);
  heap->paclause = heap->GetPrecordclause(ClVar->aclause);
  //(recordclause *)&heap->heaps[ClVar->aclause];
  heap->pacltarget = heap->GetPunsigned(heap->paclause->ptrtarget);
  //(unsigned *)&heap->heaps[heap->paclause->ptrtarget];
  if (ClVar->newclause && ClVar->newclause != isnil) {
    heap->pnclause = heap->GetPrecordclause(ClVar->newclause);
    //(recordclause *)&heap->heaps[ClVar->newclause];
    heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
    //(unsigned *)&heap->heaps[heap->pnclause->ptrtarget];
  }
}

void to_control(TClVar *ClVar, array *heap)  // запись в стек
{
  if (ClVar->scptr + 7 > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
    outerror(ErrorCode::StackOverflow);  // 28
    ClVar->stat = 5;                     /*message_no_memory|=no_memory_stac;*/
  } else {
    to_stac(ClVar->st_con, ClVar->scptr++, ClVar->frame2);
    to_stac(ClVar->st_con, ClVar->scptr++, ClVar->oldsvptr);
    to_stac(ClVar->st_con, ClVar->scptr++, ClVar->oldtptr);
    to_stac(ClVar->st_con, ClVar->scptr++, ClVar->parent);
    to_stac(ClVar->st_con, ClVar->scptr++, ClVar->aclause);
    to_stac(ClVar->st_con, ClVar->scptr++, ClVar->atomp);
    to_stac(ClVar->st_con, ClVar->scptr++, ClVar->newclause);
  }
}
//==========================вывод===============
unsigned term;
unsigned frame;
unsigned arg;
bool flaglist;

// переход по cвязи переменной на один шаг
void bound(unsigned *t, unsigned *f, unsigned *j, TClVar *ClVar, array *heap) {
  recordvar *prv = heap->GetPrecordvar(*t);
  //(recordvar *)&heap->heaps[*t];
  *j = *f + prv->num;                 // смещение в стеке
  *f = from_stac(ClVar->st_vr2, *j);  // новый кадр
  *t = from_stac(ClVar->st_vr1, *j);  // новый терм
}

unsigned occur_term(unsigned *TERM, unsigned *FRAME, TClVar *ClVar, array *heap) {
  int argt = -1;
  unsigned j;
  long i;   // для expression пока нет
  float f;  // тоже
  do {
    baserecord *tp = heap->GetPbaserecord(*TERM);
    //(baserecord *)&heap->heaps[*TERM];
    switch (tp->ident) {
    case isstring: argt = 9; break;
    case isfunction: argt = 8; break;
    case isinteger: argt = 7; break;
    case isfloat: argt = 6; break;
    case isvar: {
      bound(TERM, FRAME, &j, ClVar, heap);
      if (*TERM == isnil)
        argt = 5;
    } break;
    case issymbol: argt = 4; break;
    case islist: argt = 3; break;
    case isemptylist: argt = 2; break;
    case isunknown: argt = 1; break;
    case isexpression: {
      unsigned char ind = calculation(*TERM, *FRAME, &i, &f, ClVar, heap);
      switch (ind) {
      case isinteger: {
        recordinteger pi(i);
        *TERM = heap->apend(&pi, sizeof(recordinteger));
        if (*TERM < 0) {
          outerror(ErrorCode::TooLongList);  // 44
          argt = 0;
        }
      } break;
      case isfloat: {
        recordfloat pf(f);
        *TERM = heap->apend(&pf, sizeof(recordfloat));
        if (*TERM < 0) {
          outerror(ErrorCode::TooLongList);  // 44
          argt = 0;
        }
      } break;
      default: argt = 0;
      }
    } break;
    }
  } while (argt < 0);
  return argt;
}

bool stac(TClVar *ClVar, array *heap) {
  if (ClVar->bpt + 4 > &ClVar->BPT[_maxbptr_ - 1]) {
    return false;
  }
  *(ClVar->bpt++) = term;
  *(ClVar->bpt++) = frame;
  *(ClVar->bpt++) = arg;
  *(ClVar->bpt++) = flaglist;
  baserecord *tp = heap->GetPbaserecord(term);
  //(baserecord *)&heap->heaps[term];
  switch (tp->ident) {
  case islist: {
    recordlist *pl = (recordlist *)tp;
    term = pl->head;
  } break;
  case isfunction: {
    recordfunction *pf = (recordfunction *)tp;
    unsigned *ptrarg = heap->GetPunsigned(pf->ptrarg);
    //(unsigned *)&heap->heaps[pf->ptrarg];
    term = ptrarg[arg - 2];
  } break;
  }
  return true;
}

// j для печати разделителя
bool nextarg(unsigned *j, TScVar *ScVar, TClVar *ClVar, array *heap) {
  bool nxaex;
  do {
    nxaex = true;
    if (ClVar->bpt == ClVar->BPT) {
      return false;
    }
    flaglist = (bool)*(--ClVar->bpt);
    arg = *(--ClVar->bpt) + 1;
    frame = *(--ClVar->bpt);
    term = *(--ClVar->bpt);
    baserecord *tp = heap->GetPbaserecord(term);
    //(baserecord *)&heap->heaps[term];
    switch (tp->ident) {
    case isfunction: {
      recordfunction *pf = (recordfunction *)tp;
      if (arg - 1 > (unsigned)pf->narg) {
        buff[*j] = ')';
        (*j)++;
        nxaex = false;
      } else {
        buff[*j] = ',';
        (*j)++;
        nxaex = stac(ClVar, heap);
      }
    } break;
    case islist:
      recordlist *pl = (recordlist *)tp;
      {
        if (arg == 2) {
          buff[*j] = ']';
          (*j)++;
          nxaex = false;
        } else {
          unsigned trm = term;
          trm = pl->link;
          unsigned ff = frame;
          int a = occur_term(&trm, &ff, ClVar, heap);
          switch (a) {
          case 3:
            buff[*j] = ',';
            (*j)++;
            break;
          case 2:
            buff[*j] = ']';
            (*j)++;
            break;
          default:
            buff[*j] = '|';
            (*j)++;
            // term=trm;
            nxaex = stac(ClVar, heap);
            //  term=precordlist->link;
          }
          term = pl->link;
        }
      }
      break;
    }
  } while (!nxaex);
  return true;
}

// extern unsigned write_term(unsigned TERM,unsigned FRAME,unsigned w,unsigned j);//,FILE *d=NULL);
// прологовский вывод
unsigned prout(TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned k = 0;
  recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction *)&heap->heaps[ClVar->head];
  unsigned *ptr = heap->GetPunsigned(pf->ptrarg);
  //(unsigned *)&heap->heaps[pf->ptrarg];
  unsigned count = (unsigned)pf->narg;
  for (unsigned i = 0; i < count; i++) {
    pf = heap->GetPrecordfunction(ClVar->head);
    //(recordfunction *)&heap->heaps[ClVar->head];
    ptr = heap->GetPunsigned(pf->ptrarg);
    //(unsigned *)&heap->heaps[pf->ptrarg];
    k = write_term(ptr[i], ClVar->frame2, i, k, ScVar, ClVar, heap);
  }
  buff[k] = 0;
  if (ClVar->PrSetting->out.is_open()) {
    ClVar->PrSetting->out << buff;
  } else {
    usrout(buff);
  }
  return 3;
}

unsigned write_term(unsigned TERM, unsigned FRAME, unsigned W, unsigned j, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned i = 0;
  term = TERM;
  frame = FRAME;

  // unsigned bpt[maxbptr];
  //  bpt=BPT=new unsigned[maxbptr];
  if (!ClVar->bpt) {
    outerror(ErrorCode::NotEnoughFreeMemory);  // 2
    return j;
  }
  bool ex = true;
  flaglist = false;
  arg = 0;
  while (ex) {                          // здесь цикл8ический вывод терма
    if (term == isnil || term == NULL)  // || !term)
    {
      buff[j++] = '_';
      auto [ptr, ec] = std::to_chars(buff + j, buff + sizeof(buff), W);
      j = ptr - buff;
      // Заменено на to_chars, но не поверено
      //_itoa(W, (char*)&buff[j], 10);
      // for (; (j < sizeof(buff) - 1 && buff[j]); j++);

      ex = nextarg(&j, ScVar, ClVar, heap);
    } else {
      baserecord *tp = heap->GetPbaserecord(term);
      //(baserecord *)&heap->heaps[term];
      switch (tp->ident) {
      case isfunction: {
        ClVar->precordfunction = (recordfunction *)tp;
        ClVar->precordsconst = heap->GetPrecordsconst(ClVar->precordfunction->func);
        //(recordsconst *)&heap->heaps[ClVar->precordfunction->func];
        char *name = heap->GetPchar(ClVar->precordsconst->ptrsymb);
        //(char *)&heap->heaps[ClVar->precordsconst->ptrsymb];

        for (i = 0; i < (unsigned)ClVar->precordsconst->length; buff[j++] = *(name + i), i++)
          ;
        buff[j++] = '(';
        arg = 2;
        flaglist = false;
        ex = stac(ClVar, heap);
        i = 0;
      } break;  // ok
      case isinteger: {
        ClVar->precordinteger = (recordinteger *)tp;
        auto [ptr, ec] = std::to_chars(buff + j, buff + sizeof(buff), ClVar->precordinteger->value);
        j = ptr - buff;
        // Заменено на to_chars, но не поверено
        //_ltoa(ClVar->precordinteger->value, (char*)&buff[j], 10);
        // for (; j < sizeof(buff) - 1 && buff[j]; j++);

        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;  // ok
      case isfloat: {
        ClVar->precordfloat = (recordfloat *)tp;
        char string[40];
        sprintf(string, "%g", ClVar->precordfloat->value);
        for (i = 0; j < sizeof(buff) - 1 && string[i]; buff[j++] = string[i++])
          ;
        buff[j] = 0;
        i = 0;
        ex = nextarg(&j, ScVar, ClVar, heap);  // ok
      } break;
      case isvar: {
        bound(&term, &frame, &W, ClVar, heap);
      } break;  // ok
      case isstring:
      case issymbol: {
        ClVar->precordsconst = (recordsconst *)tp;
        char *name = heap->GetPchar(ClVar->precordsconst->ptrsymb);
        //(char *)&heap->heaps[ClVar->precordsconst->ptrsymb];
        for (i = 0; i < (unsigned)ClVar->precordsconst->length; buff[j + i] = *(name + i), i++)
          ;
        j += i;
        i = 0;
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;  // ok
      case islist: {
        if (!(arg && flaglist))  // список встретился
        {
          buff[j++] = '[';
          flaglist = true;
        }
        arg = 0;
        ex = stac(ClVar, heap);
      } break;
      case isemptylist: {
        if (arg != 1) {
          buff[j++] = '[';
          buff[j++] = ']';
        }
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;
      case iscut: {
        buff[j++] = '!';
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;
      case isunknown:
        buff[j++] = '_';
        ex = nextarg(&j, ScVar, ClVar, heap);
        break;
      case isexpression:
        long ii;
        float ff;
        unsigned char ind = calculation(term, frame, &ii, &ff, ClVar, heap);
        switch (ind) {
        case isinteger: {
          auto [ptr, ec] = std::to_chars(buff + j, buff + sizeof(buff), ii);
          // auto [ptr, ec] = std::to_chars(buff + j, buff + sizeof(buff), ii);
          j = ptr - buff;
          // Заменено на to_chars, но не поверено
          //_ltoa(ii, (char*)&buff[j], 10);
          // for (; j < sizeof(buff) - 1 && buff[j]; j++);

        } break;
        case isfloat:
          char string[40];
          sprintf(string, "%g", ff);
          for (i = 0; j < sizeof(buff) - 1 && string[i]; buff[j++] = string[i++])
            ;
          buff[j] = 0;
          i = 0;
          break;
        /*                   case isfloat  :    заменено но не проверено
                                 int dec, sign, ndig = 5;
                                 char string[200];
                                 char *p=&string[0];
                                 p=fcvt(precordfloat->value,ndig, &dec, &sign);
                                 if (sign) buff[j++]='-';
                                 for(i=0;((i<dec) && (i+j<200));buff[j+i]=p[i],i++);
                                 buff[j+i]='.';j++;
                                 for(;((i+j)<200 && p[i]);buff[i+j]=p[i],i++);
                                 j+=i;i=0;
                                 break;
        */
        default:
          char *s = const_cast<char *>("#_?_#");
          for (i = 0; i < 5 && i + j < sizeof(buff) - 1; buff[i + j] = s[i], i++)
            ;
          j += i;
          i = 0;
          break;
        }
        ex = nextarg(&j, ScVar, ClVar, heap);
        break;
      }
    }
  }
  return j + i;
}

// пока только описание функции
void prvars(TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!ClVar->varqu) {
    prdout(true);
    ClVar->flag = false;
    ClVar->stat = 1;
  } else {
    ClVar->flag = false;
    // bpt=buf;//указатель на внешний buf
    //   bpt=BPT=new unsigned[maxbptr];
    //   unsigned i=0,j=0;
    unsigned j = 0;
    // out(const_cast<char*>(""));
    unsigned w;
    for (unsigned k = 0; k < ClVar->varqu; k++) {
      w = (unsigned)k;
      ClVar->precordvar = heap->GetPrecordvar(ScVar->tvar[k]);
      //(recordvar *)&heap->heaps[ScVar->tvar[k]];
      char *name = heap->GetPchar(ClVar->precordvar->ptrsymb);
      //(char *)&heap->heaps[ClVar->precordvar->ptrsymb];
      for (j = 0; j < (unsigned)ClVar->precordvar->length; buff[j] = *(name + j), j++)
        ;
      buff[j++] = '=';
      frame = from_stac(ClVar->st_vr2, k);
      term = from_stac(ClVar->st_vr1, k);
      //======================================// вывод terma  пока не все
      buff[write_term(term, frame, w, j, ScVar, ClVar, heap)] = 0;  //,//FILE *d)
      pldout(buff);
    }  // delete BPT;
  }
}

// конец вывода
void zero(TClVar *ClVar) {
  for (unsigned i = ClVar->oldtptr; i < ClVar->tptr; i++)
    to_stac(ClVar->st_vr1, from_stac(ClVar->st_trail, i), isnil);
  ClVar->tptr = ClVar->oldtptr;
}

void st_4(TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (ClVar->newclause && ClVar->newclause != isnil) {  // pnclause=head->at(newclause);pncltarget=head->at(newclause-1);
    heap->ptclause = heap->pnclause;
    heap->ptcltarget = heap->pncltarget;
    ClVar->tclause = ClVar->newclause;
    ClVar->tryclause = 0;  // ptclause->ptrtarget[tryclause] ук на голову
    // tryclause=ptclause->head; //ук на голову
    ClVar->frame1 = ClVar->oldsvptr = ClVar->svptr;
    ClVar->svptr = ClVar->frame1 + heap->pnclause->nvars;
#ifdef PrintVarCount
    char _buf[128];
    sprintf(_buf, "nvars:%d", ClVar->pnclause->nvars);
    out(_buf);
#endif
    ClVar->newclause = heap->pnclause->next;
    if (ClVar->newclause) {
      heap->pnclause = heap->GetPrecordclause(ClVar->newclause);
      //(recordclause *)&heap->heaps[ClVar->newclause];
      heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
      //(unsigned *)&heap->heaps[heap->pnclause->ptrtarget];
    }
    if (ClVar->svptr > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
      ClVar->err = ErrorCode::StackOverflow;  // 28;
      ClVar->stat = 1;
    } else {
      if (ClVar->PrSetting->Trace) {
        pldout("Согласуется с ");
        buff[write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame1, 0, 0, ScVar, ClVar, heap)] = 0;
        pldout(buff);
      }
      ClVar->oldtptr = ClVar->tptr;
      if (unify(heap->ptcltarget[ClVar->tryclause], ClVar->head, ClVar->frame1, ClVar->frame2, ClVar, heap)) {  // удачная унификация
        if (ClVar->newclause || heap->pacltarget[ClVar->atomp + 1]) {
          to_control(ClVar, heap);
          ClVar->ntro = true;
        } else
          ClVar->ntro = false;
        if (ClVar->stat != 5) {
          if (ClVar->PrSetting->Trace)
            pldout("Успех");
          ClVar->oldtptr = ClVar->tptr;
          ClVar->oldsvptr = ClVar->svptr;
          ClVar->atomp = ClVar->tryclause;
          // atomp=-1;
          ClVar->aclause = ClVar->tclause;
          heap->paclause = heap->ptclause;
          heap->pacltarget = heap->ptcltarget;
          ClVar->stat = 2;
        }
      } else if (ClVar->stat != 1) {
        if (ClVar->PrSetting->Trace)
          pldout("Неудача");
        ClVar->svptr = ClVar->oldsvptr;
        zero(ClVar);
        ClVar->stat = 4;
      }
    }
  } else {
    if (ClVar->PrSetting->Trace) {
      if (heap->ptcltarget) {
        pldout("Цель");
        buff[write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
      }
      pldout("Неудачна ");
    }
    ClVar->stat = 5;
  }
}

void OnControl(TClVar *ClVar, array *heap) {
  // add 2008/10/04
  heap->ptclause = NULL;
  heap->ptcltarget = NULL;

  unsigned int &vmaxstack = ClVar->vmaxstack;

  ClVar->err = ErrorCode::NoErrors; // 0
  unsigned i;
  for (i = 0; i < ClVar->vmaxstack; i++) {
    ClVar->st_con[i] = isnil;
    ClVar->st_vr1[i] = isnil;
    ClVar->st_vr2[i] = isnil;
    ClVar->st_trail[i] = isnil;
  }
  for (i = 0; i < _maxbf_; i++) {
    ClVar->bf[i] = 0;
  }
  ClVar->stat = 2;
  ClVar->newclause = NULL;
  ClVar->ntro = false;  // нет оптимизации хвостовой рекурсии не знаю пока что это
  ClVar->flag = true;
  ClVar->oldtptr = ClVar->tptr = NULL;  // индекс стека следа
  ClVar->scptr = NULL;
  ClVar->frame1 = NULL;
  ClVar->parent = NULL;
  // aclause = heap->last - sizeof(recordclause);                          //чего это ?
  ClVar->aclause = heap->query;
  heap->paclause = heap->GetPrecordclause(ClVar->aclause);
  //(recordclause *)&heap->heaps[ClVar->aclause];
  heap->pacltarget = heap->GetPunsigned(heap->paclause->ptrtarget);
  //(unsigned *)&heap->heaps[heap->paclause->ptrtarget];
  ClVar->varqu = heap->paclause->nvars;
  ClVar->svptr = ClVar->varqu;
  ClVar->atomp = -1;
}

ErrorCode control(TScVar *ScVar, TClVar *ClVar, array *heap, bool *EnableRunning) {
  OnControl(ClVar, heap);
  while (ClVar->stat != 1 && ClVar->err == ErrorCode::NoErrors) {
    switch (ClVar->stat) {
    case 2:
      if (ClVar->ntro) {
        ClVar->parent = ClVar->scptr;  // место записи в стеке управления о вызывающем предл
      }
      ClVar->frame2 = ClVar->frame1;   // если stat = 2 то выполнение сразу же и 3
      [[fallthrough]];
    case 3:
      if (!heap->pacltarget[++ClVar->atomp])  // все цели удовлетворены
        if (ClVar->parent == NULL)            // есть ответ на вопрос
        {
          prvars(ScVar, ClVar, heap);  // печать переменных
          if (ClVar->stat != 1) {
            ClVar->stat = 5;  // неудача - не найдено решение
          }
        } else {
          unsigned work = ClVar->scptr;
          ClVar->scptr = ClVar->parent;  // зачем ?
          from_control(ClVar, heap);     // выборка родительской среды
          ClVar->scptr = work;
          ClVar->stat = 3;
        }
      else {
        ClVar->head = heap->pacltarget[ClVar->atomp];  // индекс новой цели
        ClVar->stat = 6;
      }
      break;
    case 6:
      if (ClVar->PrSetting->Trace) {
        pldout("Цель ");
        buff[write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
        pldout(buff);
      }
      ClVar->precordfunction = heap->GetPrecordfunction(ClVar->head);
      //(recordfunction *)&heap->heaps[ClVar->head];
      switch (ClVar->precordfunction->ident) {
      case isfunction: ClVar->bipr = ClVar->precordfunction->func; break;
      case iscut: {
        recordcut *pcut = (recordcut *)ClVar->precordfunction;
        ClVar->bipr = pcut->func;
      } break;

      default: ClVar->bipr = ClVar->head;
      }

      if (ClVar->bipr < heap->freeheap) {
        ClVar->stat = menegerbp(ClVar->bipr, ScVar, ClVar, heap);
      } else {
        ClVar->precordsconst = heap->GetPrecordsconst(ClVar->bipr);
        //(recordsconst *)&heap->heaps[ClVar->bipr];
        ClVar->newclause = ClVar->precordsconst->begin;
        if (ClVar->newclause && ClVar->newclause != isnil) {
          heap->pnclause = heap->GetPrecordclause(ClVar->newclause);
          //(recordclause *)&heap->heaps[ClVar->newclause];
          heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
          //(unsigned *)&heap->heaps[heap->pnclause->ptrtarget];
        }
        ClVar->stat = 4;
      }
      break;
    case 4: st_4(ScVar, ClVar, heap); break;
    case 5:
      if (!ClVar->scptr) {
        ClVar->stat = 1;
      } else {
        from_control(ClVar, heap);
        zero(ClVar);
        ClVar->head = heap->pacltarget[ClVar->atomp];
        ClVar->svptr = ClVar->oldsvptr;
        if (ClVar->PrSetting->Trace) {
          pldout("Переделка. Цель ");
          buff[write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
          pldout(buff);
        }
        ClVar->stat = 4;
      }
    }
    if (!*EnableRunning) {
      ClVar->err = ErrorCode::ExecutionInterrupted;  // 27;  // abort
    }
  }
  if (ClVar->flag) {
    prdout(false);
  }
  return ClVar->err;
}

// ================ унификация =======================
// возврат 0 ошибка,isinteger-результат целое,isfloat-результат вешещественное
unsigned char calculation(unsigned term, unsigned frame, long *i, float *f, TClVar *ClVar, array *heap) {
  unsigned char ind = 0;  // первоначально попытка вычислить выражение с целымы

  void *st[maxstaccalc];
  int index = 0;  // индекс в массиве st указывает на свободный эл-нт
                  //  array st(maxstaccalc);
  ClVar->precordexpression = heap->GetPrecordexpression(term);
  //(recordexpression *)&heap->heaps[term];
  unsigned char n = (unsigned char)ClVar->precordexpression->length;
  ErrorCode err = ErrorCode::NoErrors;
  baserecord *tp;
  long oi1 = 0, oi2 = 0;                 // операнды для вычисления целых
  float of1 = 0, of2 = 0;                // операнды для  вычисления вещественных
  for (int j = 0; j < n && err == ErrorCode::NoErrors; j++) {  // if (lowMemory()) return 0;//!!!сообщение о r_t_e
    unsigned *ptr = heap->GetPunsigned(ClVar->precordexpression->precord);
    //(unsigned *)&heap->heaps[ClVar->precordexpression->precord];
    if (index == maxstaccalc) {
      err = ErrorCode::StackOverflowWhileCalculatingArithmeticsExpressions;  // 40;
      continue;
    }  // это переход на начало цикла
       // и немедленное его завершение(переполнился стек)
    if (ptr[j] < isbase)  // операнд
    {
      tp = heap->GetPbaserecord(ptr[j]);
      //(baserecord *)&heap->heaps[ptr[j]];
      switch (tp->ident) {
      case isinteger: {
        ClVar->precordinteger = (recordinteger *)tp;
        st[index++] = new recordinteger(ClVar->precordinteger->value);
        oi2 = ClVar->precordinteger->value;
        ind = isinteger;
        if (!st[index - 1]) {
          err = ErrorCode::NotEnoughFreeMemory;  // 2;
          continue;
        }
      } break;
      case isfloat: {
        ClVar->precordfloat = (recordfloat *)tp;
        st[index++] = new recordfloat(ClVar->precordfloat->value);
        of2 = ClVar->precordfloat->value;
        ind = isfloat;
        if (!st[index - 1]) {
          err = ErrorCode::NotEnoughFreeMemory;  // 2;
          continue;
        }
      } break;
      case isvar: {
        unsigned tr = ptr[j], fr = frame;
        int a = occur_term(&tr, &fr, ClVar, heap);
        switch (a) {
        case 6: {
          ClVar->precordfloat = heap->GetPrecordfloat(tr);
          //(recordfloat *)&heap->heaps[tr];
          st[index++] = new recordfloat(ClVar->precordfloat->value);
          of2 = ClVar->precordfloat->value;
          ind = isfloat;
          if (!st[index - 1]) {
            err = ErrorCode::NotEnoughFreeMemory;  // 2;
            continue;
          }
        } break;
        case 7: {
          ClVar->precordinteger = heap->GetPrecordinteger(tr);
          //(recordinteger *)&heap->heaps[tr];
          st[index++] = new recordinteger(ClVar->precordinteger->value);
          oi2 = ClVar->precordinteger->value;
          ind = isinteger;
          if (!st[index - 1]) {
            err = ErrorCode::NotEnoughFreeMemory;  // 2;
            continue;
          }
        } break;
        default: {
          err = ErrorCode::ErrorInArithmeticExpression;  // 32;
        }
        }
      } break;
      default: {
        err = ErrorCode::ErrorInArithmeticExpression;  // 32;  // других типов не должно быть
      }
      }
    } else { // знак ариафм операции
      for (int k = 0; k < 2; k++) {
        tp = (baserecord *)st[index - 1];
        switch (tp->ident)  // может быть только integer или float
        {
        case isinteger:
          ClVar->precordinteger = (recordinteger *)tp;
          if (!k) {
            oi1 = ClVar->precordinteger->value;
            ind = isinteger;
          }  // первый операнд
          else if (ind == isinteger) {
            oi2 = ClVar->precordinteger->value;  // второй операнд
          } else {
            of2 = (float)ClVar->precordinteger->value;  // если первый вещественный
          }
          delete st[index - 1];
          index--;
          break;
        case isfloat:
          ClVar->precordfloat = (recordfloat *)tp;
          if (!k) {
            of1 = ClVar->precordfloat->value;
            ind = isfloat;
          }     // первый операнд
          else  // второй операнд
          {
            of2 = ClVar->precordfloat->value;
            if (ind == isinteger) {
              ind = isfloat;
              of1 = (float)oi1;
            }
          }
          delete st[index - 1];
          index--;
          break;
        }
      }  // выбраны из стека два операнда
      switch (ptr[j]) {
      case isminus:
        if (ind == isinteger) {
          oi2 -= oi1;
        } else {
          of2 -= of1;
        }
        break;
      case isplus:
        if (ind == isinteger) {
          oi2 += oi1;
        } else {
          of2 += of1;
        }
        break;
      case ismult:
        if (ind == isinteger) {
          oi2 *= oi1;
        } else {
          of2 *= of1;
        }
        break;
      case isslash:
        if (ind == isinteger) {
          if (oi1 == 0) {
            err = ErrorCode::DivisionByZeroWhileCalculatingArithmeticsExpressions; // 41
          } else {
            oi2 /= oi1;
          }
        } else {
          if (of1 == 0) {
            err = ErrorCode::DivisionByZeroWhileCalculatingArithmeticsExpressions;  // 41
          } else {
            of2 /= of1;
          }
        }
        break;
      case ismod:
        if (ind == isinteger) {
          if (oi1 == 0) {
            err = ErrorCode::DivisionByZeroWhileCalculatingArithmeticsExpressions;  // 41
          } else {
            oi2 = oi2 % oi1;
          }
        } else {
          err = ErrorCode::ModOperationAppliedToRealOperand;  // 34;
        }
        break;
      case isdiv: {
        if (ind == isinteger) {
          if (oi1 == 0) {
            err = ErrorCode::DivisionByZeroWhileCalculatingArithmeticsExpressions;  // 41;
          } else {
            oi2 /= oi1;
          }
        } else {
          err = ErrorCode::DivOperationAppliedToRealOperand;  // 35;
        }
      } break;
      }
      st[index++] = (ind == isinteger) ? (void *)new recordinteger(oi2) : (void *)new recordfloat(of2);
      if (!st[index - 1])
        err = ErrorCode::NotEnoughFreeMemory;  // 2;
    }
  }
  *i = oi2;
  *f = of2;
  if (err != ErrorCode::NoErrors) {
    outerror(err);
  }
  if (index != 1) {
    ind = 0;
  }
  while (index != 0) {
    delete st[--index];
  }
  return ind;
}

bool stac_un(TClVar *ClVar) {
  if (ClVar->bp + 5 > maxbf) {
    return false;
  }
  ClVar->bf[ClVar->bp++] = ClVar->numb;
  ClVar->bf[ClVar->bp++] = ClVar->fr2;
  ClVar->bf[ClVar->bp++] = ClVar->fr1;
  ClVar->bf[ClVar->bp++] = ClVar->term2;
  ClVar->bf[ClVar->bp++] = ClVar->term1;
  return true;
}

void nextun(TClVar *ClVar, array *heap) {
  if (!ClVar->bp) {
    ClVar->ex = false;
    return;
  }
  ClVar->term1 = ClVar->bf[--ClVar->bp];
  ClVar->term2 = ClVar->bf[--ClVar->bp];
  ClVar->fr1 = ClVar->bf[--ClVar->bp];
  ClVar->fr2 = ClVar->bf[--ClVar->bp];
  ClVar->numb = ++ClVar->bf[--ClVar->bp];
  baserecord *tp = heap->GetPbaserecord(ClVar->term1);
  //(baserecord *)&heap->heaps[ClVar->term1];
  if (tp->ident == isfunction) {
    ClVar->precordfunction = (recordfunction *)tp;
    if ((unsigned)ClVar->precordfunction->narg > ClVar->numb) {
      unsigned *ptr = heap->GetPunsigned(ClVar->precordfunction->ptrarg);
      //(unsigned *)&heap->heaps[ClVar->precordfunction->ptrarg];
      ClVar->term1 = ptr[ClVar->numb];
      ClVar->precordfunction = heap->GetPrecordfunction(ClVar->term2);
      //(recordfunction*)&heap->heaps[ClVar->term2];
      ptr = heap->GetPunsigned(ClVar->precordfunction->ptrarg);
      //(unsigned *)&heap->heaps[ClVar->precordfunction->ptrarg];
      ClVar->term2 = ptr[ClVar->numb];
      //        if (numb<precordfunction->narg)//1999
      ClVar->bp += 5;
    } else {
      nextun(ClVar, heap);
    }
    //       {if (bp>4)
    //         { term1=bf[--bp];
    //            term2=bf[--bp];
    //            fr1  =bf[--bp];
    //            fr2  =bf[--bp];
    //            numb =++bf[--bp];
    //            numb++;
    //         }
    //        else bp-=5;
    //       }
    //      if (bp<=0) {ex=false;numb=0;} //1999
  } else if (tp->ident == islist) {
    ClVar->precordlist = (recordlist *)tp;
    if (ClVar->numb < 2) {
      switch (ClVar->numb) {
      case 0:
        ClVar->term1 = ClVar->precordlist->head;
        ClVar->precordlist = heap->GetPrecordlist(ClVar->term2);
        //(recordlist *)&heap->heaps[ClVar->term2];
        ClVar->term2 = ClVar->precordlist->head;
        ClVar->bp += 5;
        break;
      case 1:
        recordlist *precordlist2 = heap->GetPrecordlist(ClVar->term2);
        //(recordlist *)&heap->heaps[ClVar->term2];
        if (ClVar->precordlist && precordlist2) {
          ClVar->term1 = ClVar->precordlist->link;
          ClVar->term2 = precordlist2->link;
        }
        break;
      }

    } else if (!ClVar->bp) {
      ClVar->ex = false;
      ClVar->numb = 0;
    }
  }
}

void swapints(TClVar *ClVar) {
  unsigned c;
  c = ClVar->term1;
  ClVar->term1 = ClVar->term2;
  ClVar->term2 = c;
  c = ClVar->fr1;
  ClVar->fr1 = ClVar->fr2;
  ClVar->fr2 = c;
}

bool unfun(TClVar *ClVar, array *heap) {
  bool ret = false;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
  //(baserecord *)&heap->heaps[ClVar->term2];
  switch (tp->ident) {
  case isinteger:            // окончательно
  case isfloat:              // окончательно
  case isstring:             // окончательно
  case issymbol:             // окончательно
  case islist:               // окончательно
  case isemptylist:          // окончательно
  case isexpression: break;  // ret уже false;
  case isvar:
    swapints(ClVar);
    ret = true;
    break;
  case isunknown:
    nextun(ClVar, heap);
    ret = true;
    break;
  case isfunction:
    recordfunction *pf1 = heap->GetPrecordfunction(ClVar->term1),
                   //(recordfunction *)&heap->heaps[ClVar->term1],
      *pf2 = (recordfunction *)tp;
    if ((pf1->narg == pf2->narg) && (pf1->func == pf2->func)) {
      ClVar->numb = -1;
      ret = stac_un(ClVar);
      nextun(ClVar, heap);
    }
  }
  return ret;
}

bool unsymb(TClVar *ClVar, array *heap) {
  bool ret = true;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
  //(baserecord *)&heap->heaps[ClVar->term2];
  switch (tp->ident)  // указатель на след униф терм
  {
  case isstring:
  case issymbol: {
    recordstring *pstr1 = heap->GetPrecordstring(ClVar->term1),
                 //(recordstring*)&heap->heaps[ClVar->term1],
      *pstr2 = (recordstring *)tp;
    char *name1 = heap->GetPchar(pstr1->ptrsymb);
    //(char *)&heap->heaps[pstr1->ptrsymb],
    char *name2 = heap->GetPchar(pstr2->ptrsymb);
    //(char *)&heap->heaps[pstr2->ptrsymb];
    if (pstr1->length == pstr2->length && !strncmp(name1, name2, pstr1->length))
    /*if (term1==term2)*/
    { /*bf[bp-5]++;*/
      nextun(ClVar, heap);
    } else
      ret = false;
  } break;
  case isvar: swapints(ClVar); break;
  case isfunction:
  case isinteger:  // окончательно
  case isfloat:
  case islist:
  case isemptylist:
  case isexpression: ret = false; break;
  case isunknown: nextun(ClVar, heap); break;
  }
  return ret;
}

// установка связи в стеке переменных
void ocrn(unsigned bd1, unsigned tr2, TClVar *ClVar) {
  to_stac(ClVar->st_vr1, bd1, tr2);  // и отметка в стеке следа
  to_stac(ClVar->st_trail, ClVar->tptr++, bd1);
}

bool unvar(TClVar *ClVar, array *heap) {
  bool ret = true;
  unsigned bd1, bd2, work;
  recordvar *precordvar = heap->GetPrecordvar(ClVar->term1);
  //(recordvar *)&heap->heaps[ClVar->term1];
  bd1 = precordvar->num + ClVar->fr1;
  work = from_stac(ClVar->st_vr1, bd1);
  if (work != isnil)  // связанная переменная
  {
    ClVar->term1 = work;
    work = from_stac(ClVar->st_vr2, bd1);
    if (work != isnil)
      ClVar->fr1 = work;
    //   else {ocrn(bd1,term2);nextun();}//!!!добавлено мной
  } else {
    baserecord *tp = heap->GetPbaserecord(ClVar->term2);
    //(baserecord *)&heap->heaps[ClVar->term2];

    switch (tp->ident) {
    case isfunction:
    case islist: to_stac(ClVar->st_vr2, bd1, ClVar->fr2);  // break; пока убрал break
    case isfloat:                                          // также как issumbol;
    case isinteger:                                        // так же как issymbol
    case isstring:
    case isemptylist:  // ret=false;break; было так но не сработала  стрспис
    case issymbol:
      ocrn(bd1, ClVar->term2, ClVar);
      nextun(ClVar, heap);
      break;
    case isvar:
      if (ClVar->fr1 < ClVar->fr2) {
        swapints(ClVar);
      } else {
        precordvar = heap->GetPrecordvar(ClVar->term2);
        //(recordvar *)&heap->heaps[ClVar->term2];
        bd2 = precordvar->num + ClVar->fr2;
        work = from_stac(ClVar->st_vr1, bd2);
        if (work == isnil) {  // свободная переменная
          if (ClVar->term1 == ClVar->term2) {
            nextun(ClVar, heap);
          } else {
            to_stac(ClVar->st_vr2, bd1, ClVar->fr2);
            ocrn(bd1, ClVar->term2, ClVar);
            nextun(ClVar, heap);
          }
        } else { // связанная переменная
          ClVar->term2 = work;
          work = from_stac(ClVar->st_vr2, bd2);
          if (work != isnil)
            ClVar->fr2 = work;
        }
      }
      break;
    case isunknown: nextun(ClVar, heap); break;
    case isexpression:
      long a;
      float af;
      unsigned char ind = calculation(ClVar->term2, ClVar->frame2, &a, &af, ClVar, heap);
      switch (ind) {
      case 0: ret = false; break;  //!!!abort_calculation;
      case isinteger: {
        recordinteger ptr(a);
        ClVar->term2 = heap->apend(&ptr, sizeof(recordinteger));
        if (ClVar->term2 < 0) {
          outerror(ErrorCode::TooLongList);  // 44
          ret = false;
        }
      } break;
      case isfloat: {
        recordfloat ptrf(af);
        ClVar->term2 = heap->apend(&ptrf, sizeof(recordfloat));
        if (ClVar->term2 < 0) {
          outerror(ErrorCode::TooLongList);  // 44
          ret = false;
        }
      } break;
      }
      break;
    }
  }
  return ret;
}

bool unint(TClVar *ClVar, array *heap) {
  bool ret = false;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
  //(baserecord *)&heap->heaps[ClVar->term2];
  switch (tp->ident) {
  case isfunction:
  case isstring:
  case issymbol:
  case islist:
  case isemptylist: break;  // ээто окончательно
  case isinteger: {
    recordinteger *precordinteger = (recordinteger *)tp;
    recordinteger *pint = heap->GetPrecordinteger(ClVar->term1);
    //(recordinteger *)&heap->heaps[ClVar->term1];
    if (precordinteger->value == pint->value) {
      ret = true;
      nextun(ClVar, heap);
    }
  } break;
  case isfloat: {
    recordfloat *precordfloat = (recordfloat *)tp;
    recordinteger *precordinteger = heap->GetPrecordinteger(ClVar->term1);
    //(recordinteger *)&heap->heaps[ClVar->term1];
    if (precordfloat->value - 1 < precordinteger->value && precordfloat->value == (float)precordinteger->value) {
      ret = true;
      nextun(ClVar, heap);
    }
  } break;  // ret уже false
  case isvar: {
    swapints(ClVar);
    ret = true;
  } break;
  case isunknown: {
    nextun(ClVar, heap);
    ret = true;
  } break;
  case isexpression: {
    float af;
    long a;
    unsigned char ind = calculation(ClVar->term2, ClVar->frame2, &a, &af, ClVar, heap);
    recordinteger *pint = heap->GetPrecordinteger(ClVar->term1);
    //(recordinteger *)&heap->heaps[ClVar->term1];
    if (ind == isinteger && pint->value == a) {
      ret = true;
      nextun(ClVar, heap);
    }
  } break;
  }
  return ret;
}

bool unfloat(TClVar *ClVar, array *heap) {
  bool ret = false;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
  //(baserecord *)&heap->heaps[ClVar->term2];
  switch (tp->ident) {
  case isfunction:
  case isstring:
  case issymbol:
  case islist:
  case isemptylist: break;  // ээто окончательно
  case isinteger: {
    recordfloat *precordfloat = heap->GetPrecordfloat(ClVar->term1);
    //(recordfloat *)&heap->heaps[ClVar->term1];
    recordinteger *precordinteger = (recordinteger *)tp;
    if (precordfloat->value - 1 < precordinteger->value && (float)precordinteger->value == precordfloat->value) {
      ret = true;
      nextun(ClVar, heap);
    }
  } break;
  case isfloat: {
    recordfloat *precordfloat = (recordfloat *)tp;
    recordfloat *pfl = heap->GetPrecordfloat(ClVar->term1);
    //(recordfloat *)&heap->heaps[ClVar->term1];
    if (precordfloat->value == pfl->value) {
      ret = true;
      nextun(ClVar, heap);
    }
  } break;

  case isvar: {
    swapints(ClVar);
    ret = true;
  } break;
  case isunknown: {
    nextun(ClVar, heap);
    ret = true;
  } break;
  case isexpression: break;
  }
  return ret;
}

bool unlist(TClVar *ClVar, array *heap) {
  bool ret = false;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
  //(baserecord *)&heap->heaps[ClVar->term2];
  switch (tp->ident) {
  case isfunction:           // окончательно
  case isinteger:            // окночательно
  case isfloat:              // окночательно
  case isstring:             // окончательно
  case issymbol:             // окночательно
  case isemptylist:          // окночательно
  case isexpression: break;  // окночательно
  case isvar:
    swapints(ClVar);
    ret = true;
    break;
  case islist:  // if (numb==0)//!!! было (numb<2)
  {
    ClVar->numb = -1;
    ret = stac_un(ClVar);
    if (ret == false)
      break;
    // nextun(ClVar, heap);
  }

  case isunknown:
    nextun(ClVar, heap);
    ret = true;
    break;
  }
  return ret;
}

bool unempty(TClVar *ClVar, array *heap) {
  bool ret = true;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
  //(baserecord *)&heap->heaps[ClVar->term2];
  switch (tp->ident) {
  case isfunction:
  case isinteger:
  case isfloat:
  case isstring:
  case issymbol:
  case islist:
  case isexpression: ret = false; break;
  case isvar: swapints(ClVar); break;
  case isemptylist:
  case isunknown: nextun(ClVar, heap); break;
  }
  return ret;
}

bool unexpr(TClVar *ClVar, array *heap) {
  bool ret = false;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
  //(baserecord *)&heap->heaps[ClVar->term2];
  switch (tp->ident) {
  case isfunction:
  case isstring:
  case issymbol:
  case islist:
  case isemptylist: break;
  case isinteger:
  case isfloat:
  case isvar:
    swapints(ClVar);
    ret = true;
    break;
  case isunknown:
    nextun(ClVar, heap);
    ret = true;
    break;
  case isexpression:
    long c, d;
    float cf, df;
    unsigned char ind1 = calculation(ClVar->term1, ClVar->frame1, &c, &cf, ClVar, heap), ind2 = calculation(ClVar->term2, ClVar->frame2, &d, &df, ClVar, heap);
    if (ind1 == ind2 && ind1 == isinteger)  //!!! сделано только для целых
      if (c == d) {
        nextun(ClVar, heap);
        ret = true;
      } else
        ;  // ret=false; уже присвоено
    else
      ;  // ret =false;//!!! нужно сообщение об ошибке периода исполнения ret=false уже
  }
  return ret;
}

bool unify(unsigned tt1, unsigned tt2, unsigned ff1, unsigned ff2, TClVar *ClVar, array *heap) {
  bool ret = true;
  ClVar->bp = NULL;
  ClVar->ex = true;
  ClVar->term1 = tt1;
  ClVar->term2 = tt2;
  ClVar->fr1 = ff1;
  ClVar->fr2 = ff2;
  while (ClVar->ex && ret) {  //!!!проверрить на прерывание программы
                              //!!! на этом месте стоит проверка на функция и hpcall если вып встр "ВЫП"
                              //=========
    baserecord *tp = heap->GetPbaserecord(ClVar->term2);
    //(baserecord *)&heap->heaps[ClVar->term2];
    if (tp->ident == isfunction) {
      recordfunction *pf = (recordfunction *)tp;
      if (pf->func == hpcall) {
        unsigned *pa = heap->GetPunsigned(pf->ptrarg);
        //(unsigned*)&heap->heaps[pf->ptrarg];
        ClVar->term2 = pa[0];
      }
    }
    //========     //========
    tp = heap->GetPbaserecord(ClVar->term1);
    //(baserecord *)&heap->heaps[ClVar->term1];
    switch (tp->ident) {
    case isfunction: ret = unfun(ClVar, heap); break;
    case isstring:
    case issymbol: ret = unsymb(ClVar, heap); break;
    case isvar: ret = unvar(ClVar, heap); break;
    case isinteger: ret = unint(ClVar, heap); break;
    case isfloat: ret = unfloat(ClVar, heap); break;
    case islist: ret = unlist(ClVar, heap); break;
    case isunknown:
      nextun(ClVar, heap);
      ret = true;
      break;
    case isemptylist: ret = unempty(ClVar, heap); break;
    case isexpression: ret = unexpr(ClVar, heap); break;  // ok
    default: return false;
    }
  }
  return ret;
}
