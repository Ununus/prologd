// НАЙТИВСЕ(шаблон, цель, список) — все решения цели и список подстановок шаблона
// (findall).
//
// Цель решается как отдельный вопрос в собственной машине (TClVar): связывания
// цели не достаются вызывающему, а её переменные предварительно копируются со
// свежими номерами в кадре вопроса.  Каждый ответ копируется ещё раз — в область
// НАЙТИВСЕ вызывающего (control.cpp, kFindallSlots), поэтому собранный список не
// делит переменные ни с целью, ни с прошлыми копиями, ни с вызывающим.

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "control.h"
#include "extfunc.h"
#include "pdefs.h"
#include "pstructs.h"

namespace {

// Коды типов аргументов, которыми пользуются предикаты (см. occur_term).
constexpr size_t kArgFunction = 8;
constexpr size_t kArgSymbol = 4;

// Что occur_term возвращает для переменной без значения.
constexpr size_t kFreeVariable = 5;

// Копия терма: свободная переменная источника заменяется свежей переменной,
// начиная с номера `first`.  Одна и та же переменная (одно место стека) даёт одну
// и ту же свежую переменную, сколько бы раз она ни встретилась.  Свежих
// переменных заводится не больше `max`, иначе копия залезла бы в чужие места
// стека; признак переполнения сообщает об этом вызывающему.
class TermCopier {
 public:
  TermCopier(array *heap, TClVar *source, size_t first, size_t max)
    : heap_(heap)
    , source_(source)
    , first_(first)
    , max_(max) {}

  //: Копия терма `term`, который читается в кадре `term_frame` машины-источника.
  size_t copy(size_t term, size_t term_frame);

  //: Сколько свежих переменных заняла копия.
  size_t used() const {
    return used_;
  }

  //: Свежие переменные кончились, копия неполна.
  bool overflow() const {
    return overflow_;
  }

 private:
  //: Свежая переменная вместо свободного места `place` машины-источника.
  size_t fresh(size_t place) {
    auto found = fresh_.find(place);
    if (found != fresh_.end()) {
      return found->second;
    }
    if (used_ >= max_) {
      overflow_ = true;
      return heap_->append(recordvar(0, 0, first_));
    }
    const size_t index = heap_->append(recordvar(0, 0, first_ + used_));
    ++used_;
    fresh_.emplace(place, index);
    return index;
  }

  array *heap_;
  TClVar *source_;
  size_t first_;
  size_t max_;
  size_t used_{ 0 };
  bool overflow_{ false };
  std::map<size_t, size_t> fresh_;  // место машины-источника -> свежая переменная
};

size_t TermCopier::copy(size_t term, size_t term_frame) {
  for (;;) {
    if (term == isnil) {  // свободная переменная: `place` уже в term_frame
      return fresh(term_frame);
    }
    baserecord *record = heap_->GetPbaserecord(term);
    switch (record->ident) {
    case isvar: {
      size_t place = 0;
      bound(&term, &term_frame, &place, source_, heap_);
      if (term == isnil) {
        return fresh(place);
      }
      break;  // связана: дальше копируется значение, в своём кадре
    }
    case isfunction: {
      // Всё, что нужно, считывается до вставок в кучу: она может переехать.
      const size_t narg = heap_->GetPrecordfunction(term)->narg;
      const size_t func = heap_->GetPrecordfunction(term)->func;
      const size_t ptrarg = heap_->GetPrecordfunction(term)->ptrarg;
      const size_t *args = heap_->GetPunsigned(ptrarg);
      std::vector<size_t> arguments(args, args + narg);
      const size_t index = heap_->append<size_t>(0, narg);
      size_t *copied = heap_->GetPunsigned(index);
      for (size_t i = 0; i < narg; ++i) {
        copied[i] = copy(arguments[i], term_frame);
      }
      return heap_->append(recordfunction(narg, func, index));
    }
    case islist: {
      const size_t head = heap_->GetPrecordlist(term)->head;
      const size_t link = heap_->GetPrecordlist(term)->link;
      return heap_->append(recordlist(copy(head, term_frame), copy(link, term_frame)));
    }
    case isexpression: {
      const size_t length = heap_->GetPrecordexpression(term)->length;
      const size_t precord = heap_->GetPrecordexpression(term)->precord;
      const size_t *operands = heap_->GetPunsigned(precord);
      std::vector<size_t> expression(operands, operands + length);
      const size_t index = heap_->append<size_t>(0, length);
      size_t *copied = heap_->GetPunsigned(index);
      for (size_t i = 0; i < length; ++i) {
        copied[i] = expression[i] < isbase ? copy(expression[i], term_frame) : expression[i];
      }
      return heap_->append(recordexpression(length, index));
    }
    default:
      return term;  // числа, строки, константы, пустой список: копировать нечего
    }
  }
}

}  // namespace

PredicateState prfindall(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  (void)sw;  // аргументы проверил bpred, типы — унификация
  if (ScVar->vgoal[1] != kArgFunction && ScVar->vgoal[1] != kArgSymbol) {
    throw std::runtime_error("НАЙТИВСЕ: Недопустимый тип аргументов");
  }

  // Аргументы берутся из самой цели (как в zap1): у переменной без значения в
  // vgoal места в стеке уже нет, а по нему узнаётся её подстановка.
  const size_t arguments = heap->GetPrecordfunction(ClVar->head)->ptrarg;
  const size_t *ptr = heap->GetPunsigned(arguments);
  const size_t list_term = ptr[2];
  const size_t frame = ClVar->frame2;  // кадр аргументов цели

  // Шаблон вычисляется, как аргумент любого предиката, но переменная без значения
  // остаётся переменной: собираются именно её подстановки.
  size_t template_term = ptr[0];
  size_t template_frame = frame;
  if (occur_term(&template_term, &template_frame, ClVar, heap) == kFreeVariable) {
    template_term = ptr[0];
    template_frame = frame;
  }

  // Цель и шаблон копируются одним копировщиком: общие переменные получают одни и
  // те же номера, поэтому подстановки цели попадают в шаблон.
  TermCopier to_nested(heap, ClVar, 0, kFindallSlots);
  const size_t goal = to_nested.copy(ScVar->vgoal[maxarity + 1], ScVar->vgoal[maxarity * 2 + 1]);
  const size_t skeleton = to_nested.copy(template_term, template_frame);
  const size_t nvars = to_nested.used();
  if (to_nested.overflow()) {
    ClVar->err = ErrorCode::TooLongList;  // 44
    return PredicateState::Error;        // 1
  }

  // Запрос для вложенной машины: цель как `?цель.`
  const size_t targets = heap->append<size_t>(0, 2);
  heap->GetPunsigned(targets)[0] = goal;
  heap->GetPunsigned(targets)[1] = 0;
  const size_t query = heap->append(recordclause(isclauseq, 0, nvars, goal, targets));

  TClVar nested;
  TScVar nested_sc;
  nested_sc.hpempty = ScVar->hpempty;
  nested_sc.hpunkn = ScVar->hpunkn;
  nested_sc.nosymbol = ScVar->nosymbol;
  nested_sc.vtat = ScVar->vtat;

  // Копии ответов ложатся в область НАЙТИВСЕ вызывающего с текущего места: места
  // прошлых операторов НАЙТИВСЕ в этом вопросе уже заняты их списками.
  std::vector<size_t> elements;
  size_t used = 0;
  const size_t base = ClVar->findallbase + ClVar->findallused;
  const std::function<void()> collect = [&] {
    TermCopier to_result(heap, &nested, base + used, kFindallSlots - ClVar->findallused - used);
    const size_t element = to_result.copy(skeleton, 0);
    if (to_result.overflow()) {
      nested.err = ErrorCode::TooLongList;  // 44
      return;
    }
    used += to_result.used();
    elements.push_back(element);
  };

  // Поля кучи, которые трогает машина: у вызывающего в них своё состояние.
  const size_t saved_query = heap->query;
  const size_t saved_ipaclause = heap->ipaclause;
  const size_t saved_ipacltarget = heap->ipacltarget;
  const size_t saved_iptclause = heap->iptclause;
  const size_t saved_iptcltarget = heap->iptcltarget;
  const size_t saved_ipnclause = heap->ipnclause;
  const size_t saved_ipncltarget = heap->ipncltarget;

  heap->query = query;
  ErrorCode err = ErrorCode::NoErrors;
  {
    // Настройки вывода (файлы, ТРАССА, ТИХО) у цели те же, что у вызывающего.
    struct SettingGuard {
      TClVar *owner;
      TClVar *nested;
      ~SettingGuard() {
        owner->PrSetting = std::move(nested->PrSetting);
      }
    } guard{ ClVar, &nested };
    nested.PrSetting = std::move(ClVar->PrSetting);
    err = control_all(&nested_sc, &nested, heap, ClVar->enableRunning, collect);
  }

  heap->query = saved_query;
  heap->ipaclause = saved_ipaclause;
  heap->ipacltarget = saved_ipacltarget;
  heap->iptclause = saved_iptclause;
  heap->iptcltarget = saved_iptcltarget;
  heap->ipnclause = saved_ipnclause;
  heap->ipncltarget = saved_ipncltarget;

  if (nested.builtin_error) {
    return PredicateState::Error;  // 1; сообщение напечатал предикат цели
  }
  if (err != ErrorCode::NoErrors) {
    ClVar->err = err;  // 44 или прерывание: текст напечатает тот, кто вёл вопрос
    return PredicateState::Error;  // 1
  }

  ClVar->findallused += used;
  size_t list = heap->append(recordemptylist());
  for (auto element = elements.rbegin(); element != elements.rend(); ++element) {
    list = heap->append(recordlist(*element, list));
  }
  // Переменные списка лежат в области НАЙТИВСЕ, то есть в кадре 0.
  if (!unify(list, list_term, 0, frame, ClVar, heap)) {
    return PredicateState::No;  // 5
  }
  return PredicateState::Yes;  // 3
}
