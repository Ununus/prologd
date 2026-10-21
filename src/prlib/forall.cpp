// ДЛЯВСЕХ(Условие, Следствие) — аналог forall: у каждого решения Условия есть
// решение Следствия.  Это та же проверка, что НЕ((Условие,НЕ(Следствие))), но
// записать её руками нельзя: конъюнкция не может быть аргументом предиката.
//
// Условие решается как отдельный вопрос в собственной машине (TClVar, как в
// НАЙТИВСЕ): его связывания не достаются вызывающему.  Для каждого ответа
// Следствие проверяется ещё одной машиной — есть ли у него решение в этом ответе;
// первый же ответ, у которого решения нет, — контрпример, и перебор Условия на
// нём обрывается.  Если решений у Условия нет вовсе, ДЛЯВСЕХ истинно.
//
// Особый предикат, а не библиотечное предложение, по двум причинам.  Записать
// проверку как НЕ((Условие,НЕ(Следствие))) в диалекте нельзя: конъюнкция не
// может быть аргументом предиката.  А тело из двух ВЫП («ВЫП(x),НЕ(ВЫП(y))»)
// разбирает аргументы второй цели в чужом кадре, если первая цель — встроенный
// предикат: ВЫП встроенного не вызывает его, а входит внутрь, не заводя кадра
// (control.cpp, st_3), — это видно и без ДЛЯВСЕХ:
//     цель(x,y):-ВЫП(x),ВЫП(y).   ?цель(БОЛЬШЕ(2,1), факт(1)).
// Здесь же обе цели решаются своими машинами, и ВЫП не участвует вовсе.

#include <functional>
#include <stdexcept>
#include <vector>

#include "control.h"
#include "extfunc.h"
#include "pdefs.h"
#include "pstructs.h"
#include "termcopy.h"

namespace {

// Коды типов аргументов, которыми пользуются предикаты (см. occur_term).
constexpr size_t kArgFunction = 8;
constexpr size_t kArgSymbol = 4;

// Останов перебора ответов: `machine` выходит из цикла, когда состояние —
// Error, и код ошибки при этом остаётся чистым (control.cpp).
void stop_search(TClVar *machine) {
  machine->stat = PredicateState::Error;  // 1;
}

// Запрос для вложенной машины: одна цель и число её переменных (как у вопроса).
size_t build_query(size_t goal, size_t nvars, array *heap) {
  const size_t targets = heap->append<size_t>(0, 2);
  heap->GetPunsigned(targets)[0] = goal;
  heap->GetPunsigned(targets)[1] = 0;
  return heap->append(recordclause(isclauseq, 0, nvars, goal, targets));
}

// Прогон одной цели вложенной машиной.  Поля кучи, которые ведёт машина, на это
// время сохраняются: у вызывающего в них своё состояние.
ErrorCode run_goal(size_t query, TScVar *ScVar, TClVar *machine, array *heap,
                   bool *enableRunning, const std::function<void()> &on_answer) {
  struct Fields {
    array *heap;
    size_t query, ipaclause, ipacltarget, iptclause, iptcltarget, ipnclause, ipncltarget;
    explicit Fields(array *h)
      : heap(h)
      , query(h->query)
      , ipaclause(h->ipaclause)
      , ipacltarget(h->ipacltarget)
      , iptclause(h->iptclause)
      , iptcltarget(h->iptcltarget)
      , ipnclause(h->ipnclause)
      , ipncltarget(h->ipncltarget) {}
    ~Fields() {
      heap->query = query;
      heap->ipaclause = ipaclause;
      heap->ipacltarget = ipacltarget;
      heap->iptclause = iptclause;
      heap->iptcltarget = iptcltarget;
      heap->ipnclause = ipnclause;
      heap->ipncltarget = ipncltarget;
    }
  } saved(heap);
  heap->query = query;
  return control_all(ScVar, machine, heap, enableRunning, on_answer);
}

// Настройки вывода (файлы, ТРАССА, ТИХО) на время вложенного перебора: они
// принадлежат тому, кто ведёт вопрос, а работают с ними вложенные машины.
struct SettingsGuard {
  TClVar *owner;
  TClVar *working;
  SettingsGuard(TClVar *o, TClVar *w)
    : owner(o)
    , working(w) {
    working->PrSetting = std::move(owner->PrSetting);
  }
  ~SettingsGuard() {
    owner->PrSetting = std::move(working->PrSetting);
  }
};

}  // namespace

PredicateState prforall(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  (void)sw;  // аргументы проверил bpred, типы — унификация
  for (size_t i = 0; i < 2; ++i) {
    if (ScVar->vgoal[i] != kArgFunction && ScVar->vgoal[i] != kArgSymbol) {
      throw std::runtime_error("ДЛЯВСЕХ: Недопустимый тип аргументов");
    }
  }

  // Аргументы берутся из самой цели (как в НАЙТИВСЕ): у переменной без значения в
  // vgoal места в стеке уже нет, а по нему узнаётся её подстановка.
  const size_t arguments = heap->GetPrecordfunction(ClVar->head)->ptrarg;
  const size_t *ptr = heap->GetPunsigned(arguments);
  const size_t frame = ClVar->frame2;  // кадр аргументов цели

  // Обе цели копируются одним копировщиком: общие переменные получают одни и те
  // же номера, поэтому подстановки Условия видны Следствию.
  TermCopier to_nested(heap, ClVar, 0, kFindallSlots);
  const size_t condition = to_nested.copy(ptr[0], frame);
  const size_t consequence = to_nested.copy(ptr[1], frame);
  if (to_nested.overflow()) {
    ClVar->err = ErrorCode::TooLongList;  // 44
    return PredicateState::Error;         // 1
  }

  TClVar nested;
  TScVar nested_sc;
  nested_sc.hpempty = ScVar->hpempty;
  nested_sc.hpunkn = ScVar->hpunkn;
  nested_sc.nosymbol = ScVar->nosymbol;
  nested_sc.vtat = ScVar->vtat;

  TClVar inner;
  TScVar inner_sc;
  inner_sc.hpempty = ScVar->hpempty;
  inner_sc.hpunkn = ScVar->hpunkn;
  inner_sc.nosymbol = ScVar->nosymbol;
  inner_sc.vtat = ScVar->vtat;

  bool counterexample = false;
  ErrorCode inner_err = ErrorCode::NoErrors;
  bool inner_reported = false;  // предикат Следствия напечатал свой текст ошибки

  // Проверка Следствия в полученном ответе Условия: нужен один его ответ, поэтому
  // перебор Следствия обрывается на первом же.
  const std::function<void()> check = [&] {
    TermCopier to_inner(heap, &nested, 0, kFindallSlots);
    const size_t goal = to_inner.copy(consequence, 0);
    if (to_inner.overflow()) {
      inner_err = ErrorCode::TooLongList;  // 44
      stop_search(&nested);
      return;
    }
    const size_t query = build_query(goal, to_inner.used(), heap);
    bool found = false;
    const std::function<void()> take = [&] {
      found = true;
      stop_search(&inner);  // хватит одного ответа
    };
    SettingsGuard settings(&nested, &inner);
    inner_err = run_goal(query, &inner_sc, &inner, heap, ClVar->enableRunning, take);
    if (inner.builtin_error) {
      inner_reported = true;
      stop_search(&nested);
      return;
    }
    if (inner_err != ErrorCode::NoErrors) {
      stop_search(&nested);
      return;
    }
    if (!found) {
      counterexample = true;
      stop_search(&nested);  // дальнейшие ответы Условия уже ничего не изменят
    }
  };

  ErrorCode err = ErrorCode::NoErrors;
  {
    SettingsGuard settings(ClVar, &nested);
    const size_t query = build_query(condition, to_nested.used(), heap);
    err = run_goal(query, &nested_sc, &nested, heap, ClVar->enableRunning, check);
  }

  if (inner_reported) {
    return PredicateState::Error;  // 1; сообщение напечатал предикат Следствия
  }
  if (inner_err != ErrorCode::NoErrors) {
    ClVar->err = inner_err;
    return PredicateState::Error;  // 1
  }
  if (nested.builtin_error) {
    return PredicateState::Error;  // 1; сообщение напечатал предикат Условия
  }
  if (err != ErrorCode::NoErrors) {
    ClVar->err = err;
    return PredicateState::Error;  // 1
  }
  return counterexample ? PredicateState::No : PredicateState::Yes;  // 5 : 3
}
