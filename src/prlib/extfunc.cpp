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

// TODO: автоматически менять
const char *kPrologVersion = "24 марта 2023";

PredicateState argnull(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (name) {
  case hpfail: return PredicateState::No;  // 5;  // ложь
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
  case hpversion: pldout(kPrologVersion); break;
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
//====================================содрано=============

// получение строки связанной с переменной
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

// ВЫП
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
    throw std::runtime_error("ВЫП: Недопустимый тип аргументов");
  }
  }
  if (w < heap->freeheap) {
    if (w == hpassrt) {
      outerror(ErrorCode::UnknownError);  // 24
      return PredicateState::Error;       // 1; // r_t_e(41);
    }
    ClVar->head = ScVar->vgoal[maxarity];
    ClVar->frame2 = ScVar->vgoal[2 * maxarity];
    return PredicateState::Builtin;  // 6;  // встроенный предикат
  }

  // невстроенный предикат под call
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
    return PredicateState::Error;       // 1; // r_t_e(41) не выполнимый предикат ВЫП
  }
  return PredicateState::Error;  // 1;
}

// запись строки и унификация его с arg
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

// унификация целого num с arg аргументом предиката
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

// унификация целых num1 и num2 с arg1 и arg2 соотвественно аргументами предиката
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

// унификация float num с arg аргументом предиката
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

// унификация float num1 с arg1 аргументом предиката
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

// СЛУЧ
PredicateState prrandom(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType n, m;
  static std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  // TODO: multiprec rng gen
  // static std::uniform_int_distribution<IntegerType> dst;
  static std::uniform_int_distribution<long long> dst;
  switch (sw) {
  case 7:  // целое: инициализация генератора
    n = occ(0, ScVar, ClVar, heap);
    rng = std::mt19937_64(n.convert_to<long long>());
    return PredicateState::Yes;  // 3;
  case 5:                        // переменная: получить новое значение
    return zap1(rng(), 1, ScVar, ClVar, heap);
  case 577:  // переменная: получить новое значение
    n = occ(1, ScVar, ClVar, heap);
    m = occ(2, ScVar, ClVar, heap);
    if (n >= m) {
      throw std::runtime_error("СЛУЧ: АРГ2 >= АРГ3");
    }
    // dst = std::uniform_int_distribution<IntegerType>(n, m);
    dst = std::uniform_int_distribution<long long>(n.convert_to<long long>(), m.convert_to<long long>());
    return zap1(dst(rng), 1, ScVar, ClVar, heap);
  default: {
    throw std::runtime_error("СЛУЧ: Недопустимый тип аргументов");
  }
  }
}

/*
bool see( void ) { return false; };
bool mytell( void ){ return false; };
*/
//=========================конец содранного

// ЗАПИСЬ_В
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

    // пытаемся открыть файл для записи.
    ClVar->PrSetting->out.open(str);
    if (!ClVar->PrSetting->out.is_open()) {
      outerror(ErrorCode::AnExceptionOccurredDuringExecution);  // 43
      return PredicateState::No;                                // 5; // r_t_e_(не могу открыть файл)
    }
    ClVar->PrSetting->name_out_file = str;
    return PredicateState::Yes;  // 3;
  }
  case 5:  // char* ps=newStr(namefileout);
    return zap3(ClVar->PrSetting->name_out_file, 1, ScVar, ClVar, heap);

  default: {
    throw std::runtime_error("ЗАПИСЬ_В: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Error;  // 1
}

// ЧТЕНИЕ_ИЗ
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
      return PredicateState::No;                                // 5; // r_t_e_(не могу открыть файл)
    }
    ClVar->PrSetting->name_in_file = str;
    return PredicateState::Yes;  // 3;
  }
  case 5:  // char* ps=newStr(namefilein);
    return zap3(ClVar->PrSetting->name_in_file, 1, ScVar, ClVar, heap);

  default: {
    throw std::runtime_error("ЧТЕНИЕ_ИЗ: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Error;  // 1;
}

// получение целого связанного с переменной
IntegerType occ(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap) {
  recordinteger *pint = heap->GetPrecordinteger(ScVar->vgoal[maxarity + x]);
  return pint->value;
}

// получение вещесв связанного с переменной
FloatType occf(size_t x, TScVar *ScVar, TClVar *ClVar, array *heap) {
  recordfloat *pf = heap->GetPrecordfloat(ScVar->vgoal[maxarity + x]);
  return pf->value;
}

// ВВКОД
PredicateState priocod(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  static char str0[2]{};
  switch (sw) {
  case 7:  // целое !!!посмотреть на ввод вещественных этого делать нельзя
  {
    if (ClVar->PrSetting->out.is_open()) {
      ClVar->PrSetting->out << char(occ(0, ScVar, ClVar, heap));
    } else {
      str0[0] = char(occ(0, ScVar, ClVar, heap));
      usrout(str0);
    }
    return PredicateState::Yes;  // 3;
  }
  case 5:  // переменная
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
    //  case 1://анонимка
    //    if (ClVar->PrSetting->fin)
    //      fgetc(ClVar->PrSetting->fin);//сдвинуть указатель файла ???
    //    return 3;
  default: {
    throw std::runtime_error("ВВКОД: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Error;  // 1;
}

// ВВОДЦЕЛ
PredicateState prrdint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType w{};
  const char *caption = "Введите целое";

  switch (sw) {
  case 7:   // Целое
  case 74:  // Целое, "caption"
  case 79:  // Целое, caption
  {         // Если введённое число совпало с аргументом то ИСТИНА иначе ЛОЖЬ
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
  case 5:   // Переменная
  case 54:  // Переменная, "caption"
  case 59:  // Переменная, caption

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
    throw std::runtime_error("ВВОДЦЕЛ: Недопустимый тип аргументов");
  }
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // rte
}

// ВВОДВЕЩ
PredicateState prrdfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  FloatType w{};
  const char *caption = "Введите вещественное";

  switch (sw) {
  case 6:   // Float
  case 64:  // Float, "caption"
  case 69:  // Float, caption
  {         // Если введённое число совпало с аргументом то ИСТИНА иначе ЛОЖЬ
            // TODO нужен ли тут eps?.
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
  case 5:   // Переменная
  case 54:  // Переменная, "caption"
  case 59:  // Переменная, caption

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
    throw std::runtime_error("ВВОДВЕЩ: Недопустимый тип аргументов");
  }
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // rte
}

// ВВОДСИМВ
PredicateState prrdsym(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  std::string str2;
  switch (sw) {
  case 9:
  case 4:
  case 99: /*то же самое что и предыдущий пункт но с заголовком диалогового окна*/
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
  case 1:  // анонимка
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
    throw std::runtime_error("ВВОДСИМВ: Недопустимый тип аргументов");
  }
  }
  return PredicateState::No;  // 5;
}

// TODO ВВОДСТР почти тоже самое что и ВВОДСИМВ
// ВВОДСТР
PredicateState prrdstr(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  std::string str2;
  switch (sw) {
  case 9:
  case 4:
  case 99: /*то же самое что и предыдущий пункт но с заголовком диалогового окна*/
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
  case 1:  // анонимка
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
    throw std::runtime_error("ВВОДСТР: Недопустимый тип аргументов");
  }
  }
  return PredicateState::No;  // 5;
}

// преобразование в целое
PredicateState print(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: return PredicateState::Yes;  // 3;  // уже целое
  case 6: return zap1(static_cast<IntegerType>(occf(0, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::ErrorInBuiltinPredicateInt);  // 36
  return PredicateState::Error;                     // 1
}

// преобразование в float
PredicateState prfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: return zap1f(static_cast<FloatType>(occ(0, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  case 6: return PredicateState::Yes;  // 3;
  }
  outerror(ErrorCode::ErrorInBuiltinPredicateFloat);  // 37
  return PredicateState::Error;                       // 1
}

// ЖДИ
PredicateState prwait(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 7: break;
  default: {
    throw std::runtime_error("ЖДИ: Недопустимый тип аргументов");
  }
  }
  IntegerType n = occ(0, ScVar, ClVar, heap);
  if (n > 0) {
    std::this_thread::sleep_for(std::chrono::duration<size_t, std::milli>(n.convert_to<size_t>()));
  }
  return PredicateState::Yes;  // 3;
}

PredicateState argone(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(1, ClVar->head, ScVar, ClVar, heap);
  size_t sw = ScVar->vgoal[0];
  switch (name) {
  case hpcall: return prcall(sw, ScVar, ClVar, heap);                                  // ВЫП
  case hprdsym: return prrdsym(sw, ScVar, ClVar, heap);                                // ВВОДСИМВ
  case hprdstr: return prrdstr(sw, ScVar, ClVar, heap);                                // ВВОДСТР
  case hprdint: return prrdint(sw, ScVar, ClVar, heap);                                // ВВОДЦЕЛ
  case hprdfloat: return prrdfloat(sw, ScVar, ClVar, heap);                            // ВВОДВЕЩ
  case hpiocod: return priocod(sw, ScVar, ClVar, heap);                                // ВВКОД
  case hpsee: return infile(sw, ScVar, ClVar, heap);                                   // ЧТЕНИЕ_ИЗ
  case hptell: return outfile(sw, ScVar, ClVar, heap);                                 // ЗАПИСЬ_В
  case hpvar: return (sw == 5) ? PredicateState::Yes : PredicateState::No;             // 3 : 5; // ПЕР
  case hpint: return (sw == 7) ? PredicateState::Yes : PredicateState::No;             // 3 : 5; // ЦЕЛ
  case hpfloat: return (sw == 6) ? PredicateState::Yes : PredicateState::No;           // 3 : 5; // ВЕЩ
  case hpsym: return (sw == 4) ? PredicateState::Yes : PredicateState::No;             // 3 : 5; // СИМВ
  case hplst: return (sw == 2 || sw == 3) ? PredicateState::Yes : PredicateState::No;  // 3 : 5; // СПИСОК (пустой или с чем-то)
  case hpwait: return prwait(sw, ScVar, ClVar, heap);                                  // ЖДИ
  case hprand: return prrandom(sw, ScVar, ClVar, heap);                                // СЛУЧ
  case hp_int: return print(sw, ScVar, ClVar, heap);                                   // преобразование int
  case hp_float: return prfloat(sw, ScVar, ClVar, heap);                               // преобразование float
  }
  return PredicateState::Error;  // 1;
}

// БОЛЬШЕ
PredicateState prgt(TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType a[2];
  int ind = 0;
  FloatType af[2];
  for (int i = 0; i < 2; i++) {
    switch (ScVar->vgoal[i]) {
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
    throw std::runtime_error("БОЛЬШЕ: Недопустимый тип аргументов");
  }
  }
  return (af[0] > af[1]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
}

// составить строку символов из элементов tp - tp должен быть list
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
        res.push_back(pint->value.convert_to<char>());
      }
    } break;
    case isemptylist: break;
    case islist: {
      res += GetStrFromList(pb, ScVar, ClVar, heap);
    } break;
    case isinteger: {
      recordinteger *pint = (recordinteger *)pb;
      res.push_back(pint->value.convert_to<char>());
    } break;
    default: {
      outerror(ErrorCode::WrongList);  // 15
      return {};
    }  // R_t_e последний не []
    }
    tp = heap->GetPbaserecord(plist->link);
  }
  return res;
}

// СТРСПИС
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
      // строка не пустая
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
    throw std::runtime_error("СТРСПИС: Недопустимый тип аргументов");
  }
  }
  return PredicateState::No;  // 5;  //!!! посмотреть на код возврата
}

// СТРЦЕЛ
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
    return zap3(occ(1, ScVar, ClVar, heap).str(), 1, ScVar, ClVar, heap);
  }
  case 56: {
    return zap3(toString(occf(1, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  }
  default: {
    throw std::runtime_error("СТРЦЕЛ: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Error;
}

// СТРВЕЩ
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
    throw std::runtime_error("СТРВЕЩ: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Error;
}

// ЦЕЛВЕЩ
PredicateState printfloat(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 76:                                                                                                                       // int float
    return (occ(0, ScVar, ClVar, heap) == (IntegerType)occf(1, ScVar, ClVar, heap)) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;
  case 75:                                                                                                                       // int var
  {
    auto int_val = occ(0, ScVar, ClVar, heap);
    auto float_val = int_val.convert_to<FloatType>();
    return zap1f(float_val, 2, ScVar, ClVar, heap);
  }
  case 56:  // var float
  {
    auto float_val = occf(1, ScVar, ClVar, heap);
    auto int_val = static_cast<IntegerType>(float_val);
    return zap1(int_val, 1, ScVar, ClVar, heap);
  }
  default: {
    throw std::runtime_error("ЦЕЛВЕЩ: Недопустимый тип аргументов");
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

// БУКВА; ЦИФРА
PredicateState whatisit(size_t sw, bool (*f)(char), size_t i, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto lnwr = occ_line(0, ScVar, ClVar, heap);
  switch (sw) {
  case 97:  // str int
  case 47: {
    size_t w = occ(1, ScVar, ClVar, heap).convert_to<size_t>();
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
    // i использовать для r_t_e
    throw std::runtime_error("БУКВА|ЦИФРА: Недопустимый тип аргументов");
  }
  }                                       
  return PredicateState::No;
}

// СКОЛЬКО
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
  default: throw std::runtime_error("СКОЛЬКО: Недопустимый тип аргументов");
  }
  outerror(ErrorCode::UnknownError);  // 24
  return PredicateState::Error;       // 1; // r_t_e
}

// ТЕРМ
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
      if (test->ident != issymbol) {
        return PredicateState::No;  // 5;
      }
    }
    plist = (recordlist *)tp;
    while (tp->ident == islist) {
      plist = (recordlist *)tp;
      tp = heap->GetPbaserecord(plist->link);
      i++;  // число элементов в списке
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
    // сделать запись функции с аргументами из списка
    assert(i > 0);
    size_t n = i - 1;  // число целей в новой функции
    auto index = heap->append<size_t>(0, n);
    auto *ptrargs = heap->GetPunsigned(index);
    plist = heap->GetPrecordlist(ScVar->vgoal[maxarity + 1]);
    auto funcsymb = plist->head;
    i = 0;
    plist = heap->GetPrecordlist(plist->link);
    while (plist->ident == islist) {
      ptrargs[i] = plist->head;
      plist = heap->GetPrecordlist(plist->link);
      i++;  // число элементов в списке
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
  case 45:  // str var -над этим надо подумать ??
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

// УДАЛ
PredicateState prdel(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (sw != 47) {
    throw std::runtime_error("УДАЛ: Недопустимый тип аргументов");
  }
  IntegerType i = occ(1, ScVar, ClVar, heap);
  recordsconst *ps = heap->GetPrecordsconst(ScVar->vgoal[maxarity]);
  if (ps->begin == isnil || ps->begin == NULL) {
    return PredicateState::No;  // 5;  // нет предложения
  }
  recordclause *pcpred = 0;
  recordclause *pc = heap->GetPrecordclause(ps->begin);
  size_t w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w++) {
    pcpred = pc;
    pc = heap->GetPrecordclause(pc->next);
  }

  if (w == 1) {  // первое предложение  даже если последнее
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

PredicateState argtwo(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(2, ClVar->head, ScVar, ClVar, heap);
  size_t sw = ScVar->vgoal[0] * 10 + ScVar->vgoal[1];
  switch (name) {
  case hpgt: return prgt(ScVar, ClVar, heap);                         // БОЛЬШЕ
  case hpstint: return prstint(sw, ScVar, ClVar, heap);               // СТРЦЕЛ
  case hpstfloat: return prstfloat(sw, ScVar, ClVar, heap);           // СТРВЕЩ
  case hpstlst: return prstlst(sw, ScVar, ClVar, heap);               // СТРСПИС
  case hpintfloat: return printfloat(sw, ScVar, ClVar, heap);         // ЦЕЛВЕЩ
  case hplettr: return whatisit(sw, letter, 55, ScVar, ClVar, heap);  // БУКВА
  case hpdigit: return whatisit(sw, digit, 56, ScVar, ClVar, heap);   // ЦИФРА
  case hpterm: return prterm(sw, ScVar, ClVar, heap);                 // ТЕРМ
  case hpdel: return prdel(sw, ScVar, ClVar, heap);                   // УДАЛ
  case hpskol: return prskol(sw, ScVar, ClVar, heap);                 // СКОЛЬКО
  case hprdsym: return prrdsym(sw, ScVar, ClVar, heap);               // ВВОДСИМВ (с заголовком)
  case hprdstr: return prrdstr(sw, ScVar, ClVar, heap);               // ВВОДСТР
  case hprdint: return prrdint(sw, ScVar, ClVar, heap);               // ВВОДЦЕЛ
  case hprdfloat: return prrdfloat(sw, ScVar, ClVar, heap);           // ВВОДВЕЩ
  }
  return PredicateState::Error;  // 1;
}

// ТОЧКА
PredicateState prset(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
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
    throw std::runtime_error("ТОЧКА: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Yes;  // 3;
}

// СЦЕП
PredicateState prapp(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 444:
  case 449:
  case 494:
  case 499:
  case 944:
  case 949:
  case 994:
  case 999:  // все str str str
  case 445:
  case 495:
  case 945:
  case 995:  // str str var
  {
    auto res = occ_line(0, ScVar, ClVar, heap) + occ_line(1, ScVar, ClVar, heap);
    if (sw == 445 || sw == 495 || sw == 945 || sw == 995) {
      // если переменная
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
    // если str0 входит в str2 и причем спереди
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
    // если str1 входит в str2 и причем сзади
    if (str2.size() > str1.size() && str2.substr(str2.size() - str1.size()) == str1) {
      return zap3(str2.substr(0, str2.size() - str1.size()), 1, ScVar, ClVar, heap);
    }
    break;
  }
  default: {
    throw std::runtime_error("СЦЕП: Недопустимый тип аргументов");
  }
  }
  return PredicateState::No;  // 5;
}

// две функции работают в паре. они используют общую
// глобальную переменную count_var и общий массив tvar
// первой стартует VarOnList
int count_var;

// подсчет переменных в списке вернет 1 если ошибка
int VarOnList(baserecord *pb, TScVar *ScVar, TClVar *ClVar, array *heap);

// подсчет переменных в функции вернет 1 если ошибка
int VarOnFunc(baserecord *pb, TScVar *ScVar, TClVar *ClVar, array *heap) {
  if (pb->ident != isfunction) {
    return 1;  // ошибка
  }
  recordfunction *pf = (recordfunction *)pb;
  auto *ptrarg = heap->GetPunsigned(pf->ptrarg);
  for (size_t i = 0; i < pf->narg; i++) {
    baserecord *tp = heap->GetPbaserecord(ptrarg[i]);
    switch (tp->ident) {
    case isfunction:
      if (VarOnFunc((baserecord *)tp, ScVar, ClVar, heap)) {
        return 1;  // ошибка
      }
      break;
    case islist:
      if (VarOnList((baserecord *)tp, ScVar, ClVar, heap)) {
        return 1;  // ошибка
      }
      break;
    case isvar: count_var++; break;
    }
  }
  return 0;
};

// подсчет переменных в списке вернет 1 если ошибка
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
        return 1;  // ошибка
      }
      break;
    case isvar: count_var++; break;
    }
    pl = heap->GetPrecordlist(pl->link);
  } while (pl->ident == islist);
  return 0;
};

// подготовка целей для предиката доб
//  поместит цели в массив bpt

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
    // выбирають цели из списка
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
  sprintf(p, "Value: %s", pbr->value.str().c_str());
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
      // это не было написано Балдиным
      // Если лист, то скопирую его
      // Константантый наверно можно не копировать?
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
          // Это всегда работает?
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
    return -1;  // ошибка
  }
  int Count = 0;
  auto *ptrarg = heap->GetPunsigned(pf->ptrarg);
  for (size_t i = 0; i < pf->narg; i++) {
    baserecord *tp = heap->GetPbaserecord(ptrarg[i]);
    switch (tp->ident) {
    case isvar: 
        // Vars.push_back((recordvar *)tp); Пока убрал, т.к. не используется
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
      throw std::runtime_error(std::string("ДОБ: ЦЕЛЬ содержит недопустимый терм: ") + tp->name());
      // throw std::runtime_error("GetVarCountFromFunction: unknown term: " + std::to_string(tp->ident));
      // раньше был pldout
      // return -1;
    }
  }
  return Count;
};

int GetVarCountFromClause(recordclause *rc, TScVar *ScVar, TClVar *ClVar, array *heap) {
  int VarCount = 0;
  if (rc->ident != isclause) {
    throw std::runtime_error(std::runtime_error("No clause in func GetVarCountFromCluse"));
    // раньше был pldout
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
      // throw std::runtime_error(std::string("ДОБ: ЦЕЛЬ должна быть функцией, получено: ") + br->name());
      // throw std::runtime_error("GetVarCountFromClause: unknown term: " + std::to_string(br->ident));
      // раньше был pldout
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

// ДОБ
PredicateState prassrt(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  auto old_index = heap->last;
  size_t error = 0;
  baserecord *tp;
  recordsconst *ps;
  recordfunction *pf;
  recordclause *pfirstclause;  // первое предложение данного типа
  recordclause *pclause;       // предложение на место которого вставиться требуемое
  size_t index;
  size_t *ptarget;  // указатель на массив с целями

  size_t ntarget = 1;  // число целей в новом предложении
  switch (sw) {
  case 427:
  case 927:
  case 827:  // _,[],int  //рассматриваем предложение как факт
             // в новом предолжении переменных не может быть
  {
    tp = heap->GetPbaserecord(ScVar->vgoal[maxarity]);
    if (tp->ident != isfunction)  // может быть либо функцией или переменной,конкретизированной функцией возможно что
                                  // нужно поискать функцию
    {
      throw std::runtime_error("ДОБ: АРГ1 не функция");
      // return PredicateState::Error;  // 1;
    }
    recordfunction *prf = (recordfunction *)tp;
    auto ArgIndex = heap->append<size_t>(0, prf->narg);
    tp = heap->GetPbaserecord(ScVar->vgoal[maxarity]);
    prf = (recordfunction *)tp;
    auto *parg = heap->GetPunsigned(ArgIndex);
    auto *prf_arg = heap->GetPunsigned(prf->ptrarg);
    for (size_t i = 0; i < prf->narg; i++) {
      // достать константы из агрументов функции.
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
    // нужно подсчитать число переменных
    count_var = 0;
    if (VarOnList(tp, ScVar, ClVar, heap) != 0)  // подсчет числа переменных в новом предложении
    {
      pldout(const_cast<char *>("Ошибка при подсчете числа переменных"));
      return PredicateState::Error;  // 1;
    }
    error = prepare_target(ScVar->vgoal[maxarity + 1], ScVar, ClVar, heap);
    if (error) {
      pldout(const_cast<char *>("Ошибка при поиске целей для ДОБ"));
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
        // throw std::runtime_error("ДОБ: ЦЕЛЬ " + std::to_string(i) + " не является функцией, получаю: " + heap->GetPbaserecord(ptarget[i])->name());
      }
      recordfunction *prf = heap->GetPrecordfunction(ptarget[i]);
      auto ArgIndex = heap->append<size_t>(0, prf->narg);
      prf = heap->GetPrecordfunction(ptarget[i]);
      auto *arg = heap->GetPunsigned(ArgIndex);
      auto *prf_arg = heap->GetPunsigned(prf->ptrarg);
      for (size_t j = 0; j < prf->narg; ++j) {
        arg[j] = *(prf_arg + j);

        // достать константы из агрументов функции.
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
      throw std::runtime_error("ДОБ: Ошибка при подсчёте числа переменных в новом правиле");
      // throw std::runtime_error("Var count calculation in new clause failure");
      // return PredicateState::No;
    }
    rc.nvars = Count;
    index = heap->append(rc);
    break;
  }
  default: {
    throw std::runtime_error("ДОБ: Недопустимый тип аргументов");
  }
  }
  // включим в цепочку предложений
  IntegerType number = occ(2, ScVar, ClVar, heap);  // куда следует воткнуть новое
  // поищем первое предложение
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
    // теперь поискать сколько переменных в предложении
  }
  if (ps->begin != NULL && ps->begin != isnil) {
    pfirstclause = heap->GetPrecordclause(ps->begin);
  } else {
    pfirstclause = NULL;  // нет таких предложения
  }
  recordclause *newcl = heap->GetPrecordclause(index);
  if (number < 2 || !pfirstclause) {
    // поставим новое предложение первым в списке
    newcl->next = ps->begin;
    ps->begin = index;  // стало первым
  } else {
    // придется втыкать в средину или конец
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

// СЛОЖЕНИЕ
PredicateState pradd(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool fl = false;  // если будет true то обработка вещ иначе целых
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
        f[i] = (FloatType)occ(i, ScVar, ClVar, heap);
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
  case 777: return (l[0] + l[1] == l[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // все целые  ццц
  case 666: return (f[0] + f[1] == f[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // все веществ ffff
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

// УМНОЖЕНИЕ   в goal[0].. 1 .. идетифик параметр
PredicateState prmul3(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[3];
  FloatType f[3];
  bool fl = false;  // если будет true то обработка вещ иначе целых
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
        f[i] = (FloatType)occ(i, ScVar, ClVar, heap);
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
  case 777: return (l[0] * l[1] == l[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // все целые  ццц
  case 666: return (f[0] * f[1] == f[2]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // все веществ ffff
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
    if (l[2] % l[0] == 0) {
      return zap1(l[2] / l[0], 2, ScVar, ClVar, heap);
    }
    f[0] = (FloatType)l[0];
    f[2] = (FloatType)l[2];
  case 656:
    if (f[0] == 0) {
      return PredicateState::No;  // 5;
    }
    return zap1f(f[2] / f[0], 2, ScVar, ClVar, heap);
  }
  outerror(ErrorCode::FailedMultiplicationInvalidArguments);  // 30
  return PredicateState::Error;                               // 1; // r_t_e не вып пред умножение
}

PredicateState argthree(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(3, ClVar->head, ScVar, ClVar, heap);
  size_t sw = 100 * ScVar->vgoal[0] + 10 * ScVar->vgoal[1] + ScVar->vgoal[2];
  switch (name) {
  case hpmul: return prmul3(sw, ScVar, ClVar, heap);     // УМНОЖЕНИЕ
  case hpset: return prset(sw, ScVar, ClVar, heap);      // ТОЧКА
  case hpapp: return prapp(sw, ScVar, ClVar, heap);      // СЦЕП
  case hpassrt: return prassrt(sw, ScVar, ClVar, heap);  // ДОБ
  case hpadd: return pradd(sw, ScVar, ClVar, heap);      // СЛОЖЕНИЕ
  case hppaint: return prpaint(sw, ScVar, ClVar, heap);  // ЗАКРАСКА
  case hprand: return prrandom(sw, ScVar, ClVar, heap);  // СЛУЧ
  }
  return PredicateState::Error;
}

// УМНОЖЕНИЕ   в goal[0].. 1 .. идетифик параметр
PredicateState prmul(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  IntegerType l[4];
  FloatType f[4];
  bool fl = false;  // если будет true то обработка вещ иначе целых
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
  /* switch(sw)     //так было до введения float
   {case 7777:return (occ(0)*occ(1)+occ(2)==occ(3))?3:5; //все целые  цццц
    case 6666:return (occf(0)*occf(1)+occf(2)==occf(3))?3:5; //все веществ ffff
    case 5777:if (occ(1))
                          {int work;
                           if ((work=occ(3)-occ(2))%occ(1)!=0) return 5;
                            return zap1(work/occ(1),1);
                          }break;

    case 7577:if (occ(0))                     //цпцц
                          {int work;
                          if ((work=occ(3)-occ(2)) % occ(0)!=0) return 5;
                          return zap1(work/occ(0),2);}break;

    case 7757:return zap1(occ(3)-occ(0)*occ(1),3);    //цпцц
    case 7775:return zap1(occ(0)*occ(1)+occ(2),4);
    case 7557:if (occ(0)) return zap2(occ(3)%occ(0),occ(3)/occ(0),3,2);break;
    case 5757:if (occ(1)) return zap2(occ(3)%occ(1),occ(3)/occ(1),3,1);break;

   }*/

  switch (sw) {
  case 7777: return (l[0] * l[1] + l[2] == l[3]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // все целые  цццц
  case 6666: return (f[0] * f[1] + f[2] == f[3]) ? PredicateState::Yes : PredicateState::No;  // 3 : 5;  // все веществ ffff
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
    if (l[0])  // цпцц
    {
      IntegerType work;
      if ((work = l[3] - l[2]) % l[0] != 0) {
        return PredicateState::No;  // 5;
      }
      return zap1(work / l[0], 2, ScVar, ClVar, heap);
    }
    break;

  case 7757: return zap1(l[3] - l[0] * l[1], 3, ScVar, ClVar, heap);   // цпцц
  case 6656: return zap1f(f[3] - f[0] * f[1], 3, ScVar, ClVar, heap);  // цпцц
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
  case 6566:                                                                        /*if (f[0])                     //цпцц
                                                                                          {float work;
                                                                                          if ((work=f[3]-f[2]) % f[0]!=0) return 5;
                                                                                          return zap1f(work/f[0],2);}break;*/
    outerror(ErrorCode::FailedMultiplicationIntegerDivisionOfSubstancesOfNumbers);  // 29
    return PredicateState::Error;                                                   // 1;
  }
  outerror(ErrorCode::FailedMultiplicationInvalidArguments);  // 30
  return PredicateState::Error;                               // 1; // r_t_e не вып пред умножение
}

// ОКРУЖНОСТЬ
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
    throw std::runtime_error("ОКРУЖНОСТЬ: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Yes;  // 3;
}

// ЗАКРАСКА
PredicateState prpaint(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
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

// КОПИЯ
PredicateState prcopy(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  switch (sw) {
  case 4775:
  case 9775:  // str int int var
  {
    auto str0 = occ_line(0, ScVar, ClVar, heap);
    auto i1 = occ(1, ScVar, ClVar, heap).convert_to<size_t>();
    auto i2 = occ(2, ScVar, ClVar, heap).convert_to<long long>();
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
    auto i1 = occ(1, ScVar, ClVar, heap).convert_to<size_t>();
    auto i2 = occ(2, ScVar, ClVar, heap).convert_to<long long>();
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
    auto i2 = occ(2, ScVar, ClVar, heap).convert_to<long long>();
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
    auto i1 = occ(1, ScVar, ClVar, heap).convert_to<size_t>();
    auto str3 = occ_line(3, ScVar, ClVar, heap);
    if (i1 > 0 && i1 + str3.size() <= str0.size() + 1 && str0.substr(i1 - 1, str3.size()) == str3) {
      return zap1(str3.size(), 3, ScVar, ClVar, heap);
    }
    return PredicateState::No;
  }
  default: {
    throw std::runtime_error("КОПИЯ: Недопустимый тип аргументов");
  }
  }
  return PredicateState::No;
}

// ПРЕДЛ
PredicateState prclaus(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  baserecord *tp;
  size_t err = 0;
  size_t index = 0;
  // типы аргументов каждого по отдельности
  size_t sw1;  // первый
  size_t sw2;  // второй
  size_t sw3;  // третий
  size_t sw4 = sw;

  // разбор типов аргументов

  sw1 = sw4 / 1000;
  sw4 = sw4 - sw1 * 1000;
  sw2 = sw4 / 100;
  sw4 = sw4 - sw2 * 100;
  sw3 = sw4 / 10;
  sw4 = sw4 - sw3 * 10;

  // проверка типов
  if (sw1 != 4 && sw1 != 9)
    err = 1;  // symb/str
  if (sw2 != 7)
    err = 2;  // int
  if (sw3 != 5 && sw3 != 4 && sw3 != 8)
    err = 3;  // var/sybm/func
  if (sw4 != 5 && sw4 != 2 && sw4 != 3)
    err = 4;  // var/list/emptylist
  if (err != 0) {
    pldout(const_cast<char *>("Неверный тип аргументов в 'ПРЕДЛ'."));  // как нибудь раскрою номер параметра
    return PredicateState::No;                                         // 5;
  }

  // поищем заданное предложение
  IntegerType i = occ(1, ScVar, ClVar, heap);  // номер заданного предл
  recordsconst *ps = heap->GetPrecordsconst(ScVar->vgoal[maxarity]);
  if (ps->begin == isnil || ps->begin == NULL) {
    return PredicateState::No;  // 5;  // нет предложения
  }
  recordclause *pc = heap->GetPrecordclause(ps->begin);
  IntegerType w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w++) {
    pc = heap->GetPrecordclause(pc->next);
  }
  if (w != i) {
    return PredicateState::No;  // 5;  // предл с данным номером отсутствует
  }

  // нашли нужное предложение
  recordfunction *pf = heap->GetPrecordfunction(ClVar->head);
  // аргументы функции предл
  auto *ptr = heap->GetPunsigned(pf->ptrarg);
  // цели найденного предложения
  auto *p = heap->GetPunsigned(pc->ptrtarget);
  // запомним состояние
  auto oldindex = heap->last;
  ClVar->oldtptr = ClVar->tptr;
  ClVar->frame1 = ClVar->oldsvptr = ClVar->svptr;
  ClVar->svptr = ClVar->frame1 + pc->nvars;  // это число переменных в предл
  if (ClVar->svptr > ClVar->vmaxstack && expand_stack(ClVar) != 0) {
    pldout(const_cast<char *>("Не достаточно памяти в стеке переменных"));
    return PredicateState::No;  // 5;
  }
  // унификация головы предлож с переменной или ...
  if (!unify(p[0], ptr[2], ClVar->frame1, ClVar->frame2, ClVar, heap)) {
    ClVar->svptr = ClVar->oldsvptr;
    // heap->last = oldindex;  //??????
    heap->cleanUp(oldindex);
    zero(ClVar);
    return PredicateState::No;  // 5;
  }

  // получим список из целей найденного предложения
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

      return PredicateState::Yes;  // 3;  // удачная унификация
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
  case hpmul: return prmul(sw, ScVar, ClVar, heap);      // УМНОЖЕНИЕ
  case hpcircl: return prcircl(sw, ScVar, ClVar, heap);  // ОКРУЖНОСТЬ
  case hpcopy: return prcopy(sw, ScVar, ClVar, heap);    // КОПИЯ
  case hpclaus: return prclaus(sw, ScVar, ClVar, heap);  // ПРЕДЛ
  }
  return PredicateState::Error;  // 1;
}

// ЛИНИЯ
PredicateState prger(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
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
    throw std::runtime_error("ЛИНИЯ: Недопустимый тип аргументов");
  }
  }
  return PredicateState::Yes;  // 3;
}

PredicateState argfive(size_t name, TScVar *ScVar, TClVar *ClVar, array *heap) {
  conarg(5, ClVar->head, ScVar, ClVar, heap);
  size_t sw = 10000 * ScVar->vgoal[0] + 1000 * ScVar->vgoal[1] + 100 * ScVar->vgoal[2] + 10 * ScVar->vgoal[3] + ScVar->vgoal[4];
  switch (name) {
  case hpline: return prger(sw, ScVar, ClVar, heap);  // ЛИНИЯ
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
  return false;  // нечего сюда лезть
}

//=========== функции ввода ==============
std::string Inputstring(const char *caption) {
  const char *pCaption = "Введите строку";
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
  const char *pCaption = "Введите строку";
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
  char *pCaption = const_cast<char *>("Введите символ");
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
