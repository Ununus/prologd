#pragma once

// Копия терма для предикатов, которые решают цель отдельно от вопроса
// (НАЙТИВСЕ, ДЛЯВСЕХ): связанная переменная источника заменяется своим
// значением, свободная — свежей переменной машины-получателя.

#include <map>
#include <vector>

#include "pdefs.h"
#include "pstructs.h"

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

inline size_t TermCopier::copy(size_t term, size_t term_frame) {
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
