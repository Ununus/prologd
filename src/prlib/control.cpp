#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "pdefs.h"
#include "pstructs.h"
#include "control.h"
#include "extfunc.h"
#include "functions.h"
// #include <charconv>

void outerror(ErrorCode err) {
  errout(GetPrErrText(err).c_str());
}

PredicateState menegerbp(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (name == ClVar->head || name == hpcut) {
    return argnull(name, ScVar, ClVar, heap);
  }
  if (name == hpout) {
    return prout(ScVar, ClVar, heap);
  }
  recordfunction *pfunc = heap->GetPrecordfunction(ClVar->head);
  if (pfunc->ident == isfunction) {
    if (bpred(name, pfunc->narg) == false) {
      outerror(ErrorCode::WrongNumberOfArgumentsInBuiltinPredicate);  // 26
      return PredicateState::Error;                                   // 1;
    }
  }
  switch (pfunc->narg) {
  case 1: return argone(name, ScVar, ClVar, heap);
  case 2: return argtwo(name, ScVar, ClVar, heap);
  case 3: return argthree(name, ScVar, ClVar, heap);
  case 4: return argfour(name, ScVar, ClVar, heap);
  case 5: return argfive(name, ScVar, ClVar, heap);
  }
  return PredicateState::Error;  // 1;
}

int expand_stack(TClVar *ClVar) {
  ClVar->vmaxstack *= 2;
  ClVar->vst_con.resize(ClVar->vmaxstack);
  ClVar->vst_vr1.resize(ClVar->vmaxstack);
  ClVar->vst_vr2.resize(ClVar->vmaxstack);
  ClVar->vst_trail.resize(ClVar->vmaxstack);
  return 0;
}

// выборка из стека управления
void from_control(TClVar *ClVar, array *heap) {
  ClVar->newclause = ClVar->vst_con[--ClVar->scptr];
  ClVar->atomp = ClVar->vst_con[--ClVar->scptr];
  ClVar->aclause = ClVar->vst_con[--ClVar->scptr];
  ClVar->parent = ClVar->vst_con[--ClVar->scptr];
  ClVar->oldtptr = ClVar->vst_con[--ClVar->scptr];
  ClVar->oldsvptr = ClVar->vst_con[--ClVar->scptr];
  ClVar->frame2 = ClVar->vst_con[--ClVar->scptr];
  heap->ipaclause = ClVar->aclause;
  heap->ipacltarget = heap->GetPrecordclause(heap->ipaclause)->ptrtarget;
  if (ClVar->newclause && ClVar->newclause != isnil) {
    heap->ipnclause = ClVar->newclause;
    heap->ipncltarget = heap->GetPrecordclause(heap->ipnclause)->ptrtarget;
  }
}

// запись в стек
void to_control(TClVar *ClVar, array *heap) {
  if (ClVar->scptr + 7 > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
    outerror(ErrorCode::StackOverflow);  // 28
    ClVar->stat = PredicateState::No;    // 5; /*message_no_memory|=no_memory_stac;*/ // переполнение стека программы
  } else {
    ClVar->vst_con[ClVar->scptr++] = ClVar->frame2;
    ClVar->vst_con[ClVar->scptr++] = ClVar->oldsvptr;
    ClVar->vst_con[ClVar->scptr++] = ClVar->oldtptr;
    ClVar->vst_con[ClVar->scptr++] = ClVar->parent;
    ClVar->vst_con[ClVar->scptr++] = ClVar->aclause;
    ClVar->vst_con[ClVar->scptr++] = ClVar->atomp;
    ClVar->vst_con[ClVar->scptr++] = ClVar->newclause;
  }
}
//==========================вывод===============
size_t term;
size_t frame;
size_t arg;
bool flaglist;

// переход по cвязи переменной на один шаг
void bound(size_t *t, size_t *f, size_t *j, TClVar *ClVar, array *heap) {
  recordvar *prv = heap->GetPrecordvar(*t);
  *j = *f + prv->num;       // смещение в стеке
  *f = ClVar->vst_vr2[*j];  // новый кадр
  *t = ClVar->vst_vr1[*j];  // новый терм
}

size_t occur_term(size_t *TERM, size_t *FRAME, TClVar *ClVar, array *heap) {
  int argt = -1;
  size_t j;
  IntegerType i;  // для expression пока нет
  FloatType f;    // тоже
  do {
    baserecord *tp = heap->GetPbaserecord(*TERM);
    switch (tp->ident) {
    case isstring: argt = 9; break;
    case isfunction: argt = 8; break;
    case isinteger: argt = 7; break;
    case isfloat: argt = 6; break;
    case isvar: {
      bound(TERM, FRAME, &j, ClVar, heap);
      if (*TERM == isnil) {
        argt = 5;
      }
    } break;
    case issymbol: argt = 4; break;
    case islist: argt = 3; break;
    case isemptylist: argt = 2; break;
    case isunknown: argt = 1; break;
    case isexpression: {
      auto ind = calculation(*TERM, *FRAME, &i, &f, ClVar, heap);
      switch (ind) {
      case isinteger: {
        *TERM = heap->append(recordinteger(i));
      } break;
      case isfloat: {
        *TERM = heap->append(recordfloat(f));
      } break;
      default: argt = 0;
      }
    } break;
    }
  } while (argt < 0);
  return argt;
}

bool stac(TClVar *ClVar, array *heap) {
  // if (ClVar->bpt + 4 > &ClVar->BPT[_maxbptr_ - 1]) {
  //   return false;
  // }
  if (ClVar->ibpt + 4 >= ClVar->vBPT.size()) {
    ClVar->vBPT.resize(ClVar->vBPT.size() * 2 + 4);
  }
  ClVar->vBPT[ClVar->ibpt++] = term;
  ClVar->vBPT[ClVar->ibpt++] = frame;
  ClVar->vBPT[ClVar->ibpt++] = arg;
  ClVar->vBPT[ClVar->ibpt++] = flaglist;
  baserecord *tp = heap->GetPbaserecord(term);
  switch (tp->ident) {
  case islist: {
    recordlist *pl = (recordlist *)tp;
    term = pl->head;
  } break;
  case isfunction: {
    recordfunction *pf = (recordfunction *)tp;
    auto *ptrarg = heap->GetPunsigned(pf->ptrarg);
    term = ptrarg[arg - 2];
  } break;
  }
  return true;
}

// j для печати разделителя
bool nextarg(size_t *j, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  bool nxaex;
  do {
    nxaex = true;
    // if (ClVar->bpt == ClVar->BPT) {
    if (ClVar->ibpt == 0) {
      return false;
    }
    flaglist = (bool)ClVar->vBPT[--ClVar->ibpt];
    arg = ClVar->vBPT[--ClVar->ibpt] + 1;
    frame = ClVar->vBPT[--ClVar->ibpt];
    term = ClVar->vBPT[--ClVar->ibpt];
    baserecord *tp = heap->GetPbaserecord(term);
    switch (tp->ident) {
    case isfunction: {
      recordfunction *pf = (recordfunction *)tp;
      if (arg > pf->narg + 1) {
        outBuff.push_back(')');
        (*j)++;
        nxaex = false;
      } else {
        outBuff.push_back(',');
        (*j)++;
        nxaex = stac(ClVar, heap);
      }
    } break;
    case islist:
      recordlist *pl = (recordlist *)tp;
      {
        if (arg == 2) {
          outBuff.push_back(']');
          (*j)++;
          nxaex = false;
        } else {
          auto trm = term;
          trm = pl->link;
          auto ff = frame;
          auto a = occur_term(&trm, &ff, ClVar, heap);
          switch (a) {
          case 3:
            outBuff.push_back(',');
            (*j)++;
            break;
          case 2:
            outBuff.push_back(']');
            (*j)++;
            break;
          default:
            outBuff.push_back('|');
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

// прологовский вывод
PredicateState prout(TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  size_t k = 0;
  recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
  auto *ptr = heap->GetPunsigned(pf->ptrarg);
  size_t count = pf->narg;
  for (size_t i = 0; i < count; i++) {
    pf = heap->GetPrecordfunction(ClVar->head);
    ptr = heap->GetPunsigned(pf->ptrarg);
    k = write_term(ptr[i], ClVar->frame2, i, k, ScVar, ClVar, heap);
  }
  if (ClVar->PrSetting->out.is_open()) {
    ClVar->PrSetting->out << outBuff;
  } else {
    usrout(outBuff.c_str());
  }
  outBuff.clear();
  return PredicateState::Yes;  // 3;
}

size_t write_term(size_t TERM, size_t FRAME, size_t W, size_t j, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  size_t i = 0;
  term = TERM;
  frame = FRAME;
  bool ex = true;
  flaglist = false;
  arg = 0;
  while (ex) {  // здесь циклический вывод терма
    if (term == isnil || term == NULL) {
      outBuff.push_back('_');
      ++j;
      std::string str = std::to_string(W);
      outBuff += str;
      j += str.size();
      ex = nextarg(&j, ScVar, ClVar, heap);
    } else {
      baserecord *tp = heap->GetPbaserecord(term);
      switch (tp->ident) {
      case isfunction: {
        ClVar->precordfunction = (recordfunction *)tp;
        ClVar->precordsconst = heap->GetPrecordsconst(ClVar->precordfunction->func);
        char *name = heap->GetPchar(ClVar->precordsconst->ptrsymb);
        for (i = 0; i < ClVar->precordsconst->length; i++) {
          outBuff.push_back(*(name + i));
          ++j;
        }
        outBuff.push_back('(');
        ++j;
        arg = 2;
        flaglist = false;
        ex = stac(ClVar, heap);
        i = 0;
      } break;  // ok
      case isinteger: {
        ClVar->precordinteger = (recordinteger *)tp;
        std::string str = ClVar->precordinteger->value.str();
        outBuff += str;
        j += str.size();
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;  // ok
      case isfloat: {
        ClVar->precordfloat = (recordfloat *)tp;
        std::string str = toString(ClVar->precordfloat->value);
        outBuff += str;
        j += str.size();
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
        for (i = 0; i < ClVar->precordsconst->length; i++) {
          outBuff.push_back(*(name + i));
        }
        j += i;
        i = 0;
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;  // ok
      case islist: {
        if (!(arg && flaglist))  // список встретился
        {
          outBuff.push_back('[');
          ++j;
          flaglist = true;
        }
        arg = 0;
        ex = stac(ClVar, heap);
      } break;
      case isemptylist: {
        if (arg != 1) {
          outBuff += "[]";
          j += 2;
        }
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;
      case iscut: {
        outBuff.push_back('!');
        ++j;
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;
      case isunknown:
        outBuff.push_back('_');
        ++j;
        ex = nextarg(&j, ScVar, ClVar, heap);
        break;
      case isexpression:
        IntegerType ii;
        FloatType ff;
        auto ind = calculation(term, frame, &ii, &ff, ClVar, heap);
        switch (ind) {
        case isinteger: {
          std::string str = ii.str();
          outBuff += str;
          j += str.size();
        } break;
        case isfloat: {
          std::string str = toString(ff);
          outBuff += str;
          j += str.size();
          i = 0;
          break;
        }
        default:
          char *s = const_cast<char *>("#_?_#");
          for (i = 0; i < 5 /* && i + j < sizeof(buff) - 1 */; i++) {
            outBuff.push_back(s[i]);
          }
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
  auto &outBuff = ClVar->outBuff;
  if (!ClVar->varqu) {
    if (!ClVar->quiet) {
      prdout(true);
    }
    ClVar->flag = false;
    ClVar->stat = PredicateState::Error;  // 1;
  } else {
    ClVar->flag = false;
    // bpt=buf;//указатель на внешний buf
    //   bpt=BPT=new unsigned[maxbptr];
    //   unsigned i=0,j=0;
    size_t j = 0, w;
    if (!ClVar->quiet) {
      for (size_t k = 0; k < ClVar->varqu; k++) {
        w = k;
        ClVar->precordvar = heap->GetPrecordvar(ScVar->vtvar[k]);
        char *name = heap->GetPchar(ClVar->precordvar->ptrsymb);
        for (j = 0; j < ClVar->precordvar->length; j++) {
          outBuff.push_back(*(name + j));
        }
        // buff[j++] = '=';
        outBuff.push_back('=');
        frame = ClVar->vst_vr2[k];
        term = ClVar->vst_vr1[k];
        //======================================// вывод terma  пока не все
        // buff[write_term(term, frame, w, j, ScVar, ClVar, heap)] = 0;  //,//FILE *d)
        write_term(term, frame, w, j, ScVar, ClVar, heap);
        pldout(outBuff.c_str());
        outBuff.clear();
      }
    }  // delete BPT;
  }
}

// конец вывода
void zero(TClVar *ClVar) {
  for (size_t i = ClVar->oldtptr; i < ClVar->tptr; i++) {
    ClVar->vst_vr1[ClVar->vst_trail[i]] = isnil;
  }
  ClVar->tptr = ClVar->oldtptr;
}

// подготовка к выполнению целей
static void st_2(TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (ClVar->ntro) {
    ClVar->parent = ClVar->scptr;  // место записи в стеке управления о вызывающем предл
  }
  ClVar->frame2 = ClVar->frame1;
}

// Выделяется цель, которая должна быть удовлетворена. Если все цели
// удовлетворены, то из стека восстанавливается родительская среда.
// Если родительской среды нет, то выводется ответная подстановка.
static void st_3(TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!heap->GetPunsigned(heap->ipacltarget)[++ClVar->atomp])  // все цели удовлетворены
    if (ClVar->parent == NULL)                                 // есть ответ на вопрос
    {
      prvars(ScVar, ClVar, heap);  // печать переменных
      if (ClVar->stat != PredicateState::Error) {
        ClVar->stat = PredicateState::No;  // 5;  // неудача - не найдено решение
      }
    } else {
      size_t work = ClVar->scptr;
      ClVar->scptr = ClVar->parent;  // зачем ?
      from_control(ClVar, heap);     // выборка родительской среды
      ClVar->scptr = work;
      ClVar->stat = PredicateState::Yes;  // 3;
    }
  else {
    ClVar->head = heap->GetPunsigned(heap->ipacltarget)[ClVar->atomp];  // индекс новой цели
    ClVar->stat = PredicateState::Builtin;                              // 6;
  }
}

// Основной шаг выполнения. Определяются, есть ли предложения,
// которые не были испытаны для данной цели.
// Если таких предикатов нет, то цель считается неудачной (Состояние 5).
// Если предложения есть, то происходит настройка на голову проедложения, в стеке
// переменных выделяется новый кадр. В зависимости от результата унификации либо
// происходит переход к целям нового предложения, либо испытывается следующее предложение.
static void st_4(TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  if (ClVar->newclause && ClVar->newclause != isnil) {  // pnclause=head->at(newclause);pncltarget=head->at(newclause-1);
    heap->iptclause = heap->ipnclause;
    heap->iptcltarget = heap->ipncltarget;
    ClVar->tclause = ClVar->newclause;
    ClVar->tryclause = 0;  // ptclause->ptrtarget[tryclause] ук на голову
    // tryclause=ptclause->head; //ук на голову
    ClVar->frame1 = ClVar->oldsvptr = ClVar->svptr;
    ClVar->svptr = ClVar->frame1 + heap->GetPrecordclause(heap->ipnclause)->nvars;
#ifdef PrintVarCount
    char _buf[128];
    sprintf(_buf, "nvars:%d", ClVar->pnclause->nvars);
    out(_buf);
#endif
    ClVar->newclause = heap->GetPrecordclause(heap->ipnclause)->next;
    if (ClVar->newclause) {
      heap->ipnclause = ClVar->newclause;
      heap->ipncltarget = heap->GetPrecordclause(heap->ipnclause)->ptrtarget;
    }
    if (ClVar->svptr > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
      ClVar->err = ErrorCode::StackOverflow;  // 28;
      ClVar->stat = PredicateState::Error;    // 1;
    } else {
      if (ClVar->PrSetting->Trace) {
        pldout("Согласуется с ");
        // buff[write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame1, 0, 0, ScVar, ClVar, heap)] = 0;
        write_term(heap->GetPunsigned(heap->iptcltarget)[ClVar->tryclause], ClVar->frame1, 0, 0, ScVar, ClVar, heap);
        pldout(outBuff.c_str());
        outBuff.clear();
      }
      ClVar->oldtptr = ClVar->tptr;
      if (unify(heap->GetPunsigned(heap->iptcltarget)[ClVar->tryclause], ClVar->head, ClVar->frame1, ClVar->frame2, ClVar, heap)) {  // удачная унификация
        if (ClVar->newclause || heap->GetPunsigned(heap->ipacltarget)[ClVar->atomp + 1]) {
          to_control(ClVar, heap);
          ClVar->ntro = true;
        } else {
          ClVar->ntro = false;
        }
        if (ClVar->stat != PredicateState::No) {
          if (ClVar->PrSetting->Trace) {
            pldout("Успех");
          }
          ClVar->oldtptr = ClVar->tptr;
          ClVar->oldsvptr = ClVar->svptr;
          ClVar->atomp = ClVar->tryclause;
          // atomp=-1;
          ClVar->aclause = ClVar->tclause;
          heap->ipaclause = heap->iptclause;
          heap->ipacltarget = heap->iptcltarget;
          ClVar->stat = PredicateState::PrepereNewTarget;  // 2;
        }
      } else if (ClVar->stat != PredicateState::Error) {
        if (ClVar->PrSetting->Trace) {
          pldout("Неудача");
        }
        ClVar->svptr = ClVar->oldsvptr;
        zero(ClVar);
        ClVar->stat = PredicateState::ControlStep;  // 4;
      }
    }
  } else {
    if (ClVar->PrSetting->Trace) {
      if (heap->iptcltarget) {
        pldout("Цель");
        // buff[write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
        write_term(heap->GetPunsigned(heap->iptcltarget)[ClVar->tryclause], ClVar->frame2, 0, 0, ScVar, ClVar, heap);
        outBuff.clear();
      }
      pldout("Неудачна ");
    }
    ClVar->stat = PredicateState::No;  // 5;
  }
}

// Неудача, для цели не найдено решение. Осуществляется возврат к предыдущей цели,
// чтобы поискать другое решение.
static void st_5(TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  if (!ClVar->scptr) {
    ClVar->stat = PredicateState::Error;  // 1;
  } else {
    from_control(ClVar, heap);
    zero(ClVar);
    ClVar->head = heap->GetPunsigned(heap->ipacltarget)[ClVar->atomp];
    ClVar->svptr = ClVar->oldsvptr;
    if (ClVar->PrSetting->Trace) {
      pldout("Переделка. Цель ");
      // buff[write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
      write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap);
      pldout(outBuff.c_str());
      outBuff.clear();
    }
    ClVar->stat = PredicateState::ControlStep;  // 4;
  }
}

// Выесняется, является ли исполняемая цель встроенным предикатом.
static void st_6(TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  if (ClVar->PrSetting->Trace) {
    pldout("Цель ");
    // buff[write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
    write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap);
    pldout(outBuff.c_str());
    outBuff.clear();
  }
  ClVar->precordfunction = heap->GetPrecordfunction(ClVar->head);
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
    ClVar->newclause = ClVar->precordsconst->begin;
    if (ClVar->newclause && ClVar->newclause != isnil) {
      heap->ipnclause = ClVar->newclause;
      heap->ipncltarget = heap->GetPrecordclause(heap->ipnclause)->ptrtarget;
    }
    ClVar->stat = PredicateState::ControlStep;  // 4;
  }
}

// Очистить состояние
static void OnControl(TClVar *ClVar, array *heap) {
  // add 2008/10/04
  heap->iptclause = 0;
  heap->iptcltarget = 0;

  ClVar->err = ErrorCode::NoErrors;  // 0
  for (size_t i = 0; i < ClVar->vmaxstack; i++) {
    ClVar->vst_con[i] = isnil;
    ClVar->vst_vr1[i] = isnil;
    ClVar->vst_vr2[i] = isnil;
    ClVar->vst_trail[i] = isnil;
  }
  ClVar->vbf.clear();

  ClVar->stat = PredicateState::PrepereNewTarget;  // 2;
  ClVar->newclause = 0;
  ClVar->ntro = false;  // нет оптимизации хвостовой рекурсии не знаю пока что это
  ClVar->flag = true;
  ClVar->oldtptr = ClVar->tptr = 0;  // индекс стека следа
  ClVar->scptr = 0;
  ClVar->frame1 = 0;
  ClVar->parent = 0;
  // aclause = heap->last - sizeof(recordclause);                          //чего это ?
  ClVar->aclause = heap->query;
  heap->ipaclause = ClVar->aclause;
  heap->ipacltarget = heap->GetPrecordclause(heap->ipaclause)->ptrtarget;
  ClVar->varqu = heap->GetPrecordclause(heap->ipaclause)->nvars;
  ClVar->svptr = ClVar->varqu;
  ClVar->atomp = -1;
  ClVar->quiet = false;
}

ErrorCode control(TScVar *ScVar, TClVar *ClVar, array *heap, bool *EnableRunning) {
  OnControl(ClVar, heap);
  while (ClVar->stat != PredicateState::Error && ClVar->err == ErrorCode::NoErrors) {
    switch (ClVar->stat) {
    case PredicateState::PrepereNewTarget:  // 2
      st_2(ScVar, ClVar, heap);
      [[fallthrough]];         // если stat = 2 то выполнение сразу же и 3
    case PredicateState::Yes:  // 3
      st_3(ScVar, ClVar, heap);
      break;
    case PredicateState::ControlStep:  // 4
      st_4(ScVar, ClVar, heap);
      break;
    case PredicateState::No:  // 5
      st_5(ScVar, ClVar, heap);
      break;
    case PredicateState::Builtin:  // 6
      st_6(ScVar, ClVar, heap);
      break;
    default: break;
    }
    if (!*EnableRunning) {
      ClVar->err = ErrorCode::ExecutionInterrupted;  // 27;  // abort
    }
  }
  if (ClVar->flag && !ClVar->quiet) {
    prdout(false);
  }
  return ClVar->err;
}

// ================ унификация =======================
// возврат 0 ошибка,isinteger-результат целое,isfloat-результат вешещественное
size_t calculation(size_t term, size_t frame, IntegerType *i, FloatType *f, TClVar *ClVar, array *heap) {
  size_t ind = 0;  // первоначально попытка вычислить выражение с целымы
  std::vector<baserecord *> sta;
  ClVar->precordexpression = heap->GetPrecordexpression(term);
  size_t n = ClVar->precordexpression->length;
  ErrorCode err = ErrorCode::NoErrors;
  baserecord *tp;
  IntegerType oi1 = 0, oi2 = 0;                                   // операнды для вычисления целых
  FloatType of1 = 0, of2 = 0;                                     // операнды для  вычисления вещественных
  for (size_t j = 0; j < n && err == ErrorCode::NoErrors; ++j) {  // if (lowMemory()) return 0;//!!!сообщение о r_t_e
    auto *ptr = heap->GetPunsigned(ClVar->precordexpression->precord);
    if (ptr[j] < isbase)  // операнд
    {
      tp = heap->GetPbaserecord(ptr[j]);
      switch (tp->ident) {
      case isinteger: {
        ClVar->precordinteger = (recordinteger *)tp;
        sta.push_back(new recordinteger(ClVar->precordinteger->value));
        oi2 = ClVar->precordinteger->value;
        ind = isinteger;
      } break;
      case isfloat: {
        ClVar->precordfloat = (recordfloat *)tp;
        sta.push_back(new recordfloat(ClVar->precordfloat->value));
        of2 = ClVar->precordfloat->value;
        ind = isfloat;
      } break;
      case isvar: {
        auto tr = ptr[j];
        auto fr = frame;
        auto a = occur_term(&tr, &fr, ClVar, heap);
        switch (a) {
        case 6: {
          ClVar->precordfloat = heap->GetPrecordfloat(tr);
          sta.push_back(new recordfloat(ClVar->precordfloat->value));
          of2 = ClVar->precordfloat->value;
          ind = isfloat;
        } break;
        case 7: {
          ClVar->precordinteger = heap->GetPrecordinteger(tr);
          sta.push_back(new recordinteger(ClVar->precordinteger->value));
          oi2 = ClVar->precordinteger->value;
          ind = isinteger;
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
    } else {  // знак ариафм операции
      for (int k = 0; k < 2; k++) {
        tp = sta.back();
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
            of2 = (FloatType)ClVar->precordinteger->value;  // если первый вещественный
          }
          delete sta.back();
          sta.pop_back();
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
              of1 = (FloatType)oi1;
            }
          }
          delete sta.back();
          sta.pop_back();
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
            err = ErrorCode::DivisionByZeroWhileCalculatingArithmeticsExpressions;  // 41
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
      if (ind == isinteger) {
        sta.push_back(new recordinteger(oi2));
      } else {
        sta.push_back(new recordfloat(of2));
      }
    }
  }
  *i = oi2;
  *f = of2;
  if (err != ErrorCode::NoErrors) {
    outerror(err);
  }
  if (sta.size() != 1) {
    ind = 0;
  }
  while (!sta.empty()) {
    delete sta.back();
    sta.pop_back();
  }
  return ind;
}

bool stac_un(TClVar *ClVar) {
  while (ClVar->bp + 5 > ClVar->vbf.size()) {
    ClVar->vbf.resize(ClVar->vbf.size() * 2 + 5);
  }
  ClVar->vbf[ClVar->bp++] = ClVar->numb;
  ClVar->vbf[ClVar->bp++] = ClVar->fr2;
  ClVar->vbf[ClVar->bp++] = ClVar->fr1;
  ClVar->vbf[ClVar->bp++] = ClVar->term2;
  ClVar->vbf[ClVar->bp++] = ClVar->term1;
  return true;
}

void nextun(TClVar *ClVar, array *heap) {
  if (!ClVar->bp) {
    ClVar->ex = false;
    return;
  }
  ClVar->term1 = ClVar->vbf[--ClVar->bp];
  ClVar->term2 = ClVar->vbf[--ClVar->bp];
  ClVar->fr1 = ClVar->vbf[--ClVar->bp];
  ClVar->fr2 = ClVar->vbf[--ClVar->bp];
  ClVar->numb = ++ClVar->vbf[--ClVar->bp];

  baserecord *tp = heap->GetPbaserecord(ClVar->term1);
  if (tp->ident == isfunction) {
    ClVar->precordfunction = (recordfunction *)tp;
    if (ClVar->precordfunction->narg > ClVar->numb) {
      auto *ptr = heap->GetPunsigned(ClVar->precordfunction->ptrarg);
      ClVar->term1 = ptr[ClVar->numb];
      ClVar->precordfunction = heap->GetPrecordfunction(ClVar->term2);
      ptr = heap->GetPunsigned(ClVar->precordfunction->ptrarg);
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
        ClVar->term2 = ClVar->precordlist->head;
        ClVar->bp += 5;
        break;
      case 1:
        recordlist *precordlist2 = heap->GetPrecordlist(ClVar->term2);
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
  auto c = ClVar->term1;
  ClVar->term1 = ClVar->term2;
  ClVar->term2 = c;
  c = ClVar->fr1;
  ClVar->fr1 = ClVar->fr2;
  ClVar->fr2 = c;
}

bool unfun(TClVar *ClVar, array *heap) {
  bool ret = false;
  baserecord *tp = heap->GetPbaserecord(ClVar->term2);
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
    recordfunction *pf1 = heap->GetPrecordfunction(ClVar->term1);
    recordfunction *pf2 = (recordfunction *)tp;
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
  switch (tp->ident)  // указатель на след униф терм
  {
  case isstring:
  case issymbol: {
    recordstring *pstr1 = heap->GetPrecordstring(ClVar->term1);
    recordstring *pstr2 = (recordstring *)tp;
    char *name1 = heap->GetPchar(pstr1->ptrsymb);
    char *name2 = heap->GetPchar(pstr2->ptrsymb);
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
void ocrn(size_t bd1, size_t tr2, TClVar *ClVar) {
  ClVar->vst_vr1[bd1] = tr2;  // и отметка в стеке следа
  ClVar->vst_trail[ClVar->tptr++] = bd1;
}

bool unvar(TClVar *ClVar, array *heap) {
  bool ret = true;
  size_t bd1, bd2, work;
  recordvar *precordvar = heap->GetPrecordvar(ClVar->term1);
  bd1 = precordvar->num + ClVar->fr1;
  work = ClVar->vst_vr1[bd1];
  if (work != isnil)  // связанная переменная
  {
    ClVar->term1 = work;
    work = ClVar->vst_vr2[bd1];
    if (work != isnil)
      ClVar->fr1 = work;
    //   else {ocrn(bd1,term2);nextun();}//!!!добавлено мной
  } else {
    baserecord *tp = heap->GetPbaserecord(ClVar->term2);
    switch (tp->ident) {
    case isfunction:
    case islist:
      ClVar->vst_vr2[bd1] = ClVar->fr2;  // break; // пока убрал break
    //[[fallthrough]]
    case isfloat:    // также как issumbol;
    case isinteger:  // так же как issymbol
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
        bd2 = precordvar->num + ClVar->fr2;
        work = ClVar->vst_vr1[bd2];
        if (work == isnil) {  // свободная переменная
          if (ClVar->term1 == ClVar->term2) {
            nextun(ClVar, heap);
          } else {
            ClVar->vst_vr2[bd1] = ClVar->fr2;
            ocrn(bd1, ClVar->term2, ClVar);
            nextun(ClVar, heap);
          }
        } else {  // связанная переменная
          ClVar->term2 = work;
          work = ClVar->vst_vr2[bd2];
          if (work != isnil)
            ClVar->fr2 = work;
        }
      }
      break;
    case isunknown: nextun(ClVar, heap); break;
    case isexpression:
      IntegerType a;
      FloatType af;
      auto ind = calculation(ClVar->term2, ClVar->frame2, &a, &af, ClVar, heap);
      switch (ind) {
      case 0: ret = false; break;  //!!!abort_calculation;
      case isinteger: {
        ClVar->term2 = heap->append(recordinteger(a));
      } break;
      case isfloat: {
        ClVar->term2 = heap->append(recordfloat(af));
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
  switch (tp->ident) {
  case isfunction:
  case isstring:
  case issymbol:
  case islist:
  case isemptylist: break;  // ээто окончательно
  case isinteger: {
    recordinteger *precordinteger = (recordinteger *)tp;
    recordinteger *pint = heap->GetPrecordinteger(ClVar->term1);
    if (precordinteger->value == pint->value) {
      ret = true;
      nextun(ClVar, heap);
    }
  } break;
  case isfloat: {
    recordfloat *precordfloat = (recordfloat *)tp;
    recordinteger *precordinteger = heap->GetPrecordinteger(ClVar->term1);
    if (precordfloat->value - 1 < precordinteger->value.convert_to<FloatType>() && precordfloat->value == (float)precordinteger->value) {
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
    IntegerType a;
    FloatType af;
    auto ind = calculation(ClVar->term2, ClVar->frame2, &a, &af, ClVar, heap);
    recordinteger *pint = heap->GetPrecordinteger(ClVar->term1);
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
  switch (tp->ident) {
  case isfunction:
  case isstring:
  case issymbol:
  case islist:
  case isemptylist: break;  // ээто окончательно
  case isinteger: {
    recordfloat *precordfloat = heap->GetPrecordfloat(ClVar->term1);
    recordinteger *precordinteger = (recordinteger *)tp;
    if (precordfloat->value - 1 < precordinteger->value.convert_to<FloatType>() && (float)precordinteger->value == precordfloat->value) {
      ret = true;
      nextun(ClVar, heap);
    }
  } break;
  case isfloat: {
    recordfloat *precordfloat = (recordfloat *)tp;
    recordfloat *pfl = heap->GetPrecordfloat(ClVar->term1);
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
    if (ret == false) {
      break;
    }
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
    IntegerType c, d;
    FloatType cf, df;
    auto ind1 = calculation(ClVar->term1, ClVar->frame1, &c, &cf, ClVar, heap), ind2 = calculation(ClVar->term2, ClVar->frame2, &d, &df, ClVar, heap);
    if (ind1 == ind2 && ind1 == isinteger) {
      //!!! сделано только для целых
      if (c == d) {
        nextun(ClVar, heap);
        ret = true;
      } else {
        ;  // ret=false; уже присвоено
      }
    } else {
      ;  // ret =false;//!!! нужно сообщение об ошибке периода исполнения ret=false уже
    }
  }
  return ret;
}

bool unify(size_t tt1, size_t tt2, size_t ff1, size_t ff2, TClVar *ClVar, array *heap) {
  bool ret = true;
  ClVar->bp = 0;
  ClVar->ex = true;
  ClVar->term1 = tt1;
  ClVar->term2 = tt2;
  ClVar->fr1 = ff1;
  ClVar->fr2 = ff2;
  while (ClVar->ex && ret) {  //!!!проверрить на прерывание программы
                              //!!! на этом месте стоит проверка на функция и hpcall если вып встр "ВЫП"
                              //=========
    baserecord *tp = heap->GetPbaserecord(ClVar->term2);
    if (tp->ident == isfunction) {
      recordfunction *pf = (recordfunction *)tp;
      if (pf->func == hpcall) {
        auto *pa = heap->GetPunsigned(pf->ptrarg);
        ClVar->term2 = pa[0];
      }
    }
    //========     //========
    tp = heap->GetPbaserecord(ClVar->term1);
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
