#pragma once

// —равнение термов так, как его понимает ЅќЋ№Ў≈: числа по значению, символы и
// строки побайтово, списки поэлементно и лексикографически.  ќдна и та же
// функци€ нужна двум предикатам Ч ЅќЋ№Ў≈ (extfunc.cpp) и —ќ–“»–ќ¬ ј
// (sort.cpp), Ч поэтому она в заголовке.

#include <string>

#include "control.h"
#include "pdefs.h"
#include "pstructs.h"

// “екст сравниваетс€ побайтово Ч так ЅќЋ№Ў≈ сравнивала символы и строки с
// самого начала (в CP1251 байт и есть символ).  ѕри равном начале больше тот,
// кто длиннее.
inline int compareText(const std::string &left, const std::string &right) {
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

// “екст символа или строки: записи у них одинаковы (адрес в куче и длина).
inline std::string termText(size_t term, array *heap) {
  const recordstring *ps = heap->GetPrecordstring(term);
  return std::string(heap->GetPchar(ps->ptrsymb), ps->length);
}

// —равнение двух термов: order = -1, 0 или 1.  —писки сравниваютс€
// лексикографически: пустой меньше любого непустого, головы Ч теми же правилами,
// что и аргументы ЅќЋ№Ў≈, а при равных головах сравниваютс€ хвосты.  false Ч
// термы сравнивать нельз€ (разные роды, свободна€ переменна€, составной терм), и
// тогда вызывающий отвечает ошибкой своего предиката.
inline bool compareTerms(size_t left, size_t right, TClVar *ClVar, array *heap, int &order) {
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
    if (leftType == 2 || rightType == 2) {  // [] Ч сама€ коротка€ из всех
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
