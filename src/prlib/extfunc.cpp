#include <stdio.h>
#include <cassert>
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

#include "prlib_config.h"

PredicateState argnull(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (name) {
  case hpfail: return PredicateState::No;  // 5;  // ����
  case hptrace: ClVar->PrSetting->Trace = true; break;
  case hpnottr: ClVar->PrSetting->Trace = false; break;
  case hpcut:
    ClVar->scptr = ClVar->parent;  //"!"
    //   zero();
    if (ClVar->parent) {
      ClVar->vst_con[ClVar->parent - 1] = isnil;
    }
    break;
  case hpquiet: ClVar->quiet = true; break;
  case hpversion: pldout(PROLOGD_VERSION_STRING); break;
  }
  return PredicateState::Yes;  // 3;
}

void conarg(size_t numb, size_t h, TScVar *ScVar, TClVar *ClVar, array *heap) {
  size_t i, term, frame;
  recordfunction *pf = heap->GetPrecordfunction(h);
  auto *ptr = heap->GetPunsigned(pf->ptrarg);
  for (i = 0; i < numb; i++) {
    frame = ClVar->frame2;
    pf = heap->GetPrecordfunction(h);
    ptr = heap->GetPunsigned(pf->ptrarg);
    term = ptr[i];
    ScVar->vgoal[i] = occur_term(&term, &frame, ClVar, heap);
    ScVar->vgoal[maxarity + i] = term;
    ScVar->vgoal[maxarity * 2 + i] = frame;
  }
}
//====================================�������=============

// ��������� ������ ��������� � ����������
char *occ_line(int x, char *lnwr, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char *ad;
  recordstring *ps = heap->GetPrecordstring(ScVar->vgoal[maxarity + x]);
  ad = heap->GetPchar(ps->ptrsymb);
  for (x = 0; x < static_cast<int>(ps->length); x++) {
    *(lnwr + x) = *(ad + x);
  }
  *(lnwr + x) = 0;

  return lnwr;
}

std::string occ_line(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap) {
  std::string rs;  // TODO: string_view
  recordstring *ps = heap->GetPrecordstring(ScVar->vgoal[maxarity + x]);
  char *ad = heap->GetPchar(ps->ptrsymb);
  std::copy(ad, ad + ps->length, std::back_inserter(rs));
  return rs;
}

// ���
PredicateState prcall(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  size_t w;
  switch (sw) {
  case 8: {
    recordfunction *p = heap->GetPrecordfunction(ScVar->vgoal[maxarity]);
    w = p->func;
  } break;
  case 4:
    w = ScVar->vgoal[maxarity];  // symbol
    break;

  default: {
    throw std::runtime_error("���: ������������ ��� ����������");
  }
  }
  if (w < heap->freeheap) {
    if (w == hpassrt) {
      outerror(ErrorCode::UnknownError);  // 24
      return PredicateState::Error;       // 1; // r_t_e(41);
    }
    ClVar->head = ScVar->vgoal[maxarity];
    ClVar->frame2 = ScVar->vgoal[2 * maxarity];
    return PredicateState::Builtin;  // 6;  // ���������� ��������
  }

  // ������������ �������� ��� call
  recordsconst *ps = heap->GetPrecordsconst(w);
  if (ps->begin && ps->begin != isnil) {
    ClVar->newclause = ps->begin;
    if (ClVar->newclause) {
      heap->ipnclause = ClVar->newclause;
      heap->ipncltarget = heap->GetPrecordclause(heap->ipnclause)->ptrtarget;
    }
    return PredicateState::ControlStep;  // 4;
  } else {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1; // r_t_e(41) �� ���������� �������� ���
  }
  return PredicateState::Error;  // 1;
}

// ������ ������ � ���������� ��� � arg
PredicateState zap3(std::string str, int arg, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto bakindex = heap->last;
  auto index = heap->append<char>(0, str.size());
  memcpy(heap->GetPchar(index), str.c_str(), sizeof(char) * str.size());
  index = heap->append(recordstring(index, str.size()));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  auto *ptr = heap->GetPunsigned(pfunction->ptrarg);
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
  auto *ptr = heap->GetPunsigned(pfunction->ptrarg);
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
  auto *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index1, ptr[arg1 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap) && unify(index2, ptr[arg2 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ���������� float num � arg ���������� ���������
PredicateState zap1f(FloatType num, int arg, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto bakindex = heap->last;
  auto index = heap->append(recordfloat(num));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  auto *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index, ptr[arg - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ���������� float num1 � arg1 ���������� ���������
PredicateState zap2f(FloatType num1, FloatType num2, int arg1, int arg2, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto bakindex = heap->last;
  auto index1 = heap->append(recordfloat(num1));
  auto index2 = heap->append(recordfloat(num2));
  recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
  auto *ptr = heap->GetPunsigned(pfunction->ptrarg);
  if (unify(index1, ptr[arg1 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap) && unify(index2, ptr[arg2 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap)) {
    return PredicateState::Yes;  // 3;
  }
  heap->cleanUp(bakindex);
  return PredicateState::No;  // 5;
}

// ����
PredicateState prrandom(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType n, m;
  static std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  // TODO: multiprec rng gen
  // static std::uniform_int_distribution<IntegerType> dst;
  static std::uniform_int_distribution<long long> dst;
  switch (sw) {
  case 7:  // �����: ������������� ����������
    n = occ(0, ScVar, ClVar, heap);
    rng = std::mt19937_64(static_cast<long long>(n.toInt64()));
    return PredicateState::Yes;  // 3;
  case 5:                        // ����������: �������� ����� ��������
    return zap1(rng(), 1, ScVar, ClVar, heap);
  case 577:  // ����������: �������� ����� ��������
    n = occ(1, ScVar, ClVar, heap);
    m = occ(2, ScVar, ClVar, heap);
    if (n >= m) {
      throw std::runtime_error("����: ���2 >= ���3");
    }
    // dst = std::uniform_int_distribution<IntegerType>(n, m);
    dst = std::uniform_int_distribution<long long>(static_cast<long long>(n.toInt64()), static_cast<long long>(m.toInt64()));
    return zap1(dst(rng), 1, ScVar, ClVar, heap);
  default: {
    throw std::runtime_error("����: ������������ ��� ����������");
  }
  }
}

/*
bool see( void ) { return false; };
bool mytell( void ){ return false; };
*/
//=========================����� ����������

// ������_�
PredicateState outfile(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 9:
  case 4:  // char const
  {
    auto str = occ_line(0, ScVar, ClVar, heap);
    ClVar->PrSetting->out.close();
    ClVar->PrSetting->name_out_file.clear();
    if (str.empty()) {
      return PredicateState::Yes;  // 3;
    }
    if (str == "con:") {
      return PredicateState::Yes;  // 3;
    }

    // �������� ������� ���� ��� ������.
    ClVar->PrSetting->out.open(str);
    if (!ClVar->PrSetting->out.is_open()) {
      outerror(ErrorCode::AnExceptionOccurredDuringExecution);  // 43
      return PredicateState::No;                                // 5; // r_t_e_(�� ���� ������� ����)
    }
    ClVar->PrSetting->name_out_file = str;
    return PredicateState::Yes;  // 3;
  }
  case 5:  // char* ps=newStr(namefileout);
    return zap3(ClVar->PrSetting->name_out_file, 1, ScVar, ClVar, heap);

  default: {
    throw std::runtime_error("������_�: ������������ ��� ����������");
  }
  }
  return PredicateState::Error;  // 1
}

// ������_��
PredicateState infile(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 9:
  case 4:  // char const
  {
    auto str = occ_line(0, ScVar, ClVar, heap);
    ClVar->PrSetting->in.close();
    ClVar->PrSetting->name_in_file.clear();
    if (str.empty()) {
      break;
    }
    if (str == "con:") {
      return PredicateState::Yes;  // 3;
    }

    ClVar->PrSetting->in.open(str);
    if (!ClVar->PrSetting->in.is_open()) {
      outerror(ErrorCode::AnExceptionOccurredDuringExecution);  // 43
      return PredicateState::No;                                // 5; // r_t_e_(�� ���� ������� ����)
    }
    ClVar->PrSetting->name_in_file = str;
    return PredicateState::Yes;  // 3;
  }
  case 5:  // char* ps=newStr(namefilein);
    return zap3(ClVar->PrSetting->name_in_file, 1, ScVar, ClVar, heap);

  default: {
    throw std::runtime_error("������_��: ������������ ��� ����������");
  }
  }
  return PredicateState::Error;  // 1;
}

// ��������� ������ ���������� � ����������
IntegerType occ(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap) {
  recordinteger *pint = heap->GetPrecordinteger(ScVar->vgoal[maxarity + x]);
  return pint->value;
}

// ��������� ������ ���������� � ����������
FloatType occf(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap) {
  recordfloat *pf = heap->GetPrecordfloat(ScVar->vgoal[maxarity + x]);
  return pf->value;
}

// �����
PredicateState priocod(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  static char str0[2]{};
  switch (sw) {
  case 7:  // ����� !!!���������� �� ���� ������������ ����� ������ ������
  {
    if (ClVar->PrSetting->out.is_open()) {
      ClVar->PrSetting->out << static_cast<char>(occ(0, ScVar, ClVar, heap).toInt64());
    } else {
      str0[0] = static_cast<char>(occ(0, ScVar, ClVar, heap).toInt64());
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
    throw std::runtime_error("�����: ������������ ��� ����������");
  }
  }
  return PredicateState::Error;  // 1;
}

// �������
PredicateState prrdint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType w{};
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
        auto str = occ_line(1, ScVar, ClVar, heap);
        if (InputInt(&w, str.c_str())) {
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
        auto str = occ_line(1, ScVar, ClVar, heap);
        if (InputInt(&w, str.c_str())) {
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
  default: {
    throw std::runtime_error("�������: ������������ ��� ����������");
  }
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // rte
}

// �������
PredicateState prrdfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  FloatType w{};
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
        auto str = occ_line(1, ScVar, ClVar, heap);
        if (InputFloat(&w, str.c_str())) {
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
        auto str = occ_line(1, ScVar, ClVar, heap);
        if (InputFloat(&w, str.c_str())) {
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
  default: {
    throw std::runtime_error("�������: ������������ ��� ����������");
  }
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // rte
}

// ��������
PredicateState prrdsym(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  std::string str2;
  switch (sw) {
  case 9:
  case 4:
  case 99: /*�� �� ����� ��� � ���������� ����� �� � ���������� ����������� ����*/
  case 94:
  case 49:
  case 44: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> str2)) {
        break;
      }
    } else {
      if (sw == 99 || sw == 94 || sw == 49 || sw == 44) {
        str2 = Inputstring(occ_line(1, ScVar, ClVar, heap).c_str());
      } else {
        str2 = Inputstring();
      }
    }
    return (occ_line(0, ScVar, ClVar, heap) == str2) ? PredicateState::Yes : PredicateState::No;
  }
  case 5:
  case 59:
  case 54: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> str2)) {
        break;
      }
    } else {
      if (sw == 59 || sw == 54) {
        str2 = Inputstring(occ_line(1, ScVar, ClVar, heap).c_str());
      } else {
        str2 = Inputstring();
      }
    }
    return zap3(str2, 1, ScVar, ClVar, heap);
  }
  case 1:  // ��������
  case 19:
  case 14: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> str2)) {
        break;
      }
    } else {
      if (sw == 19 || sw == 14) {
        str2 = Inputstring(occ_line(1, ScVar, ClVar, heap).c_str());
      } else {
        str2 = Inputstring();
      }
    }
    return PredicateState::Yes;  // 3;
  }
  default: {
    throw std::runtime_error("��������: ������������ ��� ����������");
  }
  }
  return PredicateState::No;  // 5;
}

// TODO ������� ����� ���� ����� ��� � ��������
// �������
PredicateState prrdstr(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  std::string str2;
  switch (sw) {
  case 9:
  case 4:
  case 99: /*�� �� ����� ��� � ���������� ����� �� � ���������� ����������� ����*/
  case 94:
  case 49:
  case 44: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!std::getline(ClVar->PrSetting->in, str2)) {
        break;
      }
    } else {
      if (sw == 99 || sw == 94 || sw == 49 || sw == 44) {
        str2 = Inputline(occ_line(1, ScVar, ClVar, heap).c_str());
      } else {
        str2 = Inputline();
      }
    }
    return (occ_line(0, ScVar, ClVar, heap) == str2) ? PredicateState::Yes : PredicateState::No;
  }
  case 5:
  case 59:
  case 54: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!std::getline(ClVar->PrSetting->in, str2)) {
        break;
      }
    } else {
      if (sw == 59 || sw == 54) {
        str2 = Inputline(occ_line(1, ScVar, ClVar, heap).c_str());
      } else {
        str2 = Inputline();
      }
    }
    return zap3(str2, 1, ScVar, ClVar, heap);
  }
  case 1:  // ��������
  case 19:
  case 14: {
    if (ClVar->PrSetting->in.is_open()) {
      if (!std::getline(ClVar->PrSetting->in, str2)) {
        break;
      }
    } else {
      if (sw == 19 || sw == 14) {
        str2 = Inputline(occ_line(1, ScVar, ClVar, heap).c_str());
      } else {
        str2 = Inputline();
      }
    }
    return PredicateState::Yes;  // 3;
  }
  default: {
    throw std::runtime_error("�������: ������������ ��� ����������");
  }
  }
  return PredicateState::No;  // 5;
}

// �������������� � �����
PredicateState print(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: return PredicateState::Yes;  // 3;  // ��� �����
  case 6: return zap1(IntegerType::fromDouble(occf(0, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::ErrorInBuiltinPredicateInt);  // 36
  return PredicateState::Error;                     // 1
}

// �������������� � float
PredicateState prfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: return zap1f(occ(0, ScVar, ClVar, heap).toDouble(), 1, ScVar, ClVar, heap);
  case 6: return PredicateState::Yes;  // 3;
  }
  outerror(ErrorCode::ErrorInBuiltinPredicateFloat);  // 37
  return PredicateState::Error;                       // 1
}

// ���
PredicateState prwait(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: break;
  default: {
    throw std::runtime_error("���: ������������ ��� ����������");
  }
  }
  IntegerType n = occ(0, ScVar, ClVar, heap);
  if (n > 0) {
    std::this_thread::sleep_for(std::chrono::duration<size_t, std::milli>(static_cast<size_t>(n.toInt64())));
  }
  return PredicateState::Yes;  // 3;
}

PredicateState argone(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(1, ClVar->head, ScVar, ClVar, heap);
  size_t sw = ScVar->vgoal[0];
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

static int compareText(const std::string &left, const std::string &right) {
  const size_t common = left.size() < right.size() ? left.size() : right.size();
  for (size_t i = 0; i < common; ++i) {
    if (left[i] < right[i]) {
      return -1;
    }
    if (left[i] > right[i]) {
      return 1;
    }
  }
  if (left.size() == right.size()) {
    return 0;
  }
  return (left.size() > right.size()) ? 1 : -1;
}
static std::string termText(size_t term, array *heap) {
  const recordstring *ps = heap->GetPrecordstring(term);
  return std::string(heap->GetPchar(ps->ptrsymb), ps->length);
}

static bool compareTerms(size_t left, size_t right, TClVar *ClVar, array *heap, int &order) {
  for (;;) {
    size_t leftFrame = ClVar->frame2;
    size_t rightFrame = ClVar->frame2;
    const size_t leftType = occur_term(&left, &leftFrame, ClVar, heap);
    const size_t rightType = occur_term(&right, &rightFrame, ClVar, heap);
    const bool leftNumber = (leftType == 7 || leftType == 6);
    const bool rightNumber = (rightType == 7 || rightType == 6);
    if (leftNumber && rightNumber) {
      if (leftType == 7 && rightType == 7) {
        const IntegerType l = heap->GetPrecordinteger(left)->value;
        const IntegerType r = heap->GetPrecordinteger(right)->value;
        order = (l > r) ? 1 : ((l == r) ? 0 : -1);
      } else {
        const double l = (leftType == 7) ? heap->GetPrecordinteger(left)->value.toDouble()
                                         : heap->GetPrecordfloat(left)->value;
        const double r = (rightType == 7) ? heap->GetPrecordinteger(right)->value.toDouble()
                                          : heap->GetPrecordfloat(right)->value;
        order = (l > r) ? 1 : ((l == r) ? 0 : -1);
      }
      return true;
    }
    const bool leftText = (leftType == 4 || leftType == 9);
    const bool rightText = (rightType == 4 || rightType == 9);
    if (leftText && rightText) {
      order = compareText(termText(left, heap), termText(right, heap));
      return true;
    }
    const bool leftList = (leftType == 2 || leftType == 3);
    const bool rightList = (rightType == 2 || rightType == 3);
    if (leftType == 2 || rightType == 2) {  // [] � ����� �������� �� ����
      if (!leftList || !rightList) {
        return false;
      }
      order = (leftType == rightType) ? 0 : ((leftType == 2) ? -1 : 1);
      return true;
    }
    if (!leftList || !rightList) {
      return false;
    }
    const recordlist *l = heap->GetPrecordlist(left);
    const recordlist *r = heap->GetPrecordlist(right);
    if (!compareTerms(l->head, r->head, ClVar, heap, order)) {
      return false;
    }
    if (order != 0) {
      return true;
    }
    left = l->link;
    right = r->link;
  }
}

// ������
PredicateState prgt(TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType a[2];
  int ind = ScVar->vgoal[0] * 10 + ScVar->vgoal[1];
  FloatType af[2];
  std::string as[2];
  for (int i = 0; i < 2; i++) {
    switch (ScVar->vgoal[i]) {
    case 7:
      a[i] = occ(i, ScVar, ClVar, heap);
      break;  // ind==1 || 2
    case 6:
      af[i] = occf(i, ScVar, ClVar, heap);
      break;  // ind==3 || 6
    case 4:
        as[i] = occ_line(i, ScVar, ClVar, heap);
        break;

    }
  }
  switch (ind) {
  case 66: // float float
      return (af[0] > af[1]) ? PredicateState::Yes : PredicateState::No;
  case 67: // float int
      return (af[0] > static_cast<float>(a[1].toDouble())) ? PredicateState::Yes : PredicateState::No;
  case 76: // int float
      return (static_cast<float>(a[0].toDouble()) > af[1]) ? PredicateState::Yes : PredicateState::No;
  case 77: // int int
      return (a[0] > a[1]) ? PredicateState::Yes : PredicateState::No;
  case 44:  // ������ ������, ������ ������
      return (compareText(as[0], as[1]) > 0) ? PredicateState::Yes : PredicateState::No;
  case 22:  // [] []
  case 23:  // [] list
    return PredicateState::No;
  case 32:  // list []
    return PredicateState::Yes;
  case 33: {  // list list
    int order = 0;
    if (compareTerms(ScVar->vgoal[maxarity], ScVar->vgoal[maxarity + 1], ClVar, heap, order)) {
      return (order > 0) ? PredicateState::Yes : PredicateState::No;
    }
    break;  // ����������� �������� � �� �� ������, ��� � � ������ ����� ������
  }
  default: {
    throw std::runtime_error("������: ������������ ��� ����������");
  }
  }
  throw std::runtime_error("������: ������������ ��� ����������");
  return PredicateState::No;
  //return (af[0] > af[1]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
}

// ��������� ������ �������� �� ��������� tp - tp ������ ���� list
std::string GetStrFromList(baserecord *tp, TScVar *ScVar, TClVar *ClVar, array *heap) {
  std::string res;
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
        res += GetStrFromList(pb, ScVar, ClVar, heap);
      }
      if (pb->ident == isinteger) {
        recordinteger *pint = (recordinteger *)pb;
        res.push_back(static_cast<char>(pint->value.toInt64()));
      }
    } break;
    case isemptylist: break;
    case islist: {
      res += GetStrFromList(pb, ScVar, ClVar, heap);
    } break;
    case isinteger: {
      recordinteger *pint = (recordinteger *)pb;
      res.push_back(static_cast<char>(pint->value.toInt64()));
    } break;
    default: {
      outerror(ErrorCode::WrongList);  // 15
      return {};
    }  // R_t_e ��������� �� []
    }
    tp = heap->GetPbaserecord(plist->link);
  }
  return res;
}

// �������
PredicateState prstlst(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
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
    tp = heap->GetPbaserecord(ScVar->vgoal[maxarity + 1]);
    std::string str1 = GetStrFromList(tp, ScVar, ClVar, heap);
    if (str1.empty()) {
      return PredicateState::Error;
    }
    switch (sw) {
    case 53: return zap3(str1, 1, ScVar, ClVar, heap);
    case 43:
    case 93:
      if (occ_line(0, ScVar, ClVar, heap) == str1) {
        return PredicateState::Yes;
      } else {
        return PredicateState::No;
      }
    }
  }
  case 95:  //   str/sym var
  case 45: {
    auto lnwr = occ_line(0, ScVar, ClVar, heap);
    recordfunction *pfunction = heap->GetPrecordfunction(ClVar->head);
    auto *ptr = heap->GetPunsigned(pfunction->ptrarg);
    if (lnwr.empty()) {
      auto index = heap->append(recordemptylist());
      unify(index, ptr[1], ClVar->frame2, ClVar->frame2, ClVar, heap);
      return PredicateState::Yes;  // 3;
    } else {
      // ������ �� ������
      size_t j = heap->last, index1, index2;
      for (size_t i = 0; i < lnwr.size(); i++) {
        index1 = heap->append(recordinteger(lnwr[i]));
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
    throw std::runtime_error("�������: ������������ ��� ����������");
  }
  }
  return PredicateState::No;  // 5;  //!!! ���������� �� ��� ��������
}

// ������
PredicateState prstint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 97:  // str int
  case 95:  // str var
  case 47:  // symb int
  case 45:  // symb var
  {
    IntegerType w;
    try {
      w = IntegerType(occ_line(0, ScVar, ClVar, heap));
    } catch (...) {
      return PredicateState::No;
    }

    if (sw == 95 || sw == 45) {
      return zap1(w, 2, ScVar, ClVar, heap);
    }
    return (w == occ(1, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 57: {
    return zap3(occ(1, ScVar, ClVar, heap).toString(), 1, ScVar, ClVar, heap);
  }
  case 56: {
    return zap3(toString(occf(1, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  }
  default: {
    throw std::runtime_error("������: ������������ ��� ����������");
  }
  }
  return PredicateState::Error;
}

// ������
PredicateState prstfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 96:  // str float
  case 95:  // str var
  case 46:  // symb float
  case 45:  // symb var
  {
    FloatType w;
    try {
      w = std::stod(occ_line(0, ScVar, ClVar, heap));
    } catch (...) {
      return PredicateState::No;
    }
    if (sw == 95 || sw == 45) {
      return zap1f(w, 2, ScVar, ClVar, heap);
    }
    return (w == occf(1, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 56: {
    return zap3(toString(occf(1, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  }
  default: {
    throw std::runtime_error("������: ������������ ��� ����������");
  }
  }
  return PredicateState::Error;
}

// ������
PredicateState printfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 76:                                                                                                                       // int float
    return (occ(0, ScVar, ClVar, heap) == IntegerType::fromDouble(occf(1, ScVar, ClVar, heap))) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  case 75:                                                                                                                       // int var
  {
    auto int_val = occ(0, ScVar, ClVar, heap);
    auto float_val = int_val.toDouble();
    return zap1f(float_val, 2, ScVar, ClVar, heap);
  }
  case 56:  // var float
  {
    auto float_val = occf(1, ScVar, ClVar, heap);
    auto int_val = IntegerType::fromDouble(float_val);
    return zap1(int_val, 1, ScVar, ClVar, heap);
  }
  default: {
    throw std::runtime_error("������: ������������ ��� ����������");
  }
  }
  return PredicateState::Error;
}

bool digit(char a) {
  return (isdigitrus(a)) ? true : false;
}

bool letter(char a) {
  return (isalpharus(a) || isalphaang(a)) ? true : false;
}

// �����; �����
PredicateState whatisit(size_t sw, bool (*f)(char), size_t i, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto lnwr = occ_line(0, ScVar, ClVar, heap);
  switch (sw) {
  case 97:  // str int
  case 47: {
    size_t w = static_cast<size_t>(occ(1, ScVar, ClVar, heap).toInt64());
    if (w > 0 && w <= lnwr.size()) {
      return (*f)(lnwr[w - 1]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
    }
    break;
  }
  case 95:  // str var
  case 45: {
    size_t w = 0;
    for (; w < lnwr.size() && !(*f)(lnwr[w]); ++w) {
      ;
    }
    if (w < lnwr.size()) {
      return zap1(w + 1, 2, ScVar, ClVar, heap);
    }
    break;
  }
  default: {
    // i ������������ ��� r_t_e
    throw std::runtime_error("�����|�����: ������������ ��� ����������");
  }
  }                                       
  return PredicateState::No;
}

// �������
PredicateState prskol(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  int i = 0;
  recordsconst *ps = heap->GetPrecordsconst(ScVar->vgoal[maxarity]);
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
  default: throw std::runtime_error("�������: ������������ ��� ����������");
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // r_t_e
}

size_t GetConstTerm(size_t Term, size_t Frame, TClVar *ClVar, array *heap);

// ����
PredicateState prterm(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  baserecord *tp;
  recordlist *plist;
  size_t i = 0;
  switch (sw) {
  case 83:
  case 43:
  case 93:
  case 53: {
    tp = heap->GetPbaserecord(ScVar->vgoal[maxarity + 1]);
    if (tp->ident == islist) {
      plist = (recordlist *)tp;
      baserecord *test = heap->GetPbaserecord(plist->head);
      if (test->ident != issymbol && test->ident != isvar) {
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
      outerror(ErrorCode::WrongList);  // 15
      return PredicateState::Error;
    }
    if (i == 1) {
      recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
      auto *ptr = heap->GetPunsigned(pf->ptrarg);
      if (unify(ptr[0], plist->head, ClVar->frame2, ClVar->frame2, ClVar, heap)) {
        return PredicateState::Yes;  // 3;
      }
      //				heap->last=bakindex; //??????
      return PredicateState::No;  // 5;
    }
    // ������� ������ ������� � ����������� �� ������
    assert(i > 0);
    size_t n = i - 1;  // ����� ����� � ����� �������
    auto index = heap->append<size_t>(0, n);
    auto *ptrargs = heap->GetPunsigned(index);
    plist = heap->GetPrecordlist(ScVar->vgoal[maxarity + 1]);
    auto funcsymb = plist->head;
    if (heap->GetPbaserecord(funcsymb)->ident == isvar) {
      funcsymb = GetConstTerm(funcsymb, ClVar->frame2, ClVar, heap);
      if (heap->GetPbaserecord(funcsymb)->ident == isvar) {
        return PredicateState::No;
      }
    }
    i = 0;
    plist = heap->GetPrecordlist(plist->link);
    while (plist->ident == islist) {
      ptrargs[i] = plist->head;
      plist = heap->GetPrecordlist(plist->link);
      i++;  // ����� ��������� � ������
    }
    index = heap->append(recordfunction(n, funcsymb, index));
    recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
    auto *ptr = heap->GetPunsigned(pf->ptrarg);
    if (unify(ptr[0], index, ClVar->frame2, ClVar->frame2, ClVar, heap)) {
      return PredicateState::Yes;  // 3;
    }
    //          heap->last=bakindex; //????????
    return PredicateState::No;  // 5;
  }
  case 85:  // func var
  {         // func -> list
    recordfunction *rf = heap->GetPrecordfunction(ScVar->vgoal[maxarity]);
    size_t narg = 0;  // rf->narg;
    auto oldindex = heap->last;
    recordlist pl(rf->func, oldindex + sizeof(recordlist));
    auto *ptrarg = heap->GetPunsigned(rf->ptrarg);
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
    recordlist pl(ScVar->vgoal[maxarity], oldindex + sizeof(recordlist));
    auto tlist = heap->append(pl);
    heap->append(recordemptylist());
    recordfunction *rf = heap->GetPrecordfunction(ClVar->head);
    auto *ptrarg = heap->GetPunsigned(rf->ptrarg);
    return (unify(ptrarg[1], tlist, ClVar->frame2, ClVar->frame2, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  default:{
    break;
  }
  }
  return PredicateState::No;  // 5;
}

// ����
PredicateState prdel(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (sw != 47) {
    throw std::runtime_error("����: ������������ ��� ����������");
  }
  IntegerType i = occ(1, ScVar, ClVar, heap);
  recordsconst *ps = heap->GetPrecordsconst(ScVar->vgoal[maxarity]);
  if (ps->begin == isnil || ps->begin == NULL) {
    return PredicateState::No;  // 5;  // ��� �����������
  }
  recordclause *pcpred = 0;
  recordclause *pc = heap->GetPrecordclause(ps->begin);
  size_t w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w++) {
    pcpred = pc;
    pc = heap->GetPrecordclause(pc->next);
  }

  if (w == 1) {  // ������ �����������  ���� ���� ���������
    ps->begin = pc->next;
  } else {
    if (!pcpred) {
      pcpred = heap->GetPrecordclause(ps->begin);
    }
    if (pcpred && pc) {
      pcpred->next = pc->next;
    }
  }
  return PredicateState::Yes;  // 3;
}

// ABS
PredicateState prabs(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  for (size_t i = 0; i < 2; ++i) {
    switch (ScVar->vgoal[i]) {
    case 6: // int
        f[i] = occf(i, ScVar, ClVar, heap);
        break;
    case 7: // float
        l[i] = occ(i, ScVar, ClVar, heap);
     break;
    }
  }
  switch (sw) {
  case 77: return (abs(l[0]) == l[1]) ? PredicateState::Yes : PredicateState::No;
  case 66: return (abs(f[0]) == f[1]) ? PredicateState::Yes : PredicateState::No;
  case 76: return (abs(l[0]).toDouble() == f[1]) ? PredicateState::Yes : PredicateState::No;
  case 67: return (abs(f[0]) == l[1].toDouble()) ? PredicateState::Yes : PredicateState::No;
  case 56: // var float
      if (f[1] >= 0) {
        zap1f(abs(f[1]), 1, ScVar, ClVar, heap);
        return PredicateState::Yes;
      } else {
        return PredicateState::No;
      }
  case 57: // var int
      if (l[1] >= 0) {
        zap1(abs(l[1]), 1, ScVar, ClVar, heap);
        return PredicateState::Yes;
      } else {
        return PredicateState::No;
      }
  case 75: return zap1(abs(l[0]), 2, ScVar, ClVar, heap);
  case 65: return zap1f(abs(f[0]), 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::UnknownError);
  return PredicateState::Error;
}
// ���������� ������ ������ ����� (� ������������� ������� �����)
static IntegerType integer_sqrt(const IntegerType &value) {
  if (value < 0) {
    throw std::runtime_error("SQRT: ������������� �����");
  }
  if (value == 0) {
    return 0;
  }
  IntegerType x = value;
  IntegerType y = (x + 1) / 2;
  while (y < x) {
    x = y;
    y = (x + value / x) / 2;
  }
  return x;
}
// SQRT
PredicateState prsqrt(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool isFloatProcessing = false;
  for (size_t i = 0; i < 2; ++i)
    isFloatProcessing |= (ScVar->vgoal[i] == 6);
  for (size_t i = 0; i < 2; ++i) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (isFloatProcessing) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
        break;
        switch (i) {
        case 0: sw -= 10; break;
        case 1: sw -= 1;
        }
      } else {
        l[i] = occ(i, ScVar, ClVar, heap);
      }
      break;
    }
  }
  switch (sw) {
  case 77: return (integer_sqrt(l[0]) == l[1]) ? PredicateState::Yes : PredicateState::No;
  case 66: return (sqrt(f[0]) == f[1]) ? PredicateState::Yes : PredicateState::No;
  case 57: return zap1(l[1] * l[1], 1, ScVar, ClVar, heap);
  case 56: return zap1f(f[1] * f[1], 1, ScVar, ClVar, heap);
  case 75: return zap1(integer_sqrt(l[0]), 2, ScVar, ClVar, heap);
  case 65: return zap1f(sqrt(f[0]), 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::UnknownError);
  return PredicateState::Error;
}
// SIN
PredicateState prsin(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool isFloatProcessing = true;
  for (size_t i = 0; i < 2; ++i) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (isFloatProcessing) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
        break;
        switch (i) {
        case 0: sw -= 10; break;
        case 1: sw -= 1;
        }
      } else {
        l[i] = occ(i, ScVar, ClVar, heap);
      }
      break;
    }
  }
  switch (sw) {
  // case 66: �� ���� ����� ������ No. ���������� ������������ �� �� �� �����
  case 66: return (sin(f[0]) == f[1]) ? PredicateState::Yes : PredicateState::No;
  case 56: return zap1f(asin(f[1]), 1, ScVar, ClVar, heap); 
  case 65: return zap1f(sin(f[0]), 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::UnknownError);
  return PredicateState::Error;
}
// COS
PredicateState prcos(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool isFloatProcessing = true;
  for (size_t i = 0; i < 2; ++i) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (isFloatProcessing) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
        break;
        switch (i) {
        case 0: sw -= 10; break;
        case 1: sw -= 1;
        }
      } else {
        l[i] = occ(i, ScVar, ClVar, heap);
      }
      break;
    }
  }
  switch (sw) {
  // case 66: �� ���� ����� ������ No. ���������� ������������ �� �� �� �����
  case 66: return (cos(f[0]) == f[1]) ? PredicateState::Yes : PredicateState::No;
  case 56: return zap1f(acos(f[1]), 1, ScVar, ClVar, heap);
  case 65: return zap1f(cos(f[0]), 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::UnknownError);
  return PredicateState::Error;
}
// TAN
PredicateState prtan(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool isFloatProcessing = true;
  for (size_t i = 0; i < 2; ++i) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (isFloatProcessing) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
        break;
        switch (i) {
        case 0: sw -= 10; break;
        case 1: sw -= 1;
        }
      } else {
        l[i] = occ(i, ScVar, ClVar, heap);
      }
      break;
    }
  }
  switch (sw) {
  // case 66: �� ���� ����� ������ No. ���������� ������������ �� �� �� �����
  case 66: return (tan(f[0]) == f[1]) ? PredicateState::Yes : PredicateState::No;
  case 56: return zap1f(atan(f[1]), 1, ScVar, ClVar, heap);
  case 65: return zap1f(tan(f[0]), 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::UnknownError);
  return PredicateState::Error;
}
// EXP
PredicateState prexp(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool isFloatProcessing = true;
  for (size_t i = 0; i < 2; ++i) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (isFloatProcessing) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
        break;
        switch (i) {
        case 0: sw -= 10; break;
        case 1: sw -= 1;
        }
      } else {
        l[i] = occ(i, ScVar, ClVar, heap);
      }
      break;
    }
  }
  switch (sw) {
  // case 66: �� ���� ����� ������ No. ���������� ������������ �� �� �� �����
  case 66: return (exp(f[0]) == f[1]) ? PredicateState::Yes : PredicateState::No;
  case 56: return zap1f(log(f[1]), 1, ScVar, ClVar, heap);
  case 65: return zap1f(exp(f[0]), 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::UnknownError);
  return PredicateState::Error;
}

PredicateState argtwo(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(2, ClVar->head, ScVar, ClVar, heap);
  size_t sw = ScVar->vgoal[0] * 10 + ScVar->vgoal[1];
  switch (name) {
  case hpgt: return prgt(ScVar, ClVar, heap);                         // ������
  case hpforall: return prforall(sw, ScVar, ClVar, heap);             // �������
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
  case hpabs: return prabs(sw, ScVar, ClVar, heap);                   // ABS
  case hpsqrt: return prsqrt(sw, ScVar, ClVar, heap);                 // SQRT
  case hpsin: return prsin(sw, ScVar, ClVar, heap);                   // SIN
  case hpcos: return prcos(sw, ScVar, ClVar, heap);                   // COS
  case hptan: return prtan(sw, ScVar, ClVar, heap);                   // TAN
  case hpexp: return prexp(sw, ScVar, ClVar, heap);                   // EXP
  }
  return PredicateState::Error;  // 1;
}

// �����
PredicateState prset(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType xy, x1, x2, y1, y2, color;
  switch (sw) {
  case 777: {
    x1 = occ(0, ScVar, ClVar, heap);
    x2 = x1 + 5;
    y1 = occ(1, ScVar, ClVar, heap);
    y2 = y1 + 5;
    color = occ(2, ScVar, ClVar, heap);
    SetPixel(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;
  case 775: {
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = occ(1, ScVar, ClVar, heap);
    color = GetPixel(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()));
    return zap1(color, 3, ScVar, ClVar, heap);
  }
  case 757: {
    xy = occ(0, ScVar, ClVar, heap);
    color = occ(2, ScVar, ClVar, heap);
    x1 = xy;
    x2 = xy;
    y1 = 0;
    y2 = maxgry;
    vertical(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(x2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;
  case 577: {
    xy = occ(1, ScVar, ClVar, heap);
    y1 = xy;
    y2 = xy;
    x1 = 0;
    x2 = maxgrx;
    color = 0;
    horisontal(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(y2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;
  case 557: {
    color = occ(2, ScVar, ClVar, heap);
    ClearView(static_cast<unsigned>(color.toInt64()));
  } break;
  default: {
    throw std::runtime_error("�����: ������������ ��� ����������");
  }
  }
  return PredicateState::Yes;  // 3;
}

// ����
PredicateState prapp(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
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
    auto res = occ_line(0, ScVar, ClVar, heap) + occ_line(1, ScVar, ClVar, heap);
    if (sw == 445 || sw == 495 || sw == 945 || sw == 995) {
      // ���� ����������
      return zap3(res, 3, ScVar, ClVar, heap);
    }
    return (res == occ_line(2, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  }
  case 454:
  case 459:
  case 954:  // str var str
  case 959: {
    auto str0 = occ_line(0, ScVar, ClVar, heap);
    auto str2 = occ_line(2, ScVar, ClVar, heap);
    // ���� str0 ������ � str2 � ������ �������
    if (str0.size() <= str2.size() && str0 == str2.substr(0, str0.size())) {
      return zap3(str2.substr(str0.size()), 2, ScVar, ClVar, heap);
    }
    break;
  }
  case 544:
  case 549:
  case 594:  // var str str
  case 599: {
    auto str1 = occ_line(1, ScVar, ClVar, heap);
    auto str2 = occ_line(2, ScVar, ClVar, heap);
    // ���� str1 ������ � str2 � ������ �����
    if (str2.size() > str1.size() && str2.substr(str2.size() - str1.size()) == str1) {
      return zap3(str2.substr(0, str2.size() - str1.size()), 1, ScVar, ClVar, heap);
    }
    break;
  }
  default: {
    throw std::runtime_error("����: ������������ ��� ����������");
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
  auto *ptrarg = heap->GetPunsigned(pf->ptrarg);
  for (size_t i = 0; i < pf->narg; i++) {
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
  } while (pl->ident == islist);
  return 0;
};

// ���������� ����� ��� ��������� ���
//  �������� ���� � ������ bpt

size_t target_number;

size_t prepare_target_from_list(size_t term, TScVar *ScVar, TClVar *ClVar, array *heap);

size_t prepare_target_from_var(size_t term, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!term || term == isnil) {
    return 1;
  }
  size_t error = 0;
  recordvar *pv = heap->GetPrecordvar(term);
  if (pv->ident != isvar) {
    return 1;
  }
  auto frame = ClVar->frame2;
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
    ClVar->vBPT[ClVar->ibpt + target_number++] = term;
  } break;
  default: {
    return 1;
  }
  }
  return 0;
}

size_t prepare_target_from_list(size_t term, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!term || term == isnil) {
    return 1;
  }
  size_t error = 0;
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
      ClVar->vBPT[ClVar->ibpt + target_number++] = pl->head;
      break;
    }
    default: return 1;
    }
    pl = heap->GetPrecordlist(pl->link);
  }
  return 0;  // ok
}

size_t prepare_target(size_t term, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (!term || term == isnil) {
    return 1;
  }
  baserecord *pt = heap->GetPbaserecord(term);
  // if (!ClVar->bpt) {
  //   return 1;
  // }
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

#ifdef PROLOG_DEBUG
// 2006/10/24
void PrintFunction(recordfunction *prf, int Level, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintFunction: Level %d", Level);
  pldout(pBuf);
  baserecord *pbr = heap->GetPbaserecord(prf->func);
  sprintf(p, "func: (%zd)", prf->func);
  pldout(pBuf);
  PrintTerm(pbr, Level + 1, ScVar, ClVar, heap);
  auto *ptrarg = heap->GetPunsigned(prf->ptrarg);
  sprintf(p, "arguments: (%zd)", prf->narg);
  pldout(pBuf);
  for (size_t i = 0; i < prf->narg; i++) {
    sprintf(p, "argument %zd (%zd)", i, ptrarg[i]);
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
  auto len = strlen(p);
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
  auto len = strlen(p);
  strncpy(p + len, pc, pbr->length);
  *(p + len + pbr->length) = 0;
  pldout(pBuf);
  sprintf(p, "num of clause: %zd", pbr->num);
  pldout(pBuf);
}

void PrintInteger(recordinteger *pbr, int Level) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintInteger: Level %d", Level);
  pldout(pBuf);
  sprintf(p, "Value: %s", pbr->value.toString().c_str());
  pldout(pBuf);
}

void PrintTerm(baserecord *pbr, int Level, TScVar *ScVar, TClVar *ClVar, array *heap) {
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char *p = pBuf + Level;
  sprintf(p, "PrintTerm: Level %d", Level);
  pldout(pBuf);
  auto ident = pbr->ident;
  switch (ident) {
  case isfunction: PrintFunction((recordfunction *)pbr, Level + 1, ScVar, ClVar, heap); return;
  case issymbol: PrintRecordsconst((recordsconst *)pbr, Level + 1, ScVar, ClVar, heap); return;
  case isvar: PrintVar((recordvar *)pbr, Level + 1, ScVar, ClVar, heap); return;
  case isinteger: PrintInteger((recordinteger *)pbr, Level + 1); return;
  case islist: PrintList((recordlist *)pbr, Level + 1, ScVar, ClVar, heap); return;
  default: sprintf(pBuf, "unknown id of term: %zd", ident); pldout(pBuf);
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
  switch (pbr->ident) {
  case isfunction: {
    return FindFuncFromFunc(pbr);
  } break;
  }
  return 0;
}
#endif

size_t GetConstTerm(size_t Term, size_t Frame, TClVar *ClVar, array *heap) {
  auto _Term = Term;
  auto _Frame = Frame;
  do {
    baserecord *br = heap->GetPbaserecord(_Term);
    auto ident = br->ident;
    switch (ident) {
    case isvar: {
      size_t j, _PrevTerm = _Term;
      bound(&_Term, &_Frame, &j, ClVar, heap);
      if (_Term == isnil) {
        return _PrevTerm;
      }
    } break;
    case islist: {
      // ��� �� ���� �������� ��������
      // ���� ����, �� �������� ���
      // ������������� ������� ����� �� ����������?
      auto indx = _Term;
      size_t lstPrev = 0, fstIdx = 0;
      while (heap->GetPbaserecord(indx)->ident == islist) {
        auto *lst = heap->GetPrecordlist(indx);
        auto cstTermIdx = GetConstTerm(lst->head, _Frame, ClVar, heap);
        auto lstCopy = heap->append(recordlist(cstTermIdx));
        lst = heap->GetPrecordlist(indx);
        if (lstPrev > 0) {
          heap->GetPrecordlist(lstPrev)->link = lstCopy;
        } else {
          fstIdx = lstCopy;
        }
        lstPrev = lstCopy;
        indx = lst->link;
        while (heap->GetPbaserecord(indx)->ident == isvar) {
          // ��� ������ ��������?
          size_t j;
          bound(&indx, &_Frame, &j, ClVar, heap);
        }
      }
      heap->GetPrecordlist(lstPrev)->link = heap->append(recordemptylist());
      return fstIdx;
    }
    case issymbol:
    case isinteger:
    case isfloat:
    case isemptylist:
    case isstring: return _Term;
    default: return 0;
    }
  } while (1);
}

int GetVarsFromFunction(recordfunction *pf, TScVar *ScVar, TClVar *ClVar, array *heap /*,std::vector<recordvar *> &Vars*/) {
  if (pf->ident != isfunction) {
    return -1;  // ������
  }
  int Count = 0;
  auto *ptrarg = heap->GetPunsigned(pf->ptrarg);
  for (size_t i = 0; i < pf->narg; i++) {
    baserecord *tp = heap->GetPbaserecord(ptrarg[i]);
    switch (tp->ident) {
    case isvar: 
        // Vars.push_back((recordvar *)tp); ���� �����, �.�. �� ������������
        ++Count;
        //Vars[VarCount + Count++] = (recordvar *)tp; break;
        break;
    case issymbol:
    case isinteger:
    case isfloat:
    case isexpression:
    case isunknown:
    case isemptylist: break;
    case isfunction: {
      count_var = 0;
      if (VarOnFunc(tp, ScVar, ClVar, heap)) {
        return -1;
      }
      Count += count_var;
      break;
    }
    case islist: {
      count_var = 0;
      if (VarOnList(tp, ScVar, ClVar, heap)) {
        return -1;
      }
      Count += count_var;
      break;
    }
    default:
      throw std::runtime_error(std::string("���: ���� �������� ������������ ����: ") + tp->name());
      // throw std::runtime_error("GetVarCountFromFunction: unknown term: " + std::to_string(tp->ident));
      // ������ ��� pldout
      // return -1;
    }
  }
  return Count;
};

int GetVarCountFromClause(recordclause *rc, TScVar *ScVar, TClVar *ClVar, array *heap) {
  int VarCount = 0;
  if (rc->ident != isclause) {
    throw std::runtime_error(std::runtime_error("No clause in func GetVarCountFromCluse"));
    // ������ ��� pldout
    // return -1;
  }

  auto *target = heap->GetPunsigned(rc->ptrtarget);
  size_t i = 0;
  //std::vector<recordvar *> Vars;
  while (*(target + i)) {
    int Count = 0;
    baserecord *br = heap->GetPbaserecord(*(target + i));
    switch (br->ident) {
    case isfunction: {
      Count = GetVarsFromFunction((recordfunction *)br, ScVar, ClVar, heap /*,Vars*/);
      if (Count < 0) {
        return -1;
      }
      VarCount += Count;
    } break;
    default:
      break;
      // throw std::runtime_error(std::string("���: ���� ������ ���� ��������, ��������: ") + br->name());
      // throw std::runtime_error("GetVarCountFromClause: unknown term: " + std::to_string(br->ident));
      // ������ ��� pldout
      // return -1;
    }
    i++;
  }
  /*
  int newCount = VarCount;
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
PredicateState prassrt(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto old_index = heap->last;
  size_t error = 0;
  baserecord *tp;
  recordsconst *ps;
  recordfunction *pf;
  recordclause *pfirstclause;  // ������ ����������� ������� ����
  recordclause *pclause;       // ����������� �� ����� �������� ���������� ���������
  size_t index;
  size_t *ptarget;  // ��������� �� ������ � ������

  size_t ntarget = 1;  // ����� ����� � ����� �����������
  switch (sw) {
  case 427:
  case 927:
  case 827:  // _,[],int  //������������� ����������� ��� ����
             // � ����� ����������� ���������� �� ����� ����
  {
    tp = heap->GetPbaserecord(ScVar->vgoal[maxarity]);
    if (tp->ident != isfunction)  // ����� ���� ���� �������� ��� ����������,������������������ �������� �������� ���
                                  // ����� �������� �������
    {
      throw std::runtime_error("���: ���1 �� �������");
      // return PredicateState::Error;  // 1;
    }
    recordfunction *prf = (recordfunction *)tp;
    auto ArgIndex = heap->append<size_t>(0, prf->narg);
    tp = heap->GetPbaserecord(ScVar->vgoal[maxarity]);
    prf = (recordfunction *)tp;
    auto *parg = heap->GetPunsigned(ArgIndex);
    auto *prf_arg = heap->GetPunsigned(prf->ptrarg);
    for (size_t i = 0; i < prf->narg; i++) {
      // ������� ��������� �� ���������� �������.
      *(parg + i) = GetConstTerm(*(prf_arg + i), ClVar->frame2, ClVar, heap);
    }
    auto findex = heap->append(recordfunction(prf->narg, prf->func, ArgIndex));
    index = heap->append<size_t>(0, 2);
    ptarget = heap->GetPunsigned(index);
    ptarget[0] = findex;
    ptarget[1] = NULL;
    index = heap->append(recordclause(isclause, 0, 0, findex, index));
  } break;
  case 437:
  case 937:
  case 837:  //_,list,int
  {
    tp = heap->GetPbaserecord(ScVar->vgoal[maxarity + 1]);
    // ����� ���������� ����� ����������
    count_var = 0;
    if (VarOnList(tp, ScVar, ClVar, heap) != 0)  // ������� ����� ���������� � ����� �����������
    {
      pldout(const_cast<char *>("������ ��� �������� ����� ����������"));
      return PredicateState::Error;  // 1;
    }
    error = prepare_target(ScVar->vgoal[maxarity + 1], ScVar, ClVar, heap);
    if (error) {
      pldout(const_cast<char *>("������ ��� ������ ����� ��� ���"));
      return PredicateState::Error;  // 1;
    }

    ntarget = target_number + 1;
    std::vector<size_t> ptarget(ntarget + 1);

    // auto *pl = heap->GetPrecordlist(ScVar->goal[maxarity + 1]);
    size_t i = 0;
    ptarget[i++] = ScVar->vgoal[maxarity];  // head
    while (i < ntarget) {
      ptarget[i] = ClVar->vBPT[ClVar->ibpt + i - 1];
      i++;
    }
    ptarget[i] = 0;
    for (i = 0; ptarget[i]; ++i) {
      if (heap->GetPbaserecord(ptarget[i])->ident != isfunction) {
        continue;
        // throw std::runtime_error("���: ���� " + std::to_string(i) + " �� �������� ��������, �������: " + heap->GetPbaserecord(ptarget[i])->name());
      }
      recordfunction *prf = heap->GetPrecordfunction(ptarget[i]);
      auto ArgIndex = heap->append<size_t>(0, prf->narg);
      prf = heap->GetPrecordfunction(ptarget[i]);
      auto *arg = heap->GetPunsigned(ArgIndex);
      auto *prf_arg = heap->GetPunsigned(prf->ptrarg);
      for (size_t j = 0; j < prf->narg; ++j) {
        arg[j] = *(prf_arg + j);

        // ������� ��������� �� ���������� �������.
        // auto Arg = GetConstTerm(*(prf_arg + j), ClVar->frame2, ClVar, heap);
        // // Arg = 0;
        // if (Arg) { /*
        //                    recordvar * prv = (recordvar *)&heap->heaps[Arg];
        //                    if (prv->ident == isvar)
        //                    {
        //                            recordvar newvar(prv->ptrsymb, prv->length,
        //                                                                                                    prv->num);
        //                            unsigned vindex = heap->apend(&newvar, sizeof(recordvar));
        //                            arg[j] = vindex;
        //                    }
        //                    else
        //                    */
        //   arg[j] = Arg;
        // } else {
        //   arg[j] = *(prf_arg + j);
        // }
      }
      auto findex = heap->append(recordfunction(prf->narg, prf->func, ArgIndex));
      // if (!unify(ptarget[i], findex, frame2, frame2))
      //     return 1;
      ptarget[i] = findex;
    }
    index = heap->append<size_t>(0, ntarget + 1);
    memcpy(heap->GetPunsigned(index), ptarget.data(), sizeof(size_t) * (ntarget + 1));
    // delete[] ptarget;  // TODO: move it into the heap
    recordclause rc(isclause, 0, 5, ScVar->vgoal[maxarity], index);
    int Count = GetVarCountFromClause(&rc, ScVar, ClVar, heap);
    if (Count < 0) {
      throw std::runtime_error("���: ������ ��� �������� ����� ���������� � ����� �������");
      // throw std::runtime_error("Var count calculation in new clause failure");
      // return PredicateState::No;
    }
    rc.nvars = Count;
    index = heap->append(rc);
    break;
  }
  default: {
    throw std::runtime_error("���: ������������ ��� ����������");
  }
  }
  // ������� � ������� �����������
  IntegerType number = occ(2, ScVar, ClVar, heap);  // ���� ������� �������� �����
  // ������ ������ �����������
  tp = heap->GetPbaserecord(ScVar->vgoal[maxarity]);
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
PredicateState pradd(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool fl = false;  // ���� ����� true �� ��������� ��� ����� �����
  for (size_t i = 0; i < 3; ++i) {
    if (ScVar->vgoal[i] == 6) {
      fl = true;
    }
  }
  for (size_t i = 0; i < 3; ++i) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
        switch (i) {
        case 0: sw -= 100; break;
        case 1: sw -= 10; break;
        case 2: sw -= 1;
        }
      } else {
        l[i] = occ(i, ScVar, ClVar, heap);
      }
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
PredicateState prmul3(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[3];
  FloatType f[3];
  bool fl = false;  // ���� ����� true �� ��������� ��� ����� �����
  for (size_t i = 0; i < 3; ++i) {
    if (ScVar->vgoal[i] == 6) {
      fl = true;
    }
  }
  for (size_t i = 0; i < 3; i++) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
        switch (i) {
        case 0: sw -= 100; break;
        case 1: sw -= 10; break;
        case 2: sw -= 1;
        }
      } else {
        l[i] = occ(i, ScVar, ClVar, heap);
      }
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
    f[2] = l[2].toDouble();
    f[1] = l[1].toDouble();
  case 566:
    if (f[1] == 0) {
      return PredicateState::No;  // 5;
    }
    return zap1f(f[2] / f[1], 1, ScVar, ClVar, heap);
  case 757:
    if (l[0] == 0) {
      return PredicateState::No;  // 5;
    }
    if (l[2] % l[0] == 0) {
      return zap1(l[2] / l[0], 2, ScVar, ClVar, heap);
    }
    f[0] = l[0].toDouble();
    f[2] = l[2].toDouble();
  case 656:
    if (f[0] == 0) {
      return PredicateState::No;  // 5;
    }
    return zap1f(f[2] / f[0], 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::FailedMultiplicationInvalidArguments);  // 30
  return PredicateState::Error;                               // 1; // r_t_e �� ��� ���� ���������
}

PredicateState argthree(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(3, ClVar->head, ScVar, ClVar, heap);
  size_t sw = 100 * ScVar->vgoal[0] + 10 * ScVar->vgoal[1] + ScVar->vgoal[2];
  switch (name) {
  case hpmul: return prmul3(sw, ScVar, ClVar, heap);     // ���������
  case hpset: return prset(sw, ScVar, ClVar, heap);      // �����
  case hpapp: return prapp(sw, ScVar, ClVar, heap);      // ����
  case hpassrt: return prassrt(sw, ScVar, ClVar, heap);  // ���
  case hpadd: return pradd(sw, ScVar, ClVar, heap);      // ��������
  case hppaint: return prpaint(sw, ScVar, ClVar, heap);  // ��������
  case hpfndall: return prfindall(sw, ScVar, ClVar, heap);  // ��������
  case hprand: return prrandom(sw, ScVar, ClVar, heap);  // ����
  }
  return PredicateState::Error;
}

// ���������   � goal[0].. 1 .. �������� ��������
PredicateState prmul(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool fl = false;  // ���� ����� true �� ��������� ��� ����� �����
  for (size_t i = 0; i < 4; i++) {
    if (ScVar->vgoal[i] == 6) {
      fl = true;
    }
  }
  for (size_t i = 0; i < 4; i++) {
    switch (ScVar->vgoal[i]) {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true) {
        f[i] = occ(i, ScVar, ClVar, heap).toDouble();
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
PredicateState prcircl(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType x1, x2, y1, y2, r, color;

  switch (sw) {
  case 7777: {
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = occ(1, ScVar, ClVar, heap);
    r = occ(2, ScVar, ClVar, heap);
    color = occ(3, ScVar, ClVar, heap);
    // x2 = y2 = 0;
    // Ellipse(x1, y1, x2, y2, color);
    Ellipse(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(r.toInt64()), static_cast<long long>(r.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;

  case 7577: {
    color = occ(3, ScVar, ClVar, heap);
    r = occ(2, ScVar, ClVar, heap);
    x1 = occ(0, ScVar, ClVar, heap) - r;
    y1 = 0;
    x2 = x1 + r + r;
    // y2 = maxgry;
    vertical(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(x2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;
  case 5777: {
    color = occ(3, ScVar, ClVar, heap);
    r = occ(2, ScVar, ClVar, heap);
    x1 = 0;
    y1 = occ(1, ScVar, ClVar, heap) - r;
    // x2 = maxgrx;
    y2 = y1 + r + r;
    horisontal(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(y2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;
  case 7757:
  case 7557:
  case 5757:
  case 5557:
  case 5577: {
    color = occ(3, ScVar, ClVar, heap);
    // x1 = 0; y1 = 0;
    // x2 = maxgrx; y2 = maxgry;
    ClearView(static_cast<unsigned>(color.toInt64()));
  } break;
  default: {
    throw std::runtime_error("����������: ������������ ��� ����������");
  }
  }
  return PredicateState::Yes;  // 3;
}

// ��������
PredicateState prpaint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType x, y, color;
  if (sw != 777) {
    outerror(ErrorCode::UnknownError);  // 24
    return PredicateState::Error;       // 1;
  }
  x = occ(0, ScVar, ClVar, heap);
  y = occ(1, ScVar, ClVar, heap);
  color = occ(2, ScVar, ClVar, heap);
  FloodFill(static_cast<long long>(x.toInt64()), static_cast<long long>(y.toInt64()), static_cast<unsigned>(color.toInt64()));
  return PredicateState::Yes;  // 3;
}

// �����
PredicateState prcopy(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 4775:
  case 9775:  // str int int var
  {
    auto str0 = occ_line(0, ScVar, ClVar, heap);
    auto i1 = static_cast<size_t>(occ(1, ScVar, ClVar, heap).toInt64());
    auto i2 = static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64());
    if (i2 < 0) {
      i2 = -i2;
    }
    if (i1 > 0 && i1 + i2 <= str0.size() + 1) {
      return zap3(str0.substr(i1 - 1, i2), 4, ScVar, ClVar, heap);
    }
    break;
  }
  case 4774:
  case 4779:
  case 9774:
  case 9779:  // str int int str
  {
    auto str0 = occ_line(0, ScVar, ClVar, heap);
    auto str3 = occ_line(3, ScVar, ClVar, heap);
    auto i1 = static_cast<size_t>(occ(1, ScVar, ClVar, heap).toInt64());
    auto i2 = static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64());
    if (i2 < 0) {
      i2 = -i2;
    }
    if (i1 > 0 && i1 + i2 <= str0.size() + 1 && i2 == str3.size() && str0.substr(i1 - 1, i2) == str3) {
      return PredicateState::Yes;
    } else {
      return PredicateState::No;
    }
  }
  case 4574:
  case 4579:
  case 9574:
  case 9579:  // str var int str
  {
    auto str0 = occ_line(0, ScVar, ClVar, heap);
    auto str3 = occ_line(3, ScVar, ClVar, heap);
    auto i2 = static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64());
    bool neg = false;
    if (i2 < 0) {
      i2 = -i2;
      neg = true;
    }
    if (i2 == str3.size() && str0.size() >= str3.size()) {
      if (i2 == 0) {
        return zap1(1, 2, ScVar, ClVar, heap);
      }
      size_t idx = (neg ? str0.rfind(str3) : str0.find(str3));
      if (idx < str0.size()) {
        return zap1(idx + 1, 2, ScVar, ClVar, heap);
      }
    }
    return PredicateState::No;
  }
  case 4554:
  case 4559:
  case 9554:
  case 9559:  // str var var str
  {
    auto str0 = occ_line(0, ScVar, ClVar, heap);
    auto str3 = occ_line(3, ScVar, ClVar, heap);
    if (str0.size() >= str3.size()) {
      if (str3.empty()) {
        return zap2(1, 0, 2, 3, ScVar, ClVar, heap);
      }
      size_t idx = str0.find(str3);
      if (idx < str0.size()) {
        return zap2(idx + 1, str3.size(), 2, 3, ScVar, ClVar, heap);
      }
    }
    return PredicateState::No;
  }
  case 4754:
  case 4759:
  case 9754:
  case 9759:  // str int var str
  {
    auto str0 = occ_line(0, ScVar, ClVar, heap);
    auto i1 = static_cast<size_t>(occ(1, ScVar, ClVar, heap).toInt64());
    auto str3 = occ_line(3, ScVar, ClVar, heap);
    if (i1 > 0 && i1 + str3.size() <= str0.size() + 1 && str0.substr(i1 - 1, str3.size()) == str3) {
      return zap1(str3.size(), 3, ScVar, ClVar, heap);
    }
    return PredicateState::No;
  }
  default: {
    throw std::runtime_error("�����: ������������ ��� ����������");
  }
  }
  return PredicateState::No;
}

// �����
PredicateState prclaus(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  baserecord *tp;
  size_t err = 0;
  size_t index = 0;
  // ���� ���������� ������� �� �����������
  size_t sw1;  // ������
  size_t sw2;  // ������
  size_t sw3;  // ������
  size_t sw4 = sw;

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
  recordsconst *ps = heap->GetPrecordsconst(ScVar->vgoal[maxarity]);
  if (ps->begin == isnil || ps->begin == NULL) {
    return PredicateState::No;  // 5;  // ��� �����������
  }
  recordclause *pc = heap->GetPrecordclause(ps->begin);
  IntegerType w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w += 1) {
    pc = heap->GetPrecordclause(pc->next);
  }
  if (w != i) {
    return PredicateState::No;  // 5;  // ����� � ������ ������� �����������
  }

  // ����� ������ �����������
  recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
  // ��������� ������� �����
  auto *ptr = heap->GetPunsigned(pf->ptrarg);
  // ���� ���������� �����������
  auto *p = heap->GetPunsigned(pc->ptrtarget);
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

PredicateState argfour(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(4, ClVar->head, ScVar, ClVar, heap);
  size_t sw = ScVar->vgoal[0] * 1000 + ScVar->vgoal[1] * 100 + ScVar->vgoal[2] * 10 + ScVar->vgoal[3];
  switch (name) {
  case hpmul: return prmul(sw, ScVar, ClVar, heap);      // ���������
  case hpcircl: return prcircl(sw, ScVar, ClVar, heap);  // ����������
  case hpcopy: return prcopy(sw, ScVar, ClVar, heap);    // �����
  case hpclaus: return prclaus(sw, ScVar, ClVar, heap);  // �����
  }
  return PredicateState::Error;  // 1;
}

// �����
PredicateState prger(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType x1, x2, y1, y2, color;
  switch (sw) {
  case 77777: {
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = occ(1, ScVar, ClVar, heap);
    x2 = occ(2, ScVar, ClVar, heap);
    y2 = occ(3, ScVar, ClVar, heap);
    color = occ(4, ScVar, ClVar, heap);
    MoveTo_LineTo(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(x2.toInt64()), static_cast<long long>(y2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;
  case 77757:
    vertical(static_cast<long long>(occ(0, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(1, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
    break;
  case 77577:
    horisontal(static_cast<long long>(occ(0, ScVar, ClVar, heap).toInt64()),
               static_cast<long long>(occ(1, ScVar, ClVar, heap).toInt64()),
               static_cast<long long>(occ(3, ScVar, ClVar, heap).toInt64()),
               static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
    break;
  case 75777:
    vertical(static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(3, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(0, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
    break;
  case 57777:
    horisontal(static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64()),
               static_cast<long long>(occ(3, ScVar, ClVar, heap).toInt64()),
               static_cast<long long>(occ(1, ScVar, ClVar, heap).toInt64()),
               static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
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
    Rectangle(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(x2.toInt64()), static_cast<long long>(y2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;

  case 75757: {
    color = occ(4, ScVar, ClVar, heap);
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = 0;
    x2 = occ(2, ScVar, ClVar, heap);
    y2 = maxgry;
    // SetPenAndBrushColor(color);
    Rectangle(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(x2.toInt64()), static_cast<long long>(y2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;

  case 57577: {
    color = occ(4, ScVar, ClVar, heap);
    x1 = 0;
    y1 = occ(1, ScVar, ClVar, heap);
    x2 = maxgrx;
    y2 = occ(3, ScVar, ClVar, heap);
    // SetPenAndBrushColor(color);
    Rectangle(static_cast<long long>(x1.toInt64()), static_cast<long long>(y1.toInt64()), static_cast<long long>(x2.toInt64()), static_cast<long long>(y2.toInt64()), static_cast<unsigned>(color.toInt64()));
  } break;
  case 75577: {
    vertical(0,
             static_cast<long long>(occ(3, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(0, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
    vertical(maxgrx,
             static_cast<long long>(occ(3, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(0, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
  } break;
  case 57757: {
    vertical(0,
             static_cast<long long>(occ(1, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
    vertical(maxgrx,
             static_cast<long long>(occ(1, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(2, ScVar, ClVar, heap).toInt64()),
             static_cast<long long>(occ(4, ScVar, ClVar, heap).toInt64()));
  } break;
  default: {
    throw std::runtime_error("�����: ������������ ��� ����������");
  }
  }
  return PredicateState::Yes;  // 3;
}

PredicateState argfive(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(5, ClVar->head, ScVar, ClVar, heap);
  size_t sw = 10000 * ScVar->vgoal[0] + 1000 * ScVar->vgoal[1] + 100 * ScVar->vgoal[2] + 10 * ScVar->vgoal[3] + ScVar->vgoal[4];
  switch (name) {
  case hpline: return prger(sw, ScVar, ClVar, heap);  // �����
  }
  return PredicateState::Error;  // 1;
}

bool bpred(size_t name, size_t narg) {
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

  case hpabs:
  case hpsqrt:
  case hpsin:
  case hpcos:
  case hptan:
  case hpexp:

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
  case hpfndall: return (narg == 3) ? true : false;  // ��������
  case hpforall: return (narg == 2) ? true : false;  // �������

  case hpmul: return (narg == 3 || narg == 4) ? true : false;
  case hpcircl:
  case hpcopy:
  case hpclaus: return (narg == 4) ? true : false;  // 4

  case hpline: return (narg == 5) ? true : false;  // 5
  }
  return false;  // ������ ���� �����
}

//=========== ������� ����� ==============
std::string Inputstring(const char *caption) {
  const char *pCaption = "������� ������";
  if (caption) {
    pCaption = caption;
  }
  std::string res = InputStringFromDialog(pCaption, true);
  if (res.empty()) {
    throw std::runtime_error("Can't read");
  }
  return res;
}

std::string Inputline(const char *caption) {
  const char *pCaption = "������� ������";
  if (caption) {
    pCaption = caption;
  }
  std::string res = InputStringFromDialog(pCaption, false);
  if (res.empty()) {
    throw std::runtime_error("Can't read");
  }
  return res;
}

int InputSymbol(char *c) {
  char *pCaption = const_cast<char *>("������� ������");
  auto buf = InputStringFromDialog(pCaption, true);
  if (buf.size() != 1) {
    throw std::runtime_error("Can't read");
  }
  *c = buf[0];
  return 0;
}

int InputInt(IntegerType *n, const char *caption) {
  std::string Buf = InputStringFromDialog(caption, true);
  try {
    *n = std::stoll(std::string(Buf));
  } catch (...) {
    return 1;
  }
  return 0;
}

int InputFloat(FloatType *n, const char *caption) {
  std::string Buf = InputStringFromDialog(caption, true);
  try {
    *n = std::stod(std::string(Buf));
  } catch (...) {
    return 1;
  }
  return 0;
}
