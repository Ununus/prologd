#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <random>
#include <thread>
#include "pdefs.h"
#include "pstructs.h"
#include "extfunc.h"
#include "control.h"
#include "functions.h"
// #include <charconv>

// TODO: ������������� ������
const char *kPrologVersion = "14 ����� 2023";

PredicateState argnull(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (name) {
  case hpfail: return PredicateState::No;  // 5;  // ����
  case hptrace:
    ClVar->PrSetting->Trace = true;
    break;
  case hpnottr:
    ClVar->PrSetting->Trace = false;
    break;
  case hpcut:
    ClVar->scptr = ClVar->parent;  //"!"
    //   zero();
    if (ClVar->parent) {
      to_stac(ClVar->st_con, ClVar->parent - 1, isnil);
    }
    break;
  case hpquiet: ClVar->quiet = true; break;
  case hpversion: pldout(kPrologVersion); break;
  }
  return PredicateState::Yes;  // 3;
}

// extern unsigned *goal;
// extern unsigned frame2;
// extern occur_term(unsigned *,unsigned *);
void conarg(unsigned numb, unsigned h, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned i, term, frame;
  recordfunction *pf = heap->GetPrecordfunction(h);
  unsigned *ptr = heap->GetPunsigned(pf->ptrarg);
  for (i = 0; i < numb; i++) {
    frame = ClVar->frame2;
    pf = heap->GetPrecordfunction(h);
    ptr = heap->GetPunsigned(pf->ptrarg);
    term = ptr[i];
    ScVar->goal[i] = occur_term(&term, &frame, ClVar, heap);
    ScVar->goal[maxarity + i] = term;
    ScVar->goal[maxarity * 2 + i] = frame;
  }
}
//====================================�������=============

// extern unsigned head;
// extern unsigned frame2;
// extern unsigned newclause;
// extern char *wr_line;
// extern char *wptr;

// ��������� ������ ��������� � ����������
char *occ_line(int x, char *lnwr, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char *ad;
  recordstring *ps = heap->GetPrecordstring(ScVar->goal[maxarity + x]);
  ad = heap->GetPchar(ps->ptrsymb);
  for (x = 0; x < static_cast<int>(ps->length); x++) {
    *(lnwr + x) = *(ad + x);
  }
  *(lnwr + x) = 0;

  return lnwr;
}

// ���
PredicateState prcall(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned w;
  switch (sw) {
  case 8: {
    recordfunction *p = heap->GetPrecordfunction(ScVar->goal[maxarity]);
    w = p->func;
  } break;
  case 4:
    w = ScVar->goal[maxarity];  // symbol
    break;

  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e(41); // ������������ �������
  }
  }
  if (w < heap->freeheap) {
    if (w == hpassrt) {
      outerror(ErrorCode::UnknownError);  // 24
      return PredicateState::Error;       // 1; // r_t_e(41);
    }
    ClVar->head = ScVar->goal[maxarity];
    ClVar->frame2 = ScVar->goal[2 * maxarity];
    return PredicateState::Builtin;  // 6;  // ���������� ��������
  }

  // ������������ �������� ��� call
  recordsconst *ps = heap->GetPrecordsconst(w);
  if (ps->begin && ps->begin != isnil) {
    ClVar->newclause = ps->begin;
    if (ClVar->newclause) {
      heap->pnclause = heap->GetPrecordclause(ClVar->newclause);
      heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
    }
    return PredicateState::ControlStep;  // 4;
  } else {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e(41) �� ���������� �������� ���
  }
  return PredicateState::Error;  // 1;
}

// ������ ������ � ���������� ��� � arg
PredicateState zap3(const char *str, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!str) {
    outerror(ErrorCode::UnknownError);  // 45
    return PredicateState::No;          // 5;
  }
  size_t len;
  auto bakindex = heap->last;
  auto index = heap->append<char>(0, len = strlen(str));
  memcpy(heap->GetPchar(index), str, sizeof(char) * len);
  index = heap->append(recordstring(index, (unsigned char)len));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  unsigned *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index, ptr[arg - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  // heap->last = bakindex;
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ���������� ������ num � arg ���������� ���������
PredicateState zap1(IntegerType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto bakindex = heap->last;
  auto index = heap->append(recordinteger(num));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  unsigned *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index, ptr[arg - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  // heap->last = bakindex;
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ���������� ����� num1 � num2 � arg1 � arg2 ������������� ����������� ���������
PredicateState zap2(IntegerType num1, IntegerType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto bakindex = heap->last;
  auto index1 = heap->append(recordinteger(num1));
  auto index2 = heap->append(recordinteger(num2));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  unsigned *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index1, ptr[arg1 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap) && unify(index2, ptr[arg2 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  // heap->last = bakindex;
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ���������� float num � arg ���������� ���������
PredicateState zap1f(FloatType num, unsigned arg, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto bakindex = heap->last;
  auto index = heap->append(recordfloat(num));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  unsigned *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index, ptr[arg - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  // heap->last = bakindex;
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ���������� float num1 � arg1 ���������� ���������
PredicateState zap2f(FloatType num1, FloatType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto bakindex = heap->last;
  auto index1 = heap->append(recordfloat(num1));
  auto index2 = heap->append(recordfloat(num2));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  unsigned *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index1, ptr[arg1 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap) && unify(index2, ptr[arg2 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  // heap->last = bakindex;
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ����
PredicateState prrandom(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType n, m;
  static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  // TODO: multiprec rng gen
  // static std::uniform_int_distribution<IntegerType> dst;
  static std::uniform_int_distribution<long long> dst;
  switch (sw) {
  case 7:  // �����: ������������� ����������
    n = occ(0, ScVar, ClVar, heap);
    rng = std::mt19937(n.convert_to<long long>());
    return PredicateState::Yes;  // 3;
  case 5:                        // ����������: �������� ����� ��������
    return zap1(rng(), 1, ScVar, ClVar, heap);
  case 577:  // ����������: �������� ����� ��������
    n = occ(1, ScVar, ClVar, heap);
    m = occ(2, ScVar, ClVar, heap);
    if (n > m) {
      // TODO: it is known error
      outerror(ErrorCode::UnknownError);  // 24
      return PredicateState::Error;       // 1;
    }
    // dst = std::uniform_int_distribution<IntegerType>(n, m);
    dst = std::uniform_int_distribution<long long>(n.convert_to<long long>(), m.convert_to<long long>());
    return zap1(dst(rng), 1, ScVar, ClVar, heap);
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; //!!!r_t_e(71);
  }
  }
}

/*
bool see( void ) { return false; };
bool mytell( void ){ return false; };
*/
//=========================����� ����������

// ������_�
PredicateState outfile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char str[129];

  switch (sw) {
  case 9:
  case 4:  // char const
    occ_line(0, str, ScVar, ClVar, heap);
    ClVar->PrSetting->out.close();
    ClVar->PrSetting->name_out_file.clear();
    if (str[0] == 0) {
      return PredicateState::Yes;  // 3;
    }

    // ���� con:
    if (strcmp(str, "con:") == 0) {
      return PredicateState::Yes;  // 3;
    }

    // �������� ������� ���� ��� ������.
    ClVar->PrSetting->out.open(str);
    if (!ClVar->PrSetting->out.is_open()) {
      outerror(ErrorCode::AnExceptionOccurredDuringExecution);  // 43
      return PredicateState::No;                                // 5; // r_t_e_(�� ���� ������� ����)
    }
    ClVar->PrSetting->name_out_file = std::string(str);
    return PredicateState::Yes;  // 3;
  case 5:                        // char* ps=newStr(namefileout);
    return zap3(ClVar->PrSetting->name_out_file.c_str(), 1, ScVar, ClVar, heap);

  default: return PredicateState::Error;  // 1  // r_t_e(45); ������ ��� �������� �����
  }
  return PredicateState::Error;  // 1
}

// ������_��
PredicateState infile(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char str[129];

  switch (sw) {
  case 9:
  case 4:  // char const
    occ_line(0, str, ScVar, ClVar, heap);
    ClVar->PrSetting->in.close();
    ClVar->PrSetting->name_in_file.clear();
    if (str[0] == 0) {
      break;
    }

    if (strcmp(str, "con:") == 0) {
      return PredicateState::Yes;  // 3;
    }

    ClVar->PrSetting->in.open(str);
    if (!ClVar->PrSetting->in.is_open()) {
      outerror(ErrorCode::AnExceptionOccurredDuringExecution);  // 43
      return PredicateState::No;                                // 5; // r_t_e_(�� ���� ������� ����)
    }
    ClVar->PrSetting->name_in_file = std::string(str);
    return PredicateState::Yes;  // 3;

  case 5:  // char* ps=newStr(namefilein);
    return zap3(ClVar->PrSetting->name_in_file.c_str(), 1, ScVar, ClVar, heap);

  default: return PredicateState::Error;  // 1;  // r_t_e(45); ������ ��� �������� �����
  }
  return PredicateState::Error;  // 1;
}

// ��������� ������ ���������� � ����������
IntegerType occ(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap) {
  recordinteger *pint = heap->GetPrecordinteger(ScVar->goal[maxarity + x]);
  return pint->value;
}

// ��������� ������ ���������� � ����������
FloatType occf(unsigned x, TScVar *ScVar, TClVar *ClVar, array *heap) {
  recordfloat *pf = heap->GetPrecordfloat(ScVar->goal[maxarity + x]);
  return pf->value;
}

// �����
PredicateState priocod(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  static char str0[2]{};
  switch (sw) {
  case 7:  // ����� !!!���������� �� ���� ������������ ����� ������ ������
  {
    if (ClVar->PrSetting->out.is_open()) {
      ClVar->PrSetting->out << char(occ(0, ScVar, ClVar, heap));
    } else {
      str0[0] = char(occ(0, ScVar, ClVar, heap));
      usrout(str0);
    }
    return PredicateState::Yes;  // 3;
  }
  case 5:  // ����������
  {
    char w;
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return PredicateState::No;  // 5;
      }
    }
    InputSymbol(&w);
    return zap1(w, 1, ScVar, ClVar, heap);
  }
    //  case 1://��������
    //    if (ClVar->PrSetting->fin)
    //      fgetc(ClVar->PrSetting->fin);//�������� ��������� ����� ???
    //    return 3;
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e(44);
  }
  }
  return PredicateState::Error;  // 1;
}

// �������
PredicateState prrdint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType w{};
  char str0[129]{};
  const char *caption = "������� �����";

  switch (sw) {
  case 7:   // �����
  case 74:  // �����, "caption"
  case 79:  // �����, caption
  {         // ���� �������� ����� ������� � ���������� �� ������ ����� ����
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return PredicateState::No;  // 5;
      }
    } else {
      if (sw == 74 || sw == 79) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (InputInt(&w, str0)) {
          return PredicateState::No;  // 5;
        }
      } else {
        if (InputInt(&w, caption)) {
          return PredicateState::No;  // 5;
        }
      }
    }
    return (occ(0, ScVar, ClVar, heap) == w) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 5:   // ����������
  case 54:  // ����������, "caption"
  case 59:  // ����������, caption

  case 1:
  case 14:
  case 19: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return PredicateState::No;  // 5;
      }
    } else {
      if (sw == 54 || sw == 59 || sw == 14 || sw == 19) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (InputInt(&w, str0)) {
          return PredicateState::No;  // 5;
        }
      } else {
        if (InputInt(&w, caption)) {
          return PredicateState::No;  // 5;
        }
      }
    }
    return zap1(w, 1, ScVar, ClVar, heap);
  }
  default: break;
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // rte
}

// �������
PredicateState prrdfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  FloatType w{};
  char str0[129]{};
  const char *caption = "������� ������������";

  switch (sw) {
  case 6:   // Float
  case 64:  // Float, "caption"
  case 69:  // Float, caption
  {         // ���� �������� ����� ������� � ���������� �� ������ ����� ����
            // TODO ����� �� ��� eps?.
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return PredicateState::No;  // 5;
      }
    } else {
      if (sw == 64 || sw == 69) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (InputFloat(&w, str0)) {
          return PredicateState::No;  // 5;
        }
      } else {
        if (InputFloat(&w, caption)) {
          return PredicateState::No;  // 5;
        }
      }
    }
    return (occf(0, ScVar, ClVar, heap) == w) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 5:   // ����������
  case 54:  // ����������, "caption"
  case 59:  // ����������, caption

  case 1:
  case 14:
  case 19: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return PredicateState::No;  // 5;
      }
    } else {
      if (sw == 54 || sw == 59 || sw == 14 || sw == 19) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (InputFloat(&w, str0)) {
          return PredicateState::No;  // 5;
        }
      } else {
        if (InputFloat(&w, caption)) {
          return PredicateState::No;  // 5;
        }
      }
    }
    return zap1f(w, 1, ScVar, ClVar, heap);
  }
  default: break;
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // rte
}

// ��������
PredicateState prrdsym(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char str0[255]{}, str1[255]{}, str2[255]{};
  switch (sw) {
  case 9:
  case 4:
  case 99: /*�� �� ����� ��� � ���������� ����� �� � ���������� ����������� ����*/
  case 94:
  case 49:
  case 44: {
    if (ClVar->PrSetting->in.is_open()) {
      ClVar->PrSetting->in.get(str2, 255);
      for (auto ch = ClVar->PrSetting->in.peek(); ch == 10 || ch == 13; ch = ClVar->PrSetting->in.peek()) {
        ClVar->PrSetting->in.get();
      }
      if (!(str2[0])) {
        break;
      }
    } else {
      if (sw == 99 || sw == 94 || sw == 49 || sw == 44) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputstring(str2, sizeof(str2), str0)) {  // �� cancel ������ 1;
          break;
        }
      } else {
        if (Inputstring(str2, sizeof(str2))) {  // �� cancel ������ 1;
          break;
        }
      }
    }
    return (strcmp(occ_line(0, str1, ScVar, ClVar, heap), str2)) ? PredicateState::No : PredicateState::Yes;  // 5 : 3;
  }
  case 5:
  case 59:
  case 54: {
    if (ClVar->PrSetting->in.is_open()) {
      ClVar->PrSetting->in.get(str2, 255);
      for (auto ch = ClVar->PrSetting->in.peek(); ch == 10 || ch == 13; ch = ClVar->PrSetting->in.peek()) {
        ClVar->PrSetting->in.get();
      }
      if (!(str2[0])) {
        break;
      }
    } else {
      if (sw == 59 || sw == 54) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputstring(str2, sizeof(str2), str0)) {  // �� cancel ������ 1;
          break;
        }
      } else {
        if (Inputstring(str2, sizeof(str2))) {  // �� cancel ������ 1;
          break;
        }
      }
    }
    return zap3(str2, 1, ScVar, ClVar, heap);
  }
  case 1:  // ��������
  case 19:
  case 14: {
    if (ClVar->PrSetting->in.is_open()) {
      ClVar->PrSetting->in.get(str2, 255);
      for (auto ch = ClVar->PrSetting->in.peek(); ch == 10 || ch == 13; ch = ClVar->PrSetting->in.peek()) {
        ch = ClVar->PrSetting->in.get();
      }
      if (!(str2[0])) {
        break;
      }
    } else {
      if (sw == 19 || sw == 14) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputstring(str2, sizeof(str2), str0)) {  // �� cancel ������ 1;
          break;
        }
      } else {
        if (Inputstring(str2, sizeof(str2))) {  // �� cancel ������ 1;
          break;
        }
      }
    }
    return PredicateState::Yes;  // 3;
  }
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e
  }
  }
  return PredicateState::No;  // 5;
}

// TODO ������� ����� ���� ����� ��� � ��������
// �������
PredicateState prrdstr(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char str0[255]{}, str1[255]{}, str2[255]{};
  switch (sw) {
  case 9:
  case 4:
  case 99: /*�� �� ����� ��� � ���������� ����� �� � ���������� ����������� ����*/
  case 94:
  case 49:
  case 44: {
    if (ClVar->PrSetting->in.is_open()) {
      ClVar->PrSetting->in.get(str2, 255);
      for (auto ch = ClVar->PrSetting->in.peek(); ch == 10 || ch == 13; ch = ClVar->PrSetting->in.peek()) {
        ClVar->PrSetting->in.get();
      }
      if (!(str2[0])) {
        break;
      }
    } else {
      if (sw == 99 || sw == 94 || sw == 49 || sw == 44) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputline(str2, sizeof(str2), str0)) {  // �� cancel ������ 1;
          break;
        }
      } else {
        if (Inputline(str2, sizeof(str2))) {  // �� cancel ������ 1;
          break;
        }
      }
    }
    return (strcmp(occ_line(0, str1, ScVar, ClVar, heap), str2)) ? PredicateState::No : PredicateState::Yes;  // 5 : 3;
  }
  case 5:
  case 59:
  case 54: {
    if (ClVar->PrSetting->in.is_open()) {
      ClVar->PrSetting->in.get(str2, 255);
      for (auto ch = ClVar->PrSetting->in.peek(); ch == 10 || ch == 13; ch = ClVar->PrSetting->in.peek()) {
        ClVar->PrSetting->in.get();
      }
      if (!(str2[0])) {
        break;
      }
    } else {
      if (sw == 59 || sw == 54) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputline(str2, sizeof(str2), str0)) {  // �� cancel ������ 1;
          break;
        }
      } else {
        if (Inputline(str2, sizeof(str2))) {  // �� cancel ������ 1;
          break;
        }
      }
    }
    return zap3(str2, 1, ScVar, ClVar, heap);
  }
  case 1:  // ��������
  case 19:
  case 14: {
    if (ClVar->PrSetting->in.is_open()) {
      ClVar->PrSetting->in.get(str2, 255);
      for (auto ch = ClVar->PrSetting->in.peek(); ch == 10 || ch == 13; ch = ClVar->PrSetting->in.peek()) {
        ch = ClVar->PrSetting->in.get();
      }
      if (!(str2[0])) {
        break;
      }
    } else {
      if (sw == 19 || sw == 14) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputline(str2, sizeof(str2), str0)) {  // �� cancel ������ 1;
          break;
        }
      } else {
        if (Inputline(str2, sizeof(str2))) {  // �� cancel ������ 1;
          break;
        }
      }
    }
    return PredicateState::Yes;  // 3;
  }
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e
  }
  }
  return PredicateState::No;  // 5;
}

// �������������� � �����
PredicateState print(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: return PredicateState::Yes;  // 3;  // ��� �����
  case 6: return zap1(static_cast<IntegerType>(occf(0, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::ErrorInBuiltinPredicateInt);  // 36
  return PredicateState::Error;                     // 1
}

// �������������� � float
PredicateState prfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: return zap1f(static_cast<FloatType>(occ(0, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  case 6: return PredicateState::Yes;  // 3;
  }
  outerror(ErrorCode::ErrorInBuiltinPredicateFloat);  // 37
  return PredicateState::Error;                       // 1
}

// ���
PredicateState prwait(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: break;
  default: {
    // TODO: It is known error
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1;
  }
  }
  IntegerType n = occ(0, ScVar, ClVar, heap);
  if (n > 0) {
    std::this_thread::sleep_for(std::chrono::duration<size_t, std::milli>(n.convert_to<size_t>()));
  }
  return PredicateState::Yes;  // 3;
}

PredicateState argone(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned sw;
  conarg(1, ClVar->head, ScVar, ClVar, heap);
  sw = ScVar->goal[0];
  switch (name) {
  case hpcall: return prcall(sw, ScVar, ClVar, heap);                                  // ���
  case hprdsym: return prrdsym(sw, ScVar, ClVar, heap);                                // ��������
  case hprdstr: return prrdstr(sw, ScVar, ClVar, heap);                                // �������
  case hprdint: return prrdint(sw, ScVar, ClVar, heap);                                // �������
  case hprdfloat: return prrdfloat(sw, ScVar, ClVar, heap);                            // �������
  case hpiocod: return priocod(sw, ScVar, ClVar, heap);                                // �����
  case hpsee: return infile(sw, ScVar, ClVar, heap);                                   // ������_��
  case hptell: return outfile(sw, ScVar, ClVar, heap);                                 // ������_�
  case hpvar: return (sw == 5) ? PredicateState::Yes : PredicateState::No;             // 3 : 5; // ���
  case hpint: return (sw == 7) ? PredicateState::Yes : PredicateState::No;             // 3 : 5; // ���
  case hpfloat: return (sw == 6) ? PredicateState::Yes : PredicateState::No;           // 3 : 5; // ���
  case hpsym: return (sw == 4) ? PredicateState::Yes : PredicateState::No;             // 3 : 5; // ����
  case hplst: return (sw == 2 || sw == 3) ? PredicateState::Yes : PredicateState::No;  // 3 : 5; // ������ (������ ��� � ���-��)
  case hpwait: return prwait(sw, ScVar, ClVar, heap);                                  // ���
  case hprand: return prrandom(sw, ScVar, ClVar, heap);                                // ����
  case hp_int: return print(sw, ScVar, ClVar, heap);                                   // �������������� int
  case hp_float: return prfloat(sw, ScVar, ClVar, heap);                               // �������������� float
  }
  return PredicateState::Error;  // 1;
}

// ������
PredicateState prgt(TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType a[2];
  int ind = 0;
  FloatType af[2];
  for (int i = 0; i < 2; i++) {
    switch (ScVar->goal[i]) {
    case 7:
      ind += 1 * (i + 1);
      a[i] = occ(i, ScVar, ClVar, heap);
      break;  // ind==1 || 2
    case 6:
      ind += 3 * (i + 1);
      af[i] = occf(i, ScVar, ClVar, heap);
      break;  // ind==3 || 6
    default: ind = 10;
    }
  }
  switch (ind) {
  case 3: return (a[0] > a[1]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  case 7: af[0] = (FloatType)a[0]; break;
  case 5: af[1] = (FloatType)a[1]; break;
  case 9: break;
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e(51)//� ������ �� �����
  }
  }
  return (af[0] > af[1]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
}
// ��������� ������ �������� �� ��������� tp - tp ������ ���� list
int GetStrFromList(char *Buf, size_t BufSize, baserecord *tp, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char *p = Buf;
  int len = 0;
  baserecord *pb;
  recordlist *plist;
  while (tp->ident == islist) {
    plist = (recordlist *)tp;
    pb = heap->GetPbaserecord(plist->head);
    switch (pb->ident) {
    case isvar: {
      auto _Term = plist->head;
      auto _Frame = ClVar->frame2;
      auto a = occur_term(&_Term, &_Frame, ClVar, heap);
      pb = heap->GetPbaserecord(_Term);
      if (pb->ident == islist) {
        int _len = GetStrFromList(p, BufSize - len, pb, ScVar, ClVar, heap);
        if (_len < 0)
          return -1;
        len += _len;
        p += _len;
      }
      if (pb->ident == isinteger) {
        if (BufSize - len - 1 < 0) {
          outerror(ErrorCode::TooLongList);  // 44
          return -1;
        }
        recordinteger *pint = (recordinteger *)pb;
        *(p + len) = (char)pint->value;
        len++;
      }
    } break;
    case isemptylist: break;
    case islist: {
      int _len = GetStrFromList(p, BufSize - len, pb, ScVar, ClVar, heap);
      if (_len < 0)
        return -1;
      len += _len;
      p += _len;
    } break;
    case isinteger: {
      if (BufSize - len - 1 < 0) {
        outerror(ErrorCode::TooLongList);  // 44
        return -1;
      }
      recordinteger *pint = (recordinteger *)pb;
      *(p + len) = (char)pint->value;
      len++;
    } break;
    default: {
      outerror(ErrorCode::WrongList);  // 15
      return -1;
    }  // R_t_e ��������� �� []
    }
    tp = heap->GetPbaserecord(plist->link);
  }
  /*
  if (tp->ident != isemptylist)
  {
    outerror(15);
    return -1;
  }//R_t_e ��������� �� []
  */
  *(p + len) = 0;
  return len;
}

// �������
PredicateState prstlst(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char lnwr[maxlinelen];
  baserecord *tp = 0;
  // recordlist* plist;
  // recordinteger* pint;
  // recordvar* pvar;
  switch (sw) {
  case 93:  // string list
  case 43:  // issymbol list
  case 42:  // issymbol []
  case 53:  // isvar list
  case 52:  // isvar []
  {
    tp = heap->GetPbaserecord(ScVar->goal[maxarity + 1]);
    //(baserecord *)&heap->heaps[ScVar->goal[maxarity + 1]];
    int len = GetStrFromList(lnwr, sizeof(lnwr), tp, ScVar, ClVar, heap);
    /*
    while (tp->ident == islist)
    {
      plist = (recordlist *)tp;
      pvar = heap->GetPrecordvar(plist->head);
      pint = heap->GetPrecordinteger(plist->head);
            //(recordinteger *)&heap->heaps[plist->head];
      if (pvar->ident == isvar)
      {	unsigned _Term = plist->head;
            unsigned _Frame = ClVar->frame2;
            unsigned a = occur_term(&_Term, &_Frame,
              ClVar, heap);
            pint = heap->GetPrecordinteger(_Term);
            if (a == 7)
            {
              pint = heap->GetPrecordinteger(_Term);
            }
      }
      if (pint->ident != isinteger)
      {
            outerror(24);
            return 1;
      }//r_t_e ������ �������������
      lnwr[i++] = (char)pint->value;
      tp = heap->GetPbaserecord(plist->link);
            //(baserecord *)&heap->heaps[plist->link];
    }
    if (tp->ident != isemptylist)
    {
      outerror(24);
      return 1;
    }//R_t_e ��������� �� []
    */
    if (len < 0) {
      return PredicateState::Error;  // 1;
    }
    lnwr[len] = 0;
    // char *str=newStr(lnwr);
    // char * str = lnwr;
    // if (!str)
    //	break;
    switch (sw) {
    case 53:  // heap->insert(str);
      return zap3(lnwr, 1, ScVar, ClVar, heap);
    case 43:
    case 93:
      char str[maxlinelen]{};
      occ_line(0, lnwr, ScVar, ClVar, heap);
      auto k = strncmp(str, lnwr, strlen(str));
      return (k == NULL) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
    }
  }
  case 95:  //   str/sym var
  case 45: {
    occ_line(0, lnwr, ScVar, ClVar, heap);
    auto k = strlen(lnwr);
    recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
    unsigned *ptr = heap->GetPunsigned(pfunction->ptrarg);
    if (!k) {
      auto index = heap->append(recordemptylist());
      unify(index, ptr[1], ClVar->frame2, ClVar->frame2, ClVar, heap);
      return PredicateState::Yes;  // 3;
    } else {
      // ������ �� ������
      size_t j = heap->last, index1, index2;
      for (int i = 0; i < k; i++) {  // 2006/10/17
        index1 = heap->append(recordinteger((unsigned char)lnwr[i]));
        index2 = heap->append(recordlist(index1, heap->last + sizeof(recordinteger) + sizeof(recordlist)));
      }
      auto index = heap->append(recordemptylist());
      recordlist *plst = heap->GetPrecordlist(index2);
      plst->link = index;
      unify(j + sizeof(recordinteger), ptr[1], ClVar->frame2, ClVar->frame2, ClVar, heap);
      return PredicateState::Yes;  // 3;
    }
  }
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // R_t_e
  }
  }
  return PredicateState::No;  // 5;  //!!! ���������� �� ��� ��������
}

// ������
PredicateState prstint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType w{};
  char lnwr[maxlinelen]{};
  switch (sw) {
  case 97:  // str int
  case 95:  // str var
  case 47:  // symb int
  case 45:  // symb var
  {
    occ_line(0, lnwr, ScVar, ClVar, heap);
    // auto [ptr, erc] = std::from_chars(lnwr, lnwr + sizeof(lnwr), w);
    // if (erc != std::errc{}) {
    //   return PredicateState::No;
    // }

    try {
      // ����� ����� from_chars ��� atoll
      w = std::stoll(std::string(lnwr));
    } catch (...) {
      return PredicateState::No;
    }

    if (sw == 95 || sw == 45) {
      return zap1(w, 2, ScVar, ClVar, heap);
    }
    return (w == occ(1, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 57:  // var int  �������� ����� var float
  {
    // std::to_chars(lnwr, lnwr + maxlinelen, occ(1, ScVar, ClVar, heap));
    //  �������� �� to_chars, �� �� ��������
    //_ltoa(occ(1, ScVar, ClVar, heap), lnwr, 10);
    //  char *str=newStr(lnwr);

    sprintf(lnwr, "%s", occ(1, ScVar, ClVar, heap).str().c_str());

    return zap3(lnwr, 1, ScVar, ClVar, heap);
  }
  case 56: {
    FloatType value = occf(1, ScVar, ClVar, heap);
    sprintf(lnwr, "%lf", value);
    //  std::to_chars(lnwr, lnwr + maxlinelen, value); // � gcc �� �����������
    //   �������� ��, �� �� ��������
    //  int dec, sign, ndig = 5;
    //  char* p = lnwr;
    //  p = _fcvt(value, ndig, &dec, &sign);
    //  int j = 0, i;
    //  if (sign)
    //    lnwr[j++] = '-';
    //  for (i = 0; (i < dec && i + j < maxlinelen);
    //    lnwr[j + i] = p[i], i++);
    //  lnwr[j + i] = '.';
    //  j++;
    //  for (; (i + j < maxlinelen && p[i]); lnwr[i + j] = p[i], i++);
    //  lnwr[j + i] = NULL;

    // str=newStr(lnwr);
    return zap3(lnwr, 1, ScVar, ClVar, heap);
  }
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // R_t_e
  }
  }                              // outerror(2);
  return PredicateState::Error;  // 1; //R_T_e ��� ������
}

// ������
PredicateState prstfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  FloatType w{};
  char lnwr[maxlinelen]{};
  switch (sw) {
  case 96:  // str float
  case 95:  // str var
  case 46:  // symb float
  case 45:  // symb var
  {
    occ_line(0, lnwr, ScVar, ClVar, heap);

    try {
      // ����� ����� std::from_chars(lnwr, lnwr + sizeof(lnwr), w) ��� atof;
      w = std::stod(std::string(lnwr));
    } catch (...) {
      return PredicateState::No;
    }
    if (sw == 95 || sw == 45) {
      return zap1f(w, 2, ScVar, ClVar, heap);
    }
    return (w == occf(1, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 56: {
    FloatType value = occf(1, ScVar, ClVar, heap);
    sprintf(lnwr, "%f", value);
    return zap3(lnwr, 1, ScVar, ClVar, heap);
  }
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1;
  }
  }
  return PredicateState::Error;
}
// ������
PredicateState printfloat(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType int_val = 0;
  FloatType float_val = 0.f;
  char lnwr[maxlinelen]{};
  switch (sw) {
  case 76:                                                                                                                       // int float
    return (occ(0, ScVar, ClVar, heap) == (IntegerType)occf(1, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  case 75:                                                                                                                       // int var
  {
    int_val = occ(0, ScVar, ClVar, heap);
    float_val = static_cast<FloatType>(int_val);
    return zap1f(float_val, 2, ScVar, ClVar, heap);
  }
  case 56:  // var float
  {
    float_val = occf(1, ScVar, ClVar, heap);
    int_val = static_cast<IntegerType>(float_val);
    return zap1(int_val, 1, ScVar, ClVar, heap);
  }
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1;
  }
  }
  return PredicateState::Error;
}

bool digit(char a) {
  return (isdigitrus(a) == NULL) ? false : true;
}

bool letter(char a) {
  return (isalpharus(a) || isalphaang(a)) ? true : false;
}

// ������ unsigned i ??? // ����� ���� unsignedi !!!
// �����; �����
PredicateState whatisit(unsigned sw, bool (*f)(char), unsigned i, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char lnwr[maxlinelen];
  IntegerType w;
  occ_line(0, lnwr, ScVar, ClVar, heap);
  auto len = strlen(lnwr);
  switch (sw) {
  case 97:  // str int
  case 47:
    w = occ(1, ScVar, ClVar, heap);
    if (w && len >= w) {
      w -= 1;
      return (*f)(lnwr[w.convert_to<size_t>()]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
    }
    break;
  case 95:  // str var
  case 45:
    for (w = 0; w < len && !(*f)(lnwr[w.convert_to<size_t>()]); w++)
      ;
    if (w < len)
      return zap1(w + 1, 2, ScVar, ClVar, heap);
    break;
  default: return PredicateState::Error;  // (i == i) ? 1 : 1;  // r_t_e w ��� ������ (��������� ����� ��� �����)
  }                                       // i ������������ ��� r_t_e
  return PredicateState::No;              // 5;
}

// �������
PredicateState prskol(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  int i = 0;
  recordsconst *ps = heap->GetPrecordsconst(ScVar->goal[maxarity]);
  if (ps->begin != NULL && ps->begin != isnil) {
    recordclause *pcl = heap->GetPrecordclause(ps->begin);
    i++;
    while (pcl->next != isnil && pcl->next != NULL) {
      i++;
      pcl = heap->GetPrecordclause(pcl->next);
    }
  }
  switch (sw) {
  case 45:  // str var
    return zap1(i, 2, ScVar, ClVar, heap);
  case 47:                                                                                // str int
    return (i == occ(1, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // r_t_e
}

// ����
PredicateState prterm(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  baserecord *tp;
  recordlist *plist;
  int i = 0;
  switch (sw) {
  case 83:
  case 43:
  case 93:
  case 53: {
    tp = heap->GetPbaserecord(ScVar->goal[maxarity + 1]);
    if (tp->ident == islist) {
      plist = (recordlist *)tp;
      baserecord *test = heap->GetPbaserecord(plist->head);
      if (test->ident != issymbol) {
        return PredicateState::No;  // 5;
      }
    }
    plist = (recordlist *)tp;
    while (tp->ident == islist) {
      plist = (recordlist *)tp;
      tp = heap->GetPbaserecord(plist->link);
      i++;  // ����� ��������� � ������
    }
    if (tp->ident != isemptylist) {
      outerror(ErrorCode::UnknownError);  // 24
      return PredicateState::Error;       // 1; // R_t_e ��������� �� []
    }
    if (i == 1) {
      recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
      unsigned *ptr = heap->GetPunsigned(pf->ptrarg);
      if (unify(ptr[0], plist->head, ClVar->frame2, ClVar->frame2, ClVar, heap)) {
        return PredicateState::Yes;  // 3;
      }
      //				heap->last=bakindex; //??????
      return PredicateState::No;  // 5;
    }
    // ������� ������ ������� � ����������� �� ������
    unsigned n = i - 1;  // ����� ����� � ����� �������
    auto index = heap->append<unsigned>(0, n);
    unsigned int *ptrargs = heap->GetPunsigned(index);
    plist = heap->GetPrecordlist(ScVar->goal[maxarity + 1]);
    unsigned funcsymb = plist->head;
    i = 0;
    plist = heap->GetPrecordlist(plist->link);
    while (plist->ident == islist) {
      ptrargs[i] = plist->head;
      plist = heap->GetPrecordlist(plist->link);
      i++;  // ����� ��������� � ������
    }
    index = heap->append(recordfunction((unsigned char)n, funcsymb, index));
    recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
    unsigned *ptr = heap->GetPunsigned(pf->ptrarg);
    if (unify(ptr[0], index, ClVar->frame2, ClVar->frame2, ClVar, heap)) {
      return PredicateState::Yes;  // 3;
    }
    //          heap->last=bakindex; //????????
    return PredicateState::No;  // 5;
  }
  case 85:  // func var
  {         // func -> list
    recordfunction *rf = heap->GetPrecordfunction(ScVar->goal[maxarity]);
    unsigned narg = 0;  // rf->narg;
    auto oldindex = heap->last;
    recordlist pl(rf->func, oldindex + sizeof(recordlist));
    unsigned *ptrarg = heap->GetPunsigned(rf->ptrarg);
    auto tlist = heap->append(pl);
    while (narg < rf->narg) {
      pl.head = ptrarg[narg];
      pl.link = heap->last + sizeof(recordlist);
      heap->append(pl);
      narg++;
    }
    heap->append(recordemptylist());
    rf = heap->GetPrecordfunction(ClVar->head);
    ptrarg = heap->GetPunsigned(rf->ptrarg);
    return (unify(ptrarg[1], tlist, ClVar->frame2, ClVar->frame2, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 95:  // symb var
  case 45:  // str var -��� ���� ���� �������� ??
  {         // symb -> list
    auto oldindex = heap->last;
    recordlist pl(ScVar->goal[maxarity], oldindex + sizeof(recordlist));
    auto tlist = heap->append(pl);
    heap->append(recordemptylist());
    recordfunction *rf = heap->GetPrecordfunction(ClVar->head);
    unsigned *ptrarg = heap->GetPunsigned(rf->ptrarg);
    return (unify(ptrarg[1], tlist, ClVar->frame2, ClVar->frame2, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  }
  return PredicateState::No;  // 5;
}

// ����
PredicateState prdel(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (sw != 47) {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e
  }
  IntegerType i = occ(1, ScVar, ClVar, heap);
  recordsconst *ps = heap->GetPrecordsconst(ScVar->goal[maxarity]);
  if (ps->begin == isnil || ps->begin == NULL) {
    return PredicateState::No;  // 5;  // ��� �����������
  }
  recordclause *pcpred = 0;
  recordclause *pc = heap->GetPrecordclause(ps->begin);
  unsigned w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w++) {
    pcpred = pc;
    pc = heap->GetPrecordclause(pc->next);
    //(recordclause *)&heap->heaps[pc->next];
  }

  if (w == 1) {  // ������ �����������  ���� ���� ���������
    ps->begin = pc->next;
  } else {
    if (!pcpred) {
      pcpred = heap->GetPrecordclause(ps->begin);
    }
    pcpred->next = pc->next;
  }
  return PredicateState::Yes;  // 3;
}

PredicateState argtwo(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned sw;
  conarg(2, ClVar->head, ScVar, ClVar, heap);
  sw = ScVar->goal[0] * 10 + ScVar->goal[1];
  switch (name) {
  case hpgt: return prgt(ScVar, ClVar, heap);                         // ������
  case hpstint: return prstint(sw, ScVar, ClVar, heap);               // ������
  case hpstfloat: return prstfloat(sw, ScVar, ClVar, heap);           // ������
  case hpstlst: return prstlst(sw, ScVar, ClVar, heap);               // �������
  case hpintfloat: return printfloat(sw, ScVar, ClVar, heap);         // ������
  case hplettr: return whatisit(sw, letter, 55, ScVar, ClVar, heap);  // �����
  case hpdigit: return whatisit(sw, digit, 56, ScVar, ClVar, heap);   // �����
  case hpterm: return prterm(sw, ScVar, ClVar, heap);                 // ����
  case hpdel: return prdel(sw, ScVar, ClVar, heap);                   // ����
  case hpskol: return prskol(sw, ScVar, ClVar, heap);                 // �������
  case hprdsym: return prrdsym(sw, ScVar, ClVar, heap);               // �������� (� ����������)
  case hprdstr: return prrdstr(sw, ScVar, ClVar, heap);               // �������
  case hprdint: return prrdint(sw, ScVar, ClVar, heap);               // �������
  case hprdfloat: return prrdfloat(sw, ScVar, ClVar, heap);           // �������
  }
  return PredicateState::Error;  // 1;
}

// �����
PredicateState prset(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType xy, x1, x2, y1, y2, color;
  switch (sw) {
  case 777: {
    x1 = occ(0, ScVar, ClVar, heap);
    x2 = x1 + 5;
    y1 = occ(1, ScVar, ClVar, heap);
    y2 = y1 + 5;
    color = occ(2, ScVar, ClVar, heap);
    SetPixel(x1.convert_to<long long>(), y1.convert_to<long long>(), color.convert_to<unsigned>());
  } break;
  case 775: {
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = occ(1, ScVar, ClVar, heap);
    color = GetPixel(x1.convert_to<long long>(), y1.convert_to<long long>());
    return zap1(color, 3, ScVar, ClVar, heap);
  }
  case 757: {
    xy = occ(0, ScVar, ClVar, heap);
    color = occ(2, ScVar, ClVar, heap);
    x1 = xy;
    x2 = xy;
    y1 = 0;
    y2 = maxgry;
    vertical(x1.convert_to<long long>(), y1.convert_to<long long>(), x2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;
  case 577: {
    xy = occ(1, ScVar, ClVar, heap);
    y1 = xy;
    y2 = xy;
    x1 = 0;
    x2 = maxgrx;
    color = 0;
    horisontal(x1.convert_to<long long>(), y1.convert_to<long long>(), y2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;
  case 557: {
    color = occ(2, ScVar, ClVar, heap);
    ClearView(color.convert_to<unsigned>());
  } break;
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1;
  }                                     // r_t_e(�� ��� ������ �����
  }
  return PredicateState::Yes;  // 3;
}

// ����
PredicateState prapp(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char wrln1[maxlinelen], wrln2[maxlinelen];
  switch (sw) {
  case 444:
  case 449:
  case 494:
  case 499:
  case 944:
  case 949:
  case 994:
  case 999:  // ��� str str str
  case 445:
  case 495:
  case 945:
  case 995:  // str str var
  {
    occ_line(0, wrln1, ScVar, ClVar, heap);
    occ_line(1, wrln2, ScVar, ClVar, heap);
    strcat(wrln1, wrln2);
    auto w = strlen(wrln1);
    if (w >= maxlinelen) {
      outerror(ErrorCode::UnknownError);  // 24
      return PredicateState::Error;       // 1;
    }
    if (sw == 445 || sw == 495 || sw == 945 || sw == 995) {
      // ���� ����������
      return zap3(wrln1, 3, ScVar, ClVar, heap);
    }
    occ_line(2, wrln2, ScVar, ClVar, heap);
    return (!strncmp(wrln1, wrln2, strlen(wrln1))) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 454:
  case 459:
  case 954:  // str var str
  case 959: {
    occ_line(0, wrln1, ScVar, ClVar, heap);
    occ_line(2, wrln2, ScVar, ClVar, heap);
    auto w = strlen(wrln1);
    auto i = strlen(wrln2);
    // ���� lnwr1 ������ � lnwr2 � ������ �������

    if (w <= i && !strncmp(wrln1, wrln2, w))  // 2006/10/17
    {
      return zap3(&wrln2[w], 2, ScVar, ClVar, heap);
    }
    break;
  }
  case 544:
  case 549:
  case 594:  // var str str
  case 599: {
    occ_line(1, wrln1, ScVar, ClVar, heap);
    occ_line(2, wrln2, ScVar, ClVar, heap);
    auto w = strlen(wrln1);
    auto i = strlen(wrln2);
    if (i > w && !strncmp(wrln1, &wrln2[i - w], w)) {
      wrln2[i - w] = NULL;
      return zap3((char *)wrln2, 1, ScVar, ClVar, heap);
    }
    break;
  }
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e
  }
  }
  return PredicateState::No;  // 5;
}

// ��� ������� �������� � ����. ��� ���������� �����
// ���������� ���������� count_var � ����� ������ tvar
// ������ �������� VarOnList
int count_var;

// ������� ���������� � ������ ������ 1 ���� ������
int VarOnList(baserecord *pb, TScVar *ScVar, TClVar *ClVar, array *heap);

// ������� ���������� � ������� ������ 1 ���� ������
int VarOnFunc(baserecord *pb, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (pb->ident != isfunction) {
    return 1;  // ������
  }
  recordfunction *pf = (recordfunction *)pb;
  unsigned *ptrarg = heap->GetPunsigned(pf->ptrarg);
  for (unsigned i = 0; i < (unsigned)pf->narg; i++) {
    baserecord *tp = heap->GetPbaserecord(ptrarg[i]);
    switch (tp->ident) {
    case isfunction:
      if (VarOnFunc((baserecord *)tp, ScVar, ClVar, heap)) {
        return 1;  // ������
      }
      break;
    case islist:
      if (VarOnList((baserecord *)tp, ScVar, ClVar, heap)) {
        return 1;  // ������
      }
      break;
    case isvar: count_var++; break;
    }
  }
  return 0;
};

// ������� ���������� � ������ ������ 1 ���� ������
int VarOnList(baserecord *pb, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (pb->ident != islist) {
    return 1;
  }
  recordlist *pl = (recordlist *)pb;
  do {
    recordvar *pv = heap->GetPrecordvar(pl->head);
    switch (pv->ident) {
    case isfunction:
      if (VarOnFunc((baserecord *)pv, ScVar, ClVar, heap)) {
        return 1;  // ������
      }
      break;
    case isvar: count_var++; break;
    }
    pl = heap->GetPrecordlist(pl->link);
    unsigned char ident = pl->ident;
    ident = ident + 0;
  } while (pl->ident == islist);
  return 0;
};

int VarFromFunc(baserecord *pbr);
int VarFromList(baserecord *pbr);
int VarFromFunc(baserecord *pbr) {
  return 0;
}

// ���������� ����� ��� ��������� ���
//  �������� ���� � ������ bpt

unsigned target_number;

unsigned int prepare_target_from_list(unsigned term, TScVar *ScVar, TClVar *ClVar, array *heap);

unsigned int prepare_target_from_var(unsigned term, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!term || term == isnil) {
    return 1;
  }
  unsigned error = 0;
  recordvar *pv = heap->GetPrecordvar(term);
  if (pv->ident != isvar) {
    return 1;
  }
  unsigned frame = ClVar->frame2;
  if (!occur_term(&term, &frame, ClVar, heap)) {
    return 1;
  }
  if (term == isnil || !term) {
    return 1;
  }
  pv = heap->GetPrecordvar(term);
  switch (pv->ident) {
  case islist: {
    error = prepare_target_from_list(term, ScVar, ClVar, heap);
    if (error != 0) {
      return 1;
    }
    break;
  }
  case isvar: {
    error = prepare_target_from_var(term, ScVar, ClVar, heap);
    if (error != 0) {
      return 1;
    }
    break;
  }
  case issymbol:
  case isfunction: {
    ClVar->bpt[target_number++] = term;
  } break;
  default: {
    return 1;
  }
  }
  return 0;
}

unsigned int prepare_target_from_list(unsigned term, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!term || term == isnil) {
    return 1;
  }
  unsigned error = 0;
  recordlist *pl = heap->GetPrecordlist(term);
  if (pl->ident != islist) {
    return 1;
  }
  while (pl->ident == islist) {
    // ��������� ���� �� ������
    baserecord *tp = heap->GetPbaserecord(pl->head);
    switch (tp->ident) {
    case islist: {
      error = prepare_target_from_list(pl->head, ScVar, ClVar, heap);
      if (error != 0) {
        return 1;
      }
      break;
    }
    case isvar: {
      error = prepare_target_from_var(pl->head, ScVar, ClVar, heap);
      if (error != 0) {
        return 1;
      }
      break;
    }
    case issymbol:
    case isfunction: {
      ClVar->bpt[target_number++] = pl->head;
      break;
    }
    default: return 1;
    }
    pl = heap->GetPrecordlist(pl->link);
  }
  return 0;  // ok
}

unsigned int prepare_target(unsigned term, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!term || term == isnil) {
    return 1;
  }
  baserecord *pt = heap->GetPbaserecord(term);
  //    recordlist * pl=(recordlist *)&heap->heaps[term];
  if (!ClVar->bpt) {
    return 1;
  }
  target_number = 0;
  switch (pt->ident) {
  case islist: return prepare_target_from_list(term, ScVar, ClVar, heap);
  case isvar: return prepare_target_from_var(term, ScVar, ClVar, heap);
  default: return 1;
  }
  /*
          if (pl->ident!=islist)
                  return 1;
          return prepare_target_from_list(term);
  */
}

// 2006/10/24
void PrintFunction(recordfunction *prf, int Level, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintFunction: Level %d", Level);
  pldout(pBuf);
  baserecord *pbr = heap->GetPbaserecord(prf->func);
  sprintf(p, "func: (%d)", prf->func);
  pldout(pBuf);
  PrintTerm(pbr, Level + 1, ScVar, ClVar, heap);
  unsigned *ptrarg = heap->GetPunsigned(prf->ptrarg);
  sprintf(p, "arguments: (%d)", prf->narg);
  pldout(pBuf);
  for (int i = 0; i < static_cast<int>(prf->narg); i++) {
    sprintf(p, "argument %d (%d)", i, ptrarg[i]);
    pldout(pBuf);
    baserecord *pbr = heap->GetPbaserecord(ptrarg[i]);
    PrintTerm(pbr, Level + 1, ScVar, ClVar, heap);
  }
}

void PrintRecordsconst(recordsconst *pbr, int Level, TScVar *Scvar, TClVar *ClVar, array *heap) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "Printsconst: Level %d", Level);
  char *pc = heap->GetPchar(pbr->ptrsymb);
  sprintf(p, "sconst: ");
  int len = strlen(p);
  strncpy(p + len, pc, pbr->length);
  *(p + len + pbr->length) = 0;
  pldout(pBuf);
}

void PrintVar(recordvar *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintVar: Level %d", Level);
  pldout(pBuf);
  char *pc = heap->GetPchar(pbr->ptrsymb);
  sprintf(p, "Name: ");
  int len = strlen(p);
  strncpy(p + len, pc, pbr->length);
  *(p + len + pbr->length) = 0;
  pldout(pBuf);
  sprintf(p, "num of clause: %d", pbr->num);
  pldout(pBuf);
}

void PrintInteger(recordinteger *pbr, int Level) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintInteger: Level %d", Level);
  pldout(pBuf);
  sprintf(p, "Value: %s", pbr->value.str().c_str());
  pldout(pBuf);
}

void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintTerm: Level %d", Level);
  pldout(pBuf);
  int ident = pbr->ident;
  switch (ident) {
  case isfunction: PrintFunction((recordfunction *)pbr, Level + 1, ScVar, ClVar, heap); return;
  case issymbol: PrintRecordsconst((recordsconst *)pbr, Level + 1, ScVar, ClVar, heap); return;
  case isvar: PrintVar((recordvar *)pbr, Level + 1, ScVar, ClVar, heap); return;
  case isinteger: PrintInteger((recordinteger *)pbr, Level + 1); return;
  case islist: PrintList((recordlist *)pbr, Level + 1, ScVar, ClVar, heap); return;
  default: sprintf(pBuf, "unknown id of term: %d", ident); pldout(pBuf);
  }
}

void PrintList(recordlist *pl, int Level, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintList: Level %d", Level);
  pldout(pBuf);
  while (pl->ident == islist) {
    baserecord *pb = heap->GetPbaserecord(pl->head);
    PrintTerm(pb, Level + 1, ScVar, ClVar, heap);
    pl = heap->GetPrecordlist(pl->link);
  }
}
recordfunction *FindFuncFromFunc(baserecord *pbr) {
  recordfunction *rf = (recordfunction *)0;
  return rf;
}

recordfunction *FindFuncFromTerm(baserecord *pbr) {
  pldout(const_cast<char *>("FindFundFromterm"));
  int ident = pbr->ident;
  switch (pbr->ident) {
  case isfunction: {
    return FindFuncFromFunc(pbr);
  } break;
  }
  return 0;
}

unsigned GetConstTerm(unsigned Term, unsigned Frame, TClVar *ClVar, array *heap) {
  auto _Term = Term;
  auto _Frame = Frame;
  do {
    baserecord *br = heap->GetPbaserecord(_Term);
    int ident = br->ident;
    switch (ident) {
    case isvar: {
      unsigned j;
      unsigned _PrevTerm = _Term;
      bound(&_Term, &_Frame, &j, ClVar, heap);
      if (_Term == isnil) {
        return _PrevTerm;
      }
    } break;
    case islist:
    case issymbol:
    case isinteger:
    case isfloat:
    case isstring: return _Term;
    default: return 0;
    }
  } while (1);
}

int GetVarsFromFunction(recordvar *Vars[], int VarCount, recordfunction *pf, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (pf->ident != isfunction) {
    return -1;  // ������
  }
  int Count = 0;
  unsigned *ptrarg = heap->GetPunsigned(pf->ptrarg);
  for (unsigned i = 0; i < (unsigned)pf->narg; i++) {
    baserecord *tp = heap->GetPbaserecord(ptrarg[i]);
    switch (tp->ident) {
    case isvar: Vars[VarCount + Count++] = (recordvar *)tp; break;
    case isinteger:
      break;
      /*
      case isfunction:
              if (VarOnFunc((baserecord*)tp))
                      return 1;//������
              break;
      case islist:
              if (VarOnList((baserecord*)tp))
                      return 1;//������
              break;
      */
    default:
      char _Buf[255];
      sprintf(_Buf, "GetVarCountFromFunction: unknown term %d", tp->ident);
      pldout(_Buf);
      return -1;
    }
  }
  return Count;
};

int GetVarCountFromClause(recordclause *rc, TScVar *ScVar, TClVar *ClVar, array *heap) {
  int VarCount = 0;
  if (rc->ident != isclause) {
    pldout(const_cast<char *>("No clause in func GetVarCountFromCluse"));
    return -1;
  }

  unsigned *target = heap->GetPunsigned(rc->ptrtarget);
  int i = 0;
  recordvar *Vars[128];
  while (*(target + i)) {
    int Count = 0;
    baserecord *br = heap->GetPbaserecord(*(target + i));
    switch (br->ident) {
    case isfunction: {
      Count = GetVarsFromFunction(Vars, VarCount, (recordfunction *)br, ScVar, ClVar, heap);
      if (Count < 0)
        return -1;
      VarCount += Count;
    } break;
    default:
      char _Buf[255];
      sprintf(_Buf, "GetVarCountFromClause: unknown term %d", br->ident);
      pldout(_Buf);
      return -1;
    }
    i++;
  }
  int newCount = VarCount;
  /*
  for (int j = 0; VarCount - 1; j++)
  {
          for (int k = j + 1; k < VarCount; k++)
          {
                  recordvar * var1 = Vars[j];
                  recordvar * var2 = Vars[k];
                  if (var1->ptrsymb == var2->ptrsymb && var1->num == var2->num)
                          newCount--;
          }
  }
  */
  return VarCount;
}

// ���
PredicateState prassrt(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto old_index = heap->last;
  unsigned error = 0;
  baserecord *tp;
  recordsconst *ps;
  recordlist *pl;
  recordfunction *pf;
  recordclause *pfirstclause;  // ������ ����������� ������� ����
  recordclause *pclause;       // ����������� �� ����� �������� ���������� ���������
  size_t index;
  unsigned int *ptarget;  // ��������� �� ������ � ������
  unsigned nvar = 0;      // ����� ���������� � �����������(���� �����)

  unsigned ntarget = 1;  // ����� ����� � ����� �����������
  switch (sw) {
  case 427:
  case 927:
  case 827:  // _,[],int  //������������� ����������� ��� ����
             // � ����� ����������� ���������� �� ����� ����
  {
    auto m = maxarity;
    tp = heap->GetPbaserecord(ScVar->goal[maxarity]);
    if (tp->ident != isfunction)  // ����� ���� ���� �������� ��� ����������,������������������ �������� �������� ���
                                  // ����� �������� �������
    {
      pldout(const_cast<char *>("������� '���': ���1 �� �������"));
      return PredicateState::Error;  // 1;
    }
    recordfunction *prf = (recordfunction *)tp;
    auto ArgIndex = heap->append<unsigned>(0, prf->narg);
    unsigned int *parg = heap->GetPunsigned(ArgIndex);
    unsigned *prf_arg = heap->GetPunsigned(prf->ptrarg);
    for (int i = 0; i < static_cast<int>(prf->narg); i++) {
      // ������� ��������� �� ���������� �������.
      unsigned Arg = GetConstTerm(*(prf_arg + i), ClVar->frame2, ClVar, heap);
      *(parg + i) = Arg;
    }
    auto findex = heap->append(recordfunction(prf->narg, prf->func, ArgIndex));
    int id = tp->ident;
    index = heap->append<unsigned>(0, 2);
    ptarget = heap->GetPunsigned(index);
    ptarget[0] = findex;
    ptarget[1] = NULL;
    index = heap->append(recordclause((unsigned)isclause, (unsigned)NULL, 0, findex, index));
  } break;
  case 437:
  case 937:
  case 837:  //_,list,int
  {
    tp = heap->GetPbaserecord(ScVar->goal[maxarity + 1]);
    // ����� ���������� ����� ����������
    count_var = 0;
    if (VarOnList(tp, ScVar, ClVar, heap) != 0)  // ������� ����� ���������� � ����� �����������
    {
      pldout(const_cast<char *>("������ ��� �������� ����� ����������"));
      return PredicateState::Error;  // 1;
    }
    // TODO: � ��� ��� ���������?
    nvar += count_var;  // ��� ����� ���������� ����� ����� � ������
                        //             out("���������� ����������");
                        // ���������� �����
    auto _maxarity = maxarity;
    error = prepare_target(ScVar->goal[maxarity + 1], ScVar, ClVar, heap);
    if (error) {
      pldout(const_cast<char *>("������ ��� ������ ����� ��� ���"));
      return PredicateState::Error;  // 1;
    }

    ntarget = target_number + 1;
    ptarget = new unsigned[ntarget + 1];
    if (!ptarget) {
      outerror(ErrorCode::NotEnoughFreeMemory);  // 2
      return PredicateState::Error;              // 1;
    }

    pl = heap->GetPrecordlist(ScVar->goal[maxarity + 1]);
    int i = 0;
    ptarget[i++] = ScVar->goal[maxarity];  // head
    while (i < (int)ntarget) {
      ptarget[i] = ClVar->bpt[i - 1];
      i++;
    }
    ptarget[i] = (unsigned)NULL;
    i = 0;
    while (ptarget[i]) {
      recordfunction *prf = heap->GetPrecordfunction(ptarget[i]);
      auto ArgIndex = heap->append<unsigned>(0, prf->narg);
      unsigned *arg = heap->GetPunsigned(ArgIndex);
      unsigned ptrarg = prf->ptrarg;
      unsigned *prf_arg = heap->GetPunsigned(prf->ptrarg);
      for (int j = 0; j < static_cast<int>(prf->narg); j++) {
        // ������� ��������� �� ���������� �������.
        unsigned Arg = GetConstTerm(*(prf_arg + j), ClVar->frame2, ClVar, heap);
        Arg = 0;
        if (Arg) { /*
                           recordvar * prv = (recordvar *)&heap->heaps[Arg];
                           if (prv->ident == isvar)
                           {
                                   recordvar newvar(prv->ptrsymb, prv->length,
                                                                                                           prv->num);
                                   unsigned vindex = heap->apend(&newvar, sizeof(recordvar));
                                   arg[j] = vindex;
                           }
                           else
                           */
          arg[j] = Arg;
        } else {
          arg[j] = *(prf_arg + j);
        }
      }
      auto findex = heap->append(recordfunction(prf->narg, prf->func, ArgIndex));
      // if (!unify(ptarget[i], findex, frame2, frame2))
      //     return 1;
      ptarget[i] = findex;
      i++;
    }
    index = heap->append<unsigned>(0, ntarget + 1);
    memcpy(heap->GetPunsigned(index), ptarget, sizeof(unsigned) * (ntarget + 1));
    recordclause rc(isclause,
                    (unsigned)NULL,
                    5,  // nvar,
                    ScVar->goal[maxarity],
                    index);
    int Count = GetVarCountFromClause(&rc, ScVar, ClVar, heap);
    if (Count < 0) {
      pldout(const_cast<char *>("Var count calculation in new clause failure"));
      return PredicateState::No;  // 5;
    }
    rc.nvars = Count;
    index = heap->append(rc);
    break;
  }
  default: {
    pldout(const_cast<char *>("������������ ��� ���������� � '���'"));
    return PredicateState::Error;  // 1;
  }
  }
  if (!index) {
    // heap->last = old_index;
    heap->cleanUp(old_index);
    outerror(ErrorCode::TooLongList);  // 44
    return PredicateState::No;         // 5;
  }
  // ������� � ������� �����������
  IntegerType number = occ(2, ScVar, ClVar, heap);  // ���� ������� �������� �����
  // ������ ������ �����������
  tp = heap->GetPbaserecord(ScVar->goal[maxarity]);
  if (tp->ident == issymbol) {
    ps = (recordsconst *)tp;
  } else {
    if (tp->ident != isfunction) {
      outerror(ErrorCode::UnknownError);  // 24
      return PredicateState::Error;       // 1;
    }
    pf = (recordfunction *)tp;
    recordfunction *_pf = pf;
    ps = heap->GetPrecordsconst(pf->func);
    recordsconst *_ps = ps;
    // ������ �������� ������� ���������� � �����������
  }
  if (ps->begin != NULL && ps->begin != isnil) {
    pfirstclause = heap->GetPrecordclause(ps->begin);
  } else {
    pfirstclause = NULL;  // ��� ����� �����������
  }
  recordclause *newcl = heap->GetPrecordclause(index);
  if (number < 2 || !pfirstclause) {
    // �������� ����� ����������� ������ � ������
    newcl->next = ps->begin;
    ps->begin = index;  // ����� ������
  } else {
    // �������� ������� � ������� ��� �����
    int i = 1;
    pclause = pfirstclause;
    while (i < number - 1 && pclause->next && pclause->next != isnil)
      if (pclause->next && pclause->next != isnil) {
        pclause = heap->GetPrecordclause(pclause->next);
        i++;
      }
    if (i == number - 1) {
      newcl->next = pclause->next;
    }
    pclause->next = index;
  }
  return PredicateState::Yes;  // 3;
}

// ��������
PredicateState pradd(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool fl = false;  // ���� ����� true �� ��������� ��� ����� �����
  int i;
  for (i = 0; i < 3; i++) {
    if (ScVar->goal[i] == 6) {
      fl = true;
    }
  }
  for (i = 0; i < 3; i++) {
    switch (ScVar->goal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true) {
        f[i] = (FloatType)occ(i, ScVar, ClVar, heap);
        switch (i) {
        case 0: sw -= 100; break;
        case 1: sw -= 10; break;
        case 2: sw -= 1;
        }
      } else
        l[i] = occ(i, ScVar, ClVar, heap);
    }
  }
  switch (sw) {
  case 777: return (l[0] + l[1] == l[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // ��� �����  ���
  case 666: return (f[0] + f[1] == f[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // ��� ������� ffff
  case 775: return zap1(l[0] + l[1], 3, ScVar, ClVar, heap);
  case 665: return zap1f(f[0] + f[1], 3, ScVar, ClVar, heap);
  case 577: return zap1(l[2] - l[1], 1, ScVar, ClVar, heap);
  case 566: return zap1f(f[2] - f[1], 1, ScVar, ClVar, heap);
  case 757: return zap1(l[2] - l[0], 2, ScVar, ClVar, heap);
  case 656: return zap1f(f[2] / f[0], 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::FailedAddition);  // 31
  return PredicateState::Error;         // 1;
}

// ���������   � goal[0].. 1 .. �������� ��������
PredicateState prmul3(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[3];
  FloatType f[3];
  bool fl = false;  // ���� ����� true �� ��������� ��� ����� �����
  int i;
  for (i = 0; i < 3; i++) {
    if (ScVar->goal[i] == 6)
      fl = true;
  }
  // for (i = 0; i < 4; i++)
  for (i = 0; i < 3; i++) {
    switch (ScVar->goal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true) {
        f[i] = (FloatType)occ(i, ScVar, ClVar, heap);
        switch (i) {
        case 0: sw -= 100; break;
        case 1: sw -= 10; break;
        case 2: sw -= 1;
        }
      } else
        l[i] = occ(i, ScVar, ClVar, heap);
    }
  }
  switch (sw) {
  case 777: return (l[0] * l[1] == l[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // ��� �����  ���
  case 666: return (f[0] * f[1] == f[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // ��� ������� ffff
  case 775: return zap1(l[0] * l[1], 3, ScVar, ClVar, heap);
  case 665: return zap1f(f[0] * f[1], 3, ScVar, ClVar, heap);
  case 577:
    if (l[1] == 0) {
      return PredicateState::No;  // 5;
    }
    if (l[2] % l[1] == 0) {
      return zap1(l[2] / l[1], 1, ScVar, ClVar, heap);
    }
    f[2] = (FloatType)l[1];
    f[1] = (FloatType)l[1];
  case 566:
    if (f[1] == 0) {
      return PredicateState::No;  // 5;
    }
    return zap1f(f[2] / f[1], 1, ScVar, ClVar, heap);
  case 757:
    if (l[0] == 0) {
      return PredicateState::No;  // 5;
    }
    if (l[2] % l[0] == 0)
      return zap1(l[2] / l[0], 2, ScVar, ClVar, heap);
    f[0] = (FloatType)l[0];
    f[2] = (FloatType)l[2];
  case 656:
    if (f[0] == 0) {
      return PredicateState::No;  // 5;
    }
    return zap1f(f[2] / f[0], 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::FailedMultiplicationInvalidArguments);  // 30
  return PredicateState::Error;                               // 1; // r_t_e �� ��� ���� ���������
}

PredicateState argthree(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned sw;
  conarg(3, ClVar->head, ScVar, ClVar, heap);
  sw = 100 * ScVar->goal[0] + 10 * ScVar->goal[1] + ScVar->goal[2];
  switch (name) {
  case hpmul: return prmul3(sw, ScVar, ClVar, heap);     // ���������
  case hpset: return prset(sw, ScVar, ClVar, heap);      // �����
  case hpapp: return prapp(sw, ScVar, ClVar, heap);      // ����
  case hpassrt: return prassrt(sw, ScVar, ClVar, heap);  // ���
  case hpadd: return pradd(sw, ScVar, ClVar, heap);      // ��������
  case hppaint: return prpaint(sw, ScVar, ClVar, heap);  // ��������
  case hprand: return prrandom(sw, ScVar, ClVar, heap);  // ����
  }
  // return 0;
  return PredicateState::Error;
}

// ���������   � goal[0].. 1 .. �������� ��������
PredicateState prmul(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool fl = false;  // ���� ����� true �� ��������� ��� ����� �����
  int i;
  for (i = 0; i < 4; i++) {
    if (ScVar->goal[i] == 6)
      fl = true;
  }
  for (i = 0; i < 4; i++) {
    switch (ScVar->goal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true) {
        f[i] = (FloatType)occ(i, ScVar, ClVar, heap);
        switch (i) {
        case 0: sw -= 1000; break;
        case 1: sw -= 100; break;
        case 2: sw -= 10; break;
        case 3: sw -= 1;
        }
      } else
        l[i] = occ(i, ScVar, ClVar, heap);
    }
  }
  /* switch(sw)     //��� ���� �� �������� float
   {case 7777:return (occ(0)*occ(1)+occ(2)==occ(3))?3:5; //��� �����  ����
    case 6666:return (occf(0)*occf(1)+occf(2)==occf(3))?3:5; //��� ������� ffff
    case 5777:if (occ(1))
                          {int work;
                           if ((work=occ(3)-occ(2))%occ(1)!=0) return 5;
                            return zap1(work/occ(1),1);
                          }break;

    case 7577:if (occ(0))                     //����
                          {int work;
                          if ((work=occ(3)-occ(2)) % occ(0)!=0) return 5;
                          return zap1(work/occ(0),2);}break;

    case 7757:return zap1(occ(3)-occ(0)*occ(1),3);    //����
    case 7775:return zap1(occ(0)*occ(1)+occ(2),4);
    case 7557:if (occ(0)) return zap2(occ(3)%occ(0),occ(3)/occ(0),3,2);break;
    case 5757:if (occ(1)) return zap2(occ(3)%occ(1),occ(3)/occ(1),3,1);break;

   }*/

  switch (sw) {
  case 7777: return (l[0] * l[1] + l[2] == l[3]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // ��� �����  ����
  case 6666: return (f[0] * f[1] + f[2] == f[3]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // ��� ������� ffff
  case 5777:
    if (l[1]) {
      IntegerType work;
      if ((work = l[3] - l[2]) % l[1] != 0) {
        return PredicateState::No;  // 5;
      }
      return zap1(work / l[1], 1, ScVar, ClVar, heap);
    }
    break;

  case 7577:
    if (l[0])  // ����
    {
      IntegerType work;
      if ((work = l[3] - l[2]) % l[0] != 0) {
        return PredicateState::No;  // 5;
      }
      return zap1(work / l[0], 2, ScVar, ClVar, heap);
    }
    break;

  case 7757: return zap1(l[3] - l[0] * l[1], 3, ScVar, ClVar, heap);   // ����
  case 6656: return zap1f(f[3] - f[0] * f[1], 3, ScVar, ClVar, heap);  // ����
  case 7775: return zap1(l[0] * l[1] + l[2], 4, ScVar, ClVar, heap);
  case 6665: return zap1f(f[0] * f[1] + f[2], 4, ScVar, ClVar, heap);
  case 7557:
    if (l[0]) {
      return zap2(l[3] % l[0], l[3] / l[0], 3, 2, ScVar, ClVar, heap);
    }
    break;
  case 5757:
    if (l[1]) {
      return zap2(l[3] % l[1], l[3] / l[1], 3, 1, ScVar, ClVar, heap);
    }
    break;
  case 6767:                                                                        // if (f[1]) return zap2f(f[3]%f[1],f[3]/f[1],3,1);break;
  case 6556:                                                                        // if (f[0]) return zap2(f[3]%f[0],f[3]/f[0],3,2);break;
  case 5666:                                                                        /*if (f[1])
                                                                                          {float work;
                                                                                           if ((work=f[3]-f[2])%f[1]!=0) return 5;
                                                                                            return zap1f(work/f[1],1);
                                                                                          }break; */
  case 6566:                                                                        /*if (f[0])                     //����
                                                                                          {float work;
                                                                                          if ((work=f[3]-f[2]) % f[0]!=0) return 5;
                                                                                          return zap1f(work/f[0],2);}break;*/
    outerror(ErrorCode::FailedMultiplicationIntegerDivisionOfSubstancesOfNumbers);  // 29
    return PredicateState::Error;                                                   // 1;
  }
  outerror(ErrorCode::FailedMultiplicationInvalidArguments);  // 30
  return PredicateState::Error;                               // 1; // r_t_e �� ��� ���� ���������
}

// ����������
PredicateState prcircl(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType x1, x2, y1, y2, r, color;

  switch (sw) {
  case 7777: {
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = occ(1, ScVar, ClVar, heap);
    r = occ(2, ScVar, ClVar, heap);
    color = occ(3, ScVar, ClVar, heap);
    // x2 = y2 = 0;
    // Ellipse(x1, y1, x2, y2, color);
    Ellipse(x1.convert_to<long long>(), y1.convert_to<long long>(), r.convert_to<long long>(), r.convert_to<long long>(), color.convert_to<unsigned>());
  } break;

  case 7577: {
    color = occ(3, ScVar, ClVar, heap);
    r = occ(2, ScVar, ClVar, heap);
    x1 = occ(0, ScVar, ClVar, heap) - r;
    y1 = 0;
    x2 = x1 + r + r;
    // y2 = maxgry;
    vertical(x1.convert_to<long long>(), y1.convert_to<long long>(), x2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;
  case 5777: {
    color = occ(3, ScVar, ClVar, heap);
    r = occ(2, ScVar, ClVar, heap);
    x1 = 0;
    y1 = occ(1, ScVar, ClVar, heap) - r;
    // x2 = maxgrx;
    y2 = y1 + r + r;
    horisontal(x1.convert_to<long long>(), y1.convert_to<long long>(), y2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;
  case 7757:
  case 7557:
  case 5757:
  case 5557:
  case 5577: {
    color = occ(3, ScVar, ClVar, heap);
    // x1 = 0; y1 = 0;
    // x2 = maxgrx; y2 = maxgry;
    ClearView(color.convert_to<unsigned>());
  } break;
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e
  }
  }
  return PredicateState::Yes;  // 3;
}

// ��������
PredicateState prpaint(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType x, y, color;
  if (sw != 777) {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1;
  }
  x = occ(0, ScVar, ClVar, heap);
  y = occ(1, ScVar, ClVar, heap);
  color = occ(2, ScVar, ClVar, heap);
  FloodFill(x.convert_to<long long>(), y.convert_to<long long>(), color.convert_to<unsigned>());
  return PredicateState::Yes;  // 3;
}

// �����
PredicateState prcopy(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char str1[maxlinelen], str2[maxlinelen];
  IntegerType i1, i2, i3, i4;
  switch (sw) {
  case 4775:
  case 9775:  // str int int var
    occ_line(0, str1, ScVar, ClVar, heap);
    i1 = occ(1, ScVar, ClVar, heap);
    i2 = occ(2, ScVar, ClVar, heap);
    i3 = strlen(str1);
    if (i1 > 0 && i2 >= 0 && i3 >= i1 + i2 - 1) {
      str1[(i1 + i2 - 1).convert_to<size_t>()] = NULL;
      // char *s=newStr(&str1[i1-1]);
      return zap3(&str1[(i1 - 1).convert_to<size_t>()], 4, ScVar, ClVar, heap);
    }
    break;
  case 4774:
  case 4779:
  case 9774:
  case 9779:  // str int int str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = occ(1, ScVar, ClVar, heap);
    i2 = occ(2, ScVar, ClVar, heap);
    i3 = strlen(str1);
    i4 = strlen(str2);
    return (i1 > 0 && i2 >= 0 && i2 == i4 && i3 >= i1 + i4 - 1 && !strncmp(&str1[(i1 - 1).convert_to<size_t>()], str2, i2.convert_to<size_t>()))
           ? PredicateState::Yes
           : PredicateState::No;  // 3 : 5;

  case 4574:
  case 4579:
  case 9574:
  case 9579:  // str var int str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = occ(2, ScVar, ClVar, heap);
    i2 = strlen(str1);
    i3 = strlen(str2);
    if (i1 >= 0 && i2 >= i1 + i3 - 1 && i3 == i1) {
      for (i4 = i2 - i3; i4 >= 0 && strncmp(&str1[i4.convert_to<size_t>()], str2, i3.convert_to<size_t>()); i4--)
        ;
      if (i4 >= 0)
        return zap1(i4 + 1, 2, ScVar, ClVar, heap);
    }
    break;
  case 4554:
  case 4559:
  case 9554:
  case 9559:  // str var var str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = strlen(str1);
    i2 = strlen(str2);
    if (i1 >= i2) {
      int result = 1;
      for (i3 = i1 - i2; i3 >= 0 && result != 0; i3--)
        result = strncmp(&str1[i3.convert_to<size_t>()], str2, i2.convert_to<size_t>());
      if (!result) {
        return zap2(i3 + 2, i2, 2, 3, ScVar, ClVar, heap);
      }
    }
    break;
  case 4754:
  case 4759:
  case 9754:
  case 9759:  // str int var str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = strlen(str1);
    i2 = strlen(str2);
    i3 = occ(1, ScVar, ClVar, heap);
    if ((i3 > 0) && (i1 >= i2 + i3 - 1) && (!strncmp(&str1[(i3 - 1).convert_to<size_t>()], str2, i2.convert_to<size_t>())))
      return zap1(i2, 3, ScVar, ClVar, heap);
    break;
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1;                           // r_t_e
  }
  }
  return PredicateState::No;  // 5;
}

// �����
PredicateState prclaus(unsigned sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  baserecord *tp;
  unsigned err = 0;
  size_t index = 0;
  // ���� ���������� ������� �� �����������
  unsigned sw1;  // ������
  unsigned sw2;  // ������
  unsigned sw3;  // ������
  unsigned sw4 = sw;

  // ������ ����� ����������

  sw1 = sw4 / 1000;
  sw4 = sw4 - sw1 * 1000;
  sw2 = sw4 / 100;
  sw4 = sw4 - sw2 * 100;
  sw3 = sw4 / 10;
  sw4 = sw4 - sw3 * 10;

  // �������� �����
  if (sw1 != 4 && sw1 != 9)
    err = 1;  // symb/str
  if (sw2 != 7)
    err = 2;  // int
  if (sw3 != 5 && sw3 != 4 && sw3 != 8)
    err = 3;  // var/sybm/func
  if (sw4 != 5 && sw4 != 2 && sw4 != 3)
    err = 4;  // var/list/emptylist
  if (err != 0) {
    pldout(const_cast<char *>("�������� ��� ���������� � '�����'."));  // ��� ������ ������� ����� ���������
    return PredicateState::No;                                         // 5;
  }

  // ������ �������� �����������
  IntegerType i = occ(1, ScVar, ClVar, heap);  // ����� ��������� �����
  recordsconst *ps = heap->GetPrecordsconst(ScVar->goal[maxarity]);
  if (ps->begin == isnil || ps->begin == NULL) {
    return PredicateState::No;  // 5;  // ��� �����������
  }
  recordclause *pc = heap->GetPrecordclause(ps->begin);
  IntegerType w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w++) {
    pc = heap->GetPrecordclause(pc->next);
  }
  if (w != i) {
    return PredicateState::No;  // 5;  // ����� � ������ ������� �����������
  }

  // ����� ������ �����������
  recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
  // ��������� ������� �����
  unsigned *ptr = heap->GetPunsigned(pf->ptrarg);
  // ���� ���������� �����������
  unsigned *p = heap->GetPunsigned(pc->ptrtarget);
  // �������� ���������
  auto oldindex = heap->last;
  ClVar->oldtptr = ClVar->tptr;
  ClVar->frame1 = ClVar->oldsvptr = ClVar->svptr;
  ClVar->svptr = ClVar->frame1 + pc->nvars;  // ��� ����� ���������� � �����
  if (ClVar->svptr > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
    pldout(const_cast<char *>("�� ���������� ������ � ����� ����������"));
    return PredicateState::No;  // 5;
  }
  // ���������� ������ ������� � ���������� ��� ...
  if (!unify(p[0], ptr[2], ClVar->frame1, ClVar->frame2, ClVar, heap)) {
    ClVar->svptr = ClVar->oldsvptr;
    // heap->last = oldindex;  //??????
    heap->cleanUp(oldindex);
    zero(ClVar);
    return PredicateState::No;  // 5;
  }

  // ������� ������ �� ����� ���������� �����������
  tp = heap->GetPbaserecord(pc->head);
  size_t tlist = 0;
  switch (tp->ident) {
  case isfunction: {
    recordfunction *rf = (recordfunction *)tp;
    auto oldindex = heap->last;
    recordlist pl(rf->func, oldindex + sizeof(recordlist));
    for (size_t i = 1; p[i] != 0 && p[i] != isnil && !err; ++i) {
      pl.head = p[i];
      pl.link = heap->last + sizeof(recordlist);
      index = heap->append(pl);
      if (tlist == 0) {
        tlist = index;
      }
    }
  }
  case issymbol: {
    if (!err) {
      index = heap->append(recordemptylist());
      if (tlist == 0) {
        tlist = index;
      }
    }
    if (!err) {
      if (!unify(tlist, ptr[3], ClVar->frame1, ClVar->frame2, ClVar, heap)) {
        ClVar->svptr = ClVar->oldsvptr;
        // heap->last = oldindex;  //?????
        heap->cleanUp(oldindex);
        zero(ClVar);
        return PredicateState::No;  // 5;
      }
      if (sw4 == 2 || sw4 == 3) {
        ClVar->svptr = ClVar->oldsvptr;
        // heap->last = oldindex;  //?????
        heap->cleanUp(oldindex);
        zero(ClVar);
      }

      return PredicateState::Yes;  // 3;  // ������� ����������
    }
    break;
  }
  }
  return PredicateState::No;  // 5;
}

PredicateState argfour(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned sw;
  conarg(4, ClVar->head, ScVar, ClVar, heap);
  sw = ScVar->goal[0] * 1000 + ScVar->goal[1] * 100 + ScVar->goal[2] * 10 + ScVar->goal[3];
  switch (name) {
  case hpmul: return prmul(sw, ScVar, ClVar, heap);      // ���������
  case hpcircl: return prcircl(sw, ScVar, ClVar, heap);  // ����������
  case hpcopy: return prcopy(sw, ScVar, ClVar, heap);    // �����
  case hpclaus: return prclaus(sw, ScVar, ClVar, heap);  // �����
  }
  // return 0;
  return PredicateState::Error;  // 1;
}

// �����
PredicateState prger(unsigned long sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  // int xy;
  IntegerType x1, x2, y1, y2, color;
  switch (sw) {
  case 77777: {
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = occ(1, ScVar, ClVar, heap);
    x2 = occ(2, ScVar, ClVar, heap);
    y2 = occ(3, ScVar, ClVar, heap);
    color = occ(4, ScVar, ClVar, heap);
    MoveTo_LineTo(x1.convert_to<long long>(), y1.convert_to<long long>(), x2.convert_to<long long>(), y2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;
  case 77757:
    vertical(occ(0, ScVar, ClVar, heap).convert_to<long long>(),
             occ(1, ScVar, ClVar, heap).convert_to<long long>(),
             occ(2, ScVar, ClVar, heap).convert_to<long long>(),
             occ(4, ScVar, ClVar, heap).convert_to<long long>());
    break;
  case 77577:
    horisontal(occ(0, ScVar, ClVar, heap).convert_to<long long>(),
               occ(1, ScVar, ClVar, heap).convert_to<long long>(),
               occ(3, ScVar, ClVar, heap).convert_to<long long>(),
               occ(4, ScVar, ClVar, heap).convert_to<long long>());
    break;
  case 75777:
    vertical(occ(2, ScVar, ClVar, heap).convert_to<long long>(),
             occ(3, ScVar, ClVar, heap).convert_to<long long>(),
             occ(0, ScVar, ClVar, heap).convert_to<long long>(),
             occ(4, ScVar, ClVar, heap).convert_to<long long>());
    break;
  case 57777:
    horisontal(occ(2, ScVar, ClVar, heap).convert_to<long long>(),
               occ(3, ScVar, ClVar, heap).convert_to<long long>(),
               occ(1, ScVar, ClVar, heap).convert_to<long long>(),
               occ(4, ScVar, ClVar, heap).convert_to<long long>());
    break;
    //=================

  case 55777:
  case 77557:
  case 75557:
  case 57557:
  case 55757:
  case 55577:
  case 55557: {
    color = occ(4, ScVar, ClVar, heap);
    x1 = 0;
    y1 = 0;
    x2 = maxgrx;
    y2 = maxgry;
    // SetPenAndBrushColor(color);
    Rectangle(x1.convert_to<long long>(), y1.convert_to<long long>(), x2.convert_to<long long>(), y2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;

  case 75757: {
    color = occ(4, ScVar, ClVar, heap);
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = 0;
    x2 = occ(2, ScVar, ClVar, heap);
    y2 = maxgry;
    // SetPenAndBrushColor(color);
    Rectangle(x1.convert_to<long long>(), y1.convert_to<long long>(), x2.convert_to<long long>(), y2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;

  case 57577: {
    color = occ(4, ScVar, ClVar, heap);
    x1 = 0;
    y1 = occ(1, ScVar, ClVar, heap);
    x2 = maxgrx;
    y2 = occ(3, ScVar, ClVar, heap);
    // SetPenAndBrushColor(color);
    Rectangle(x1.convert_to<long long>(), y1.convert_to<long long>(), x2.convert_to<long long>(), y2.convert_to<long long>(), color.convert_to<unsigned>());
  } break;
  case 75577: {
    vertical(0,
             occ(3, ScVar, ClVar, heap).convert_to<long long>(),
             occ(0, ScVar, ClVar, heap).convert_to<long long>(),
             occ(4, ScVar, ClVar, heap).convert_to<long long>());
    vertical(maxgrx,
             occ(3, ScVar, ClVar, heap).convert_to<long long>(),
             occ(0, ScVar, ClVar, heap).convert_to<long long>(),
             occ(4, ScVar, ClVar, heap).convert_to<long long>());
  } break;
  case 57757: {
    vertical(0,
             occ(1, ScVar, ClVar, heap).convert_to<long long>(),
             occ(2, ScVar, ClVar, heap).convert_to<long long>(),
             occ(4, ScVar, ClVar, heap).convert_to<long long>());
    vertical(maxgrx,
             occ(1, ScVar, ClVar, heap).convert_to<long long>(),
             occ(2, ScVar, ClVar, heap).convert_to<long long>(),
             occ(4, ScVar, ClVar, heap).convert_to<long long>());
  } break;
  default: {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // R_T_E
  }
  }
  return PredicateState::Yes;  // 3;
}

PredicateState argfive(unsigned name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  unsigned long sw;
  conarg(5, ClVar->head, ScVar, ClVar, heap);
  sw = (unsigned long)(10000l * ScVar->goal[0]);
  sw += 1000 * ScVar->goal[1];
  sw += 100 * ScVar->goal[2];
  sw += 10 * ScVar->goal[3];
  sw += ScVar->goal[4];
  switch (name) {
  case hpline: return prger(sw, ScVar, ClVar, heap);  // �����
  }
  return PredicateState::Error;  // 1;
}

bool bpred(unsigned name, unsigned narg) {
  switch (name) {
  case hpquiet:
  case hpfail:
  case hptrace:
  case hpnottr:
  case hpcut: return (narg == 0) ? true : false;  // 0

  case hp_int:
  case hp_float:
  case hpcall:
  case hpiocod:
  case hpsee:
  case hptell:
  case hpvar:
  case hpint:
  case hpfloat:
  case hpsym:
  case hplst:
  case hpwait: return (narg == 1) ? true : false;  // 1

  case hprdint:
  case hprdfloat:
  case hprdsym:
  case hprdstr: return (narg == 1 || narg == 2) ? true : false;

  case hprand: return (narg == 1 || narg == 3) ? true : false;

  case hpgt:
  case hpstint:
  case hpstfloat:
  case hpstlst:
  case hpintfloat:
  case hplettr:
  case hpdigit:
  case hpterm:
  case hpdel:
  case hpskol: return (narg == 2) ? true : false;  // 2

  case hpadd:
  case hpset:
  case hpapp:
  case hppaint:
  case hpassrt: return (narg == 3) ? true : false;  // 3

  case hpmul: return (narg == 3 || narg == 4) ? true : false;
  case hpcircl:
  case hpcopy:
  case hpclaus: return (narg == 4) ? true : false;  // 4

  case hpline: return (narg == 5) ? true : false;  // 5
  }
  return false;  // ������ ���� �����
}

//=========== ������� ����� ==============
int Inputstring(char *buf, int size, char *caption) {
  int err = 0;
  char *pCaption = const_cast<char *>("������� ������");
  if (caption)
    pCaption = caption;
  int _err = InputStringFromDialog(buf, size, pCaption, true);
  if (!_err) {
    // pldout(buf);
  } else {
    err = 1;
  }
  return err;
}

int Inputline(char *buf, int size, char *caption) {
  int err = 0;
  char *pCaption = const_cast<char *>("������� ������");
  if (caption)
    pCaption = caption;
  int _err = InputStringFromDialog(buf, size, pCaption, false);
  if (!_err) {
    // pldout(buf);
  } else {
    err = 1;
  }
  return err;
}

int InputSymbol(char *c) {
  int err = 0;
  char *pCaption = const_cast<char *>("������� ������");
  char Buf[2];
  err = InputStringFromDialog(Buf, sizeof(Buf), pCaption, true);
  if (!err) {
    Buf[1] = 0;
    *c = Buf[0];
    // pldout(Buf);
    return 0;
  }
  return 1;
}

int InputInt(IntegerType *n, const char *caption) {
  char Buf[255]{};
  int err = InputStringFromDialog(Buf, sizeof(Buf), const_cast<char *>(caption), true);
  try {
    // ����� ����� from_chars ��� atoll
    *n = std::stoll(std::string(Buf));
  } catch (...) {
    err = 1;
  }
  return err;
}

int InputFloat(FloatType *n, const char *caption) {
  char Buf[255]{};
  int err = InputStringFromDialog(Buf, sizeof(Buf), const_cast<char *>(caption), true);
  try {
    // ����� ����� from_chars ��� atof
    *n = std::stod(std::string(Buf));
  } catch (...) {
    err = 1;
  }
  return err;
}
