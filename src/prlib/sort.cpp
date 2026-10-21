// —ќ–“»–ќ¬ ј(—писок, ќтсортированный) Ч сортировка списка по возрастанию.
//
// —равнение Ч то же, чем пользуетс€ ЅќЋ№Ў≈ (compare.h): числа по значению,
// символы и строки побайтово, вложенные списки поэлементно.  “ермы разного рода
// (число и слово) им не сравнить, поэтому така€ пара, как и у самого ЅќЋ№Ў≈, Ч
// ошибка, а не какой-то из двух возможных пор€дков.
//
// ѕовторы остаютс€ одним значением, как у sort/2 в ѕрологе-ƒ 2.x: [3,1,2,1] Ч
// это [1,2,3], а 1 и 1.0 Ч одно значение.  —ортировка устойчива€, поэтому из
// равных в списке остаЄтс€ тот, что сто€л раньше.
//
// јргументы берутс€ из головы цели (ptrarg), а не из vgoal: vgoal[maxarity+1]
// помнит тип свободной переменной, но не еЄ место в куче, Ч второй аргумент
// бывает и свободной переменной, и еЄ св€зывает как раз этот предикат.

#include <algorithm>
#include <set>
#include <stdexcept>
#include <vector>

#include "compare.h"
#include "control.h"
#include "extfunc.h"
#include "pdefs.h"
#include "pstructs.h"

namespace {

//  оды типов, которыми отвечает occur_term.
constexpr size_t kArgEmptyList = 2;
constexpr size_t kArgList = 3;

[[noreturn]] void incomparable() {
  throw std::runtime_error("—ќ–“»–ќ¬ ј: Ќедопустимый тип аргументов");
}

// Ёлементы списка, как они лежат в куче: сам список не копируетс€ и не мен€етс€.
// —писок, который содержит себ€, Ч тоже ошибка: обычный обход его не кончит.
std::vector<size_t> listElements(size_t list, TClVar *ClVar, array *heap) {
  std::vector<size_t> elements;
  std::set<size_t> visited;
  size_t term = list;
  for (;;) {
    size_t frame = ClVar->frame2;
    const size_t type = occur_term(&term, &frame, ClVar, heap);
    if (type == kArgEmptyList) {
      return elements;
    }
    if (type != kArgList || !visited.insert(term).second) {
      incomparable();
    }
    const recordlist *cell = heap->GetPrecordlist(term);
    elements.push_back(cell->head);
    term = cell->link;
  }
}

// ћеньше ли левое правого по правилам ЅќЋ№Ў≈; несравнима€ пара Ч ошибка.
bool lessThan(size_t left, size_t right, TClVar *ClVar, array *heap) {
  int order = 0;
  if (!compareTerms(left, right, ClVar, heap, order)) {
    incomparable();
  }
  return order < 0;
}

bool sameTerm(size_t left, size_t right, TClVar *ClVar, array *heap) {
  int order = 0;
  if (!compareTerms(left, right, ClVar, heap, order)) {
    incomparable();
  }
  return order == 0;
}

}  // namespace

PredicateState prsort(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  (void)sw;  // арность провер€ет bpred
  const size_t type = ScVar->vgoal[0];
  if (type != kArgList && type != kArgEmptyList) {
    incomparable();
  }

  const size_t arguments = heap->GetPrecordfunction(ClVar->head)->ptrarg;
  const size_t *argument = heap->GetPunsigned(arguments);
  const size_t frame = ClVar->frame2;

  std::vector<size_t> elements = listElements(argument[0], ClVar, heap);
  std::stable_sort(elements.begin(), elements.end(),
                   [ClVar, heap](size_t left, size_t right) {
                     return lessThan(left, right, ClVar, heap);
                   });
  elements.erase(std::unique(elements.begin(), elements.end(),
                             [ClVar, heap](size_t left, size_t right) {
                               return sameTerm(left, right, ClVar, heap);
                             }),
                 elements.end());

  size_t list = heap->append(recordemptylist());
  for (auto element = elements.rbegin(); element != elements.rend(); ++element) {
    list = heap->append(recordlist(*element, list));
  }
  // Ёлементы Ч термы вызывающего, поэтому и новый список, и второй аргумент
  // живут в его кадре.
  if (!unify(list, argument[1], frame, frame, ClVar, heap)) {
    return PredicateState::No;  // 5
  }
  return PredicateState::Yes;  // 3
}
