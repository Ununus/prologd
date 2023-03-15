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
  errout(GetPrErrText(err));
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

void to_stac(size_t *b, size_t index, size_t value) {
  *(b + index) = value;
}

size_t from_stac(size_t *b, size_t index) {
  return *(b + index);
}

int expand_stack(TClVar *ClVar) {
  size_t vmaxstacknew = (ClVar->vmaxstack * 2);
  size_t *st_conN;
  size_t *st_vr1N;
  size_t *st_vr2N;
  size_t *st_trailN;

  st_conN = new size_t[vmaxstacknew];
  st_vr1N = new size_t[vmaxstacknew];
  st_vr2N = new size_t[vmaxstacknew];
  st_trailN = new size_t[vmaxstacknew];

  size_t i;
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

// ������� �� ����� ����������
void from_control(TClVar *ClVar, array *heap) {
  ClVar->newclause = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->atomp = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->aclause = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->parent = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->oldtptr = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->oldsvptr = from_stac(ClVar->st_con, --ClVar->scptr);
  ClVar->frame2 = from_stac(ClVar->st_con, --ClVar->scptr);
  heap->paclause = heap->GetPrecordclause(ClVar->aclause);
  heap->pacltarget = heap->GetPunsigned(heap->paclause->ptrtarget);
  if (ClVar->newclause && ClVar->newclause != isnil) {
    heap->pnclause = heap->GetPrecordclause(ClVar->newclause);
    heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
  }
}

// ������ � ����
void to_control(TClVar *ClVar, array *heap) {
  if (ClVar->scptr + 7 > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
    outerror(ErrorCode::StackOverflow);  // 28
    ClVar->stat = PredicateState::No;    // 5; /*message_no_memory|=no_memory_stac;*/ // ������������ ����� ���������
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
//==========================�����===============
size_t term;
size_t frame;
size_t arg;
bool flaglist;

// ������� �� c���� ���������� �� ���� ���
void bound(size_t *t, size_t *f, size_t *j, TClVar *ClVar, array *heap) {
  recordvar *prv = heap->GetPrecordvar(*t);
  *j = *f + prv->num;                 // �������� � �����
  *f = from_stac(ClVar->st_vr2, *j);  // ����� ����
  *t = from_stac(ClVar->st_vr1, *j);  // ����� ����
}

size_t occur_term(size_t *TERM, size_t *FRAME, TClVar *ClVar, array *heap) {
  int argt = -1;
  size_t j;
  IntegerType i;  // ��� expression ���� ���
  FloatType f;    // ����
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
  if (ClVar->bpt + 4 > &ClVar->BPT[_maxbptr_ - 1]) {
    return false;
  }
  *(ClVar->bpt++) = term;
  *(ClVar->bpt++) = frame;
  *(ClVar->bpt++) = arg;
  *(ClVar->bpt++) = flaglist;
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

// j ��� ������ �����������
bool nextarg(size_t *j, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
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
    switch (tp->ident) {
    case isfunction: {
      recordfunction *pf = (recordfunction *)tp;
      if (arg > pf->narg + 1) {
        // buff[*j] = ')';
        outBuff.push_back(')');
        (*j)++;
        nxaex = false;
      } else {
        // buff[*j] = ',';
        outBuff.push_back(',');
        (*j)++;
        nxaex = stac(ClVar, heap);
      }
    } break;
    case islist:
      recordlist *pl = (recordlist *)tp;
      {
        if (arg == 2) {
          // buff[*j] = ']';
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
            // buff[*j] = ',';
            outBuff.push_back(',');
            (*j)++;
            break;
          case 2:
            // buff[*j] = ']';
            outBuff.push_back(']');
            (*j)++;
            break;
          default:
            // buff[*j] = '|';
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

// ������������ �����
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
  // buff[k] = 0;
  if (ClVar->PrSetting->out.is_open()) {
    // ClVar->PrSetting->out << buff;
    ClVar->PrSetting->out << outBuff.c_str();
  } else {
    // usrout(buff);
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

  // unsigned bpt[maxbptr];
  //  bpt=BPT=new unsigned[maxbptr];
  if (!ClVar->bpt) {
    outerror(ErrorCode::NotEnoughFreeMemory);  // 2
    return j;
  }
  bool ex = true;
  flaglist = false;
  arg = 0;
  while (ex) {                          // ����� ����8������� ����� �����
    if (term == isnil || term == NULL)  // || !term)
    {
      // buff[j++] = '_';
      outBuff.push_back('_');
      ++j;
      // auto [ptr, ec] = std::to_chars(buff + j, buff + sizeof(buff), W);
      // j = ptr - buff;

      // �������� �� to_chars, �� �� ��������
      //_itoa(W, (char*)&buff[j], 10);
      // for (; (j < sizeof(buff) - 1 && buff[j]); j++);

      std::string res = std::to_string(W);
      for (size_t q = 0; q < res.size(); ++q) {
        // buff[j] = res[q];
        outBuff.push_back(res[q]);
        ++j;
      }

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
        // buff[j++] = '(';
        outBuff.push_back('(');
        ++j;
        arg = 2;
        flaglist = false;
        ex = stac(ClVar, heap);
        i = 0;
      } break;  // ok
      case isinteger: {
        ClVar->precordinteger = (recordinteger *)tp;
        // auto [ptr, ec] = std::to_chars(buff + j, buff + sizeof(buff), ClVar->precordinteger->value);
        // j = ptr - buff;

        // �������� �� to_chars, �� �� ��������
        //_ltoa(ClVar->precordinteger->value, (char*)&buff[j], 10);
        // for (; j < sizeof(buff) - 1 && buff[j]; j++);

        // std::string res = std::to_string(ClVar->precordinteger->value);
        std::string res = ClVar->precordinteger->value.str();
        for (size_t q = 0; q < res.size(); ++q) {
          // buff[j] = res[q];
          outBuff.push_back(res[q]);
          ++j;
        }

        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;  // ok
      case isfloat: {
        ClVar->precordfloat = (recordfloat *)tp;
        char string[40];
        sprintf(string, "%g", ClVar->precordfloat->value);
        for (i = 0; /* j < sizeof(buff) - 1 && */ string[i]; ++i) {
          outBuff.push_back(string[i]);
          ++j;
        }
        // buff[j] = 0;
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
        if (!(arg && flaglist))  // ������ ����������
        {
          // buff[j++] = '[';
          outBuff.push_back('[');
          ++j;
          flaglist = true;
        }
        arg = 0;
        ex = stac(ClVar, heap);
      } break;
      case isemptylist: {
        if (arg != 1) {
          // buff[j++] = '[';
          // buff[j++] = ']';
          outBuff += "[]";
          j += 2;
        }
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;
      case iscut: {
        // buff[j++] = '!';
        outBuff.push_back('!');
        ++j;
        ex = nextarg(&j, ScVar, ClVar, heap);
      } break;
      case isunknown:
        // buff[j++] = '_';
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
          // auto [ptr, ec] = std::to_chars(buff + j, buff + sizeof(buff), ii);
          // j = ptr - buff;

          // �������� �� to_chars, �� �� ��������
          //_ltoa(ii, (char*)&buff[j], 10);
          // for (; j < sizeof(buff) - 1 && buff[j]; j++);

          // std::string res = std::to_string(ii);
          std::string res = ii.str();
          for (size_t q = 0; q < res.size(); ++q) {
            // buff[j] = res[q];
            outBuff.push_back(res[q]);
            ++j;
          }

        } break;
        case isfloat:
          char string[40];
          sprintf(string, "%g", ff);
          for (i = 0; /* j < sizeof(buff) - 1 && */ string[i]; ++i) {
            outBuff.push_back(string[i]);
            ++j;
          }
          // buff[j] = 0;
          i = 0;
          break;
        /*                   case isfloat  :    �������� �� �� ���������
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

// ���� ������ �������� �������
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
    // bpt=buf;//��������� �� ������� buf
    //   bpt=BPT=new unsigned[maxbptr];
    //   unsigned i=0,j=0;
    size_t j = 0, w;
    if (!ClVar->quiet) {
      for (size_t k = 0; k < ClVar->varqu; k++) {
        w = k;
        ClVar->precordvar = heap->GetPrecordvar(ScVar->tvar[k]);
        char *name = heap->GetPchar(ClVar->precordvar->ptrsymb);
        for (j = 0; j < ClVar->precordvar->length; j++) {
          outBuff.push_back(*(name + j));
        }
        // buff[j++] = '=';
        outBuff.push_back('=');
        frame = from_stac(ClVar->st_vr2, k);
        term = from_stac(ClVar->st_vr1, k);
        //======================================// ����� terma  ���� �� ���
        // buff[write_term(term, frame, w, j, ScVar, ClVar, heap)] = 0;  //,//FILE *d)
        write_term(term, frame, w, j, ScVar, ClVar, heap);
        pldout(outBuff.c_str());
        outBuff.clear();
      }
    }  // delete BPT;
  }
}

// ����� ������
void zero(TClVar *ClVar) {
  for (size_t i = ClVar->oldtptr; i < ClVar->tptr; i++) {
    to_stac(ClVar->st_vr1, from_stac(ClVar->st_trail, i), isnil);
  }
  ClVar->tptr = ClVar->oldtptr;
}

// ���������� � ���������� �����
static void st_2(TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (ClVar->ntro) {
    ClVar->parent = ClVar->scptr;  // ����� ������ � ����� ���������� � ���������� �����
  }
  ClVar->frame2 = ClVar->frame1;
}

// ���������� ����, ������� ������ ���� �������������. ���� ��� ����
// �������������, �� �� ����� ����������������� ������������ �����.
// ���� ������������ ����� ���, �� ��������� �������� �����������.
static void st_3(TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!heap->pacltarget[++ClVar->atomp])  // ��� ���� �������������
    if (ClVar->parent == NULL)            // ���� ����� �� ������
    {
      prvars(ScVar, ClVar, heap);  // ������ ����������
      if (ClVar->stat != PredicateState::Error) {
        ClVar->stat = PredicateState::No;  // 5;  // ������� - �� ������� �������
      }
    } else {
      size_t work = ClVar->scptr;
      ClVar->scptr = ClVar->parent;  // ����� ?
      from_control(ClVar, heap);     // ������� ������������ �����
      ClVar->scptr = work;
      ClVar->stat = PredicateState::Yes;  // 3;
    }
  else {
    ClVar->head = heap->pacltarget[ClVar->atomp];  // ������ ����� ����
    ClVar->stat = PredicateState::Builtin;         // 6;
  }
}

// �������� ��� ����������. ������������, ���� �� �����������,
// ������� �� ���� �������� ��� ������ ����.
// ���� ����� ���������� ���, �� ���� ��������� ��������� (��������� 5).
// ���� ����������� ����, �� ���������� ��������� �� ������ ������������, � �����
// ���������� ���������� ����� ����. � ����������� �� ���������� ���������� ����
// ���������� ������� � ����� ������ �����������, ���� ������������ ��������� �����������.
static void st_4(TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  if (ClVar->newclause && ClVar->newclause != isnil) {  // pnclause=head->at(newclause);pncltarget=head->at(newclause-1);
    heap->ptclause = heap->pnclause;
    heap->ptcltarget = heap->pncltarget;
    ClVar->tclause = ClVar->newclause;
    ClVar->tryclause = 0;  // ptclause->ptrtarget[tryclause] �� �� ������
    // tryclause=ptclause->head; //�� �� ������
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
      heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
    }
    if (ClVar->svptr > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
      ClVar->err = ErrorCode::StackOverflow;  // 28;
      ClVar->stat = PredicateState::Error;    // 1;
    } else {
      if (ClVar->PrSetting->Trace) {
        pldout("����������� � ");
        // buff[write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame1, 0, 0, ScVar, ClVar, heap)] = 0;
        write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame1, 0, 0, ScVar, ClVar, heap);
        pldout(outBuff.c_str());
        outBuff.clear();
      }
      ClVar->oldtptr = ClVar->tptr;
      if (unify(heap->ptcltarget[ClVar->tryclause], ClVar->head, ClVar->frame1, ClVar->frame2, ClVar, heap)) {  // ������� ����������
        if (ClVar->newclause || heap->pacltarget[ClVar->atomp + 1]) {
          to_control(ClVar, heap);
          ClVar->ntro = true;
        } else {
          ClVar->ntro = false;
        }
        if (ClVar->stat != PredicateState::No) {
          if (ClVar->PrSetting->Trace) {
            pldout("�����");
          }
          ClVar->oldtptr = ClVar->tptr;
          ClVar->oldsvptr = ClVar->svptr;
          ClVar->atomp = ClVar->tryclause;
          // atomp=-1;
          ClVar->aclause = ClVar->tclause;
          heap->paclause = heap->ptclause;
          heap->pacltarget = heap->ptcltarget;
          ClVar->stat = PredicateState::PrepereNewTarget;  // 2;
        }
      } else if (ClVar->stat != PredicateState::Error) {
        if (ClVar->PrSetting->Trace) {
          pldout("�������");
        }
        ClVar->svptr = ClVar->oldsvptr;
        zero(ClVar);
        ClVar->stat = PredicateState::ControlStep;  // 4;
      }
    }
  } else {
    if (ClVar->PrSetting->Trace) {
      if (heap->ptcltarget) {
        pldout("����");
        // buff[write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
        write_term(heap->ptcltarget[ClVar->tryclause], ClVar->frame2, 0, 0, ScVar, ClVar, heap);
        outBuff.clear();
      }
      pldout("�������� ");
    }
    ClVar->stat = PredicateState::No;  // 5;
  }
}

// �������, ��� ���� �� ������� �������. �������������� ������� � ���������� ����,
// ����� �������� ������ �������.
static void st_5(TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  if (!ClVar->scptr) {
    ClVar->stat = PredicateState::Error;  // 1;
  } else {
    from_control(ClVar, heap);
    zero(ClVar);
    ClVar->head = heap->pacltarget[ClVar->atomp];
    ClVar->svptr = ClVar->oldsvptr;
    if (ClVar->PrSetting->Trace) {
      pldout("���������. ���� ");
      // buff[write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap)] = 0;
      write_term(ClVar->head, ClVar->frame2, 0, 0, ScVar, ClVar, heap);
      pldout(outBuff.c_str());
      outBuff.clear();
    }
    ClVar->stat = PredicateState::ControlStep;  // 4;
  }
}

// ����������, �������� �� ����������� ���� ���������� ����������.
static void st_6(TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto &outBuff = ClVar->outBuff;
  if (ClVar->PrSetting->Trace) {
    pldout("���� ");
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
      heap->pnclause = heap->GetPrecordclause(ClVar->newclause);
      heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
    }
    ClVar->stat = PredicateState::ControlStep;  // 4;
  }
}

// �������� ���������
static void OnControl(TClVar *ClVar, array *heap) {
  // add 2008/10/04
  heap->ptclause = NULL;
  heap->ptcltarget = NULL;

  auto &vmaxstack = ClVar->vmaxstack;

  ClVar->err = ErrorCode::NoErrors;  // 0
  for (size_t i = 0; i < ClVar->vmaxstack; i++) {
    ClVar->st_con[i] = isnil;
    ClVar->st_vr1[i] = isnil;
    ClVar->st_vr2[i] = isnil;
    ClVar->st_trail[i] = isnil;
  }
  for (size_t i = 0; i < _maxbf_; i++) {
    ClVar->bf[i] = 0;
  }
  ClVar->stat = PredicateState::PrepereNewTarget;  // 2;
  ClVar->newclause = NULL;
  ClVar->ntro = false;  // ��� ����������� ��������� �������� �� ���� ���� ��� ���
  ClVar->flag = true;
  ClVar->oldtptr = ClVar->tptr = NULL;  // ������ ����� �����
  ClVar->scptr = NULL;
  ClVar->frame1 = NULL;
  ClVar->parent = NULL;
  // aclause = heap->last - sizeof(recordclause);                          //���� ��� ?
  ClVar->aclause = heap->query;
  heap->paclause = heap->GetPrecordclause(ClVar->aclause);
  heap->pacltarget = heap->GetPunsigned(heap->paclause->ptrtarget);
  ClVar->varqu = heap->paclause->nvars;
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
      [[fallthrough]];         // ���� stat = 2 �� ���������� ����� �� � 3
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

// ================ ���������� =======================
// ������� 0 ������,isinteger-��������� �����,isfloat-��������� ��������������
size_t calculation(size_t term, size_t frame, IntegerType *i, FloatType *f, TClVar *ClVar, array *heap) {
  size_t ind = 0;  // ������������� ������� ��������� ��������� � ������

  void *st[maxstaccalc];
  int index = 0;  // ������ � ������� st ��������� �� ��������� ��-��
                  //  array st(maxstaccalc);
  ClVar->precordexpression = heap->GetPrecordexpression(term);
  size_t n = ClVar->precordexpression->length;
  ErrorCode err = ErrorCode::NoErrors;
  baserecord *tp;
  IntegerType oi1 = 0, oi2 = 0;                                // �������� ��� ���������� �����
  FloatType of1 = 0, of2 = 0;                                  // �������� ���  ���������� ������������
  for (int j = 0; j < n && err == ErrorCode::NoErrors; j++) {  // if (lowMemory()) return 0;//!!!��������� � r_t_e
    auto *ptr = heap->GetPunsigned(ClVar->precordexpression->precord);
    if (index == maxstaccalc) {
      err = ErrorCode::StackOverflowWhileCalculatingArithmeticsExpressions;  // 40;
      continue;
    }                     // ��� ������� �� ������ �����
                          // � ����������� ��� ����������(������������ ����)
    if (ptr[j] < isbase)  // �������
    {
      tp = heap->GetPbaserecord(ptr[j]);
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
        auto tr = ptr[j];
        auto fr = frame;
        auto a = occur_term(&tr, &fr, ClVar, heap);
        switch (a) {
        case 6: {
          ClVar->precordfloat = heap->GetPrecordfloat(tr);
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
        err = ErrorCode::ErrorInArithmeticExpression;  // 32;  // ������ ����� �� ������ ����
      }
      }
    } else {  // ���� ������ ��������
      for (int k = 0; k < 2; k++) {
        tp = (baserecord *)st[index - 1];
        switch (tp->ident)  // ����� ���� ������ integer ��� float
        {
        case isinteger:
          ClVar->precordinteger = (recordinteger *)tp;
          if (!k) {
            oi1 = ClVar->precordinteger->value;
            ind = isinteger;
          }  // ������ �������
          else if (ind == isinteger) {
            oi2 = ClVar->precordinteger->value;  // ������ �������
          } else {
            of2 = (FloatType)ClVar->precordinteger->value;  // ���� ������ ������������
          }
          delete st[index - 1];
          index--;
          break;
        case isfloat:
          ClVar->precordfloat = (recordfloat *)tp;
          if (!k) {
            of1 = ClVar->precordfloat->value;
            ind = isfloat;
          }     // ������ �������
          else  // ������ �������
          {
            of2 = ClVar->precordfloat->value;
            if (ind == isinteger) {
              ind = isfloat;
              of1 = (FloatType)oi1;
            }
          }
          delete st[index - 1];
          index--;
          break;
        }
      }  // ������� �� ����� ��� ��������
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
      st[index++] = (ind == isinteger) ? (void *)new recordinteger(oi2) : (void *)new recordfloat(of2);
      if (!st[index - 1]) {
        err = ErrorCode::NotEnoughFreeMemory;  // 2;
      }
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
  case isinteger:            // ������������
  case isfloat:              // ������������
  case isstring:             // ������������
  case issymbol:             // ������������
  case islist:               // ������������
  case isemptylist:          // ������������
  case isexpression: break;  // ret ��� false;
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
  switch (tp->ident)  // ��������� �� ���� ���� ����
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
  case isinteger:  // ������������
  case isfloat:
  case islist:
  case isemptylist:
  case isexpression: ret = false; break;
  case isunknown: nextun(ClVar, heap); break;
  }
  return ret;
}

// ��������� ����� � ����� ����������
void ocrn(size_t bd1, size_t tr2, TClVar *ClVar) {
  to_stac(ClVar->st_vr1, bd1, tr2);  // � ������� � ����� �����
  to_stac(ClVar->st_trail, ClVar->tptr++, bd1);
}

bool unvar(TClVar *ClVar, array *heap) {
  bool ret = true;
  size_t bd1, bd2, work;
  recordvar *precordvar = heap->GetPrecordvar(ClVar->term1);
  bd1 = precordvar->num + ClVar->fr1;
  work = from_stac(ClVar->st_vr1, bd1);
  if (work != isnil)  // ��������� ����������
  {
    ClVar->term1 = work;
    work = from_stac(ClVar->st_vr2, bd1);
    if (work != isnil)
      ClVar->fr1 = work;
    //   else {ocrn(bd1,term2);nextun();}//!!!��������� ����
  } else {
    baserecord *tp = heap->GetPbaserecord(ClVar->term2);
    switch (tp->ident) {
    case isfunction:
    case islist: to_stac(ClVar->st_vr2, bd1, ClVar->fr2);  // break; // ���� ����� break
    //[[fallthrough]]
    case isfloat:                                          // ����� ��� issumbol;
    case isinteger:                                        // ��� �� ��� issymbol
    case isstring:
    case isemptylist:  // ret=false;break; ���� ��� �� �� ���������  �������
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
        work = from_stac(ClVar->st_vr1, bd2);
        if (work == isnil) {  // ��������� ����������
          if (ClVar->term1 == ClVar->term2) {
            nextun(ClVar, heap);
          } else {
            to_stac(ClVar->st_vr2, bd1, ClVar->fr2);
            ocrn(bd1, ClVar->term2, ClVar);
            nextun(ClVar, heap);
          }
        } else {  // ��������� ����������
          ClVar->term2 = work;
          work = from_stac(ClVar->st_vr2, bd2);
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
  case isemptylist: break;  // ���� ������������
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
  } break;  // ret ��� false
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
  case isemptylist: break;  // ���� ������������
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
  case isfunction:           // ������������
  case isinteger:            // ������������
  case isfloat:              // ������������
  case isstring:             // ������������
  case issymbol:             // ������������
  case isemptylist:          // ������������
  case isexpression: break;  // ������������
  case isvar:
    swapints(ClVar);
    ret = true;
    break;
  case islist:  // if (numb==0)//!!! ���� (numb<2)
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
    if (ind1 == ind2 && ind1 == isinteger)  //!!! ������� ������ ��� �����
      if (c == d) {
        nextun(ClVar, heap);
        ret = true;
      } else
        ;  // ret=false; ��� ���������
    else
      ;  // ret =false;//!!! ����� ��������� �� ������ ������� ���������� ret=false ���
  }
  return ret;
}

bool unify(size_t tt1, size_t tt2, size_t ff1, size_t ff2, TClVar *ClVar, array *heap) {
  bool ret = true;
  ClVar->bp = NULL;
  ClVar->ex = true;
  ClVar->term1 = tt1;
  ClVar->term2 = tt2;
  ClVar->fr1 = ff1;
  ClVar->fr2 = ff2;
  while (ClVar->ex && ret) {  //!!!���������� �� ���������� ���������
                              //!!! �� ���� ����� ����� �������� �� ������� � hpcall ���� ��� ���� "���"
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
