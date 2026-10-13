#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "control.h"
#include "extfunc.h"
#include "pdefs.h"
#include "pstructs.h"
#include "termcopy.h"

namespace {

// ���� ����� ����������, �������� ���������� ��������� (��. occur_term).
constexpr size_t kArgFunction = 8;
constexpr size_t kArgSymbol = 4;

// ��� occur_term ���������� ��� ���������� ��� ��������.
constexpr size_t kFreeVariable = 5;

}  // namespace

PredicateState prfindall(size_t sw, TScVar *ScVar, TClVar *ClVar, array *heap) {
  (void)sw;  // ��������� �������� bpred, ���� � ����������
  if (ScVar->vgoal[1] != kArgFunction && ScVar->vgoal[1] != kArgSymbol) {
    throw std::runtime_error("��������: ������������ ��� ����������");
  }

  // ��������� ������� �� ����� ���� (��� � zap1): � ���������� ��� �������� �
  // vgoal ����� � ����� ��� ���, � �� ���� ������� � �����������.
  const size_t arguments = heap->GetPrecordfunction(ClVar->head)->ptrarg;
  const size_t *ptr = heap->GetPunsigned(arguments);
  const size_t list_term = ptr[2];
  const size_t frame = ClVar->frame2;  // ���� ���������� ����

  // ������ �����������, ��� �������� ������ ���������, �� ���������� ��� ��������
  // ������� ����������: ���������� ������ � �����������.
  size_t template_term = ptr[0];
  size_t template_frame = frame;
  if (occur_term(&template_term, &template_frame, ClVar, heap) == kFreeVariable) {
    template_term = ptr[0];
    template_frame = frame;
  }

  // ���� � ������ ���������� ����� ������������: ����� ���������� �������� ���� �
  // �� �� ������, ������� ����������� ���� �������� � ������.
  TermCopier to_nested(heap, ClVar, 0, kFindallSlots);
  const size_t goal = to_nested.copy(ScVar->vgoal[maxarity + 1], ScVar->vgoal[maxarity * 2 + 1]);
  const size_t skeleton = to_nested.copy(template_term, template_frame);
  const size_t nvars = to_nested.used();
  if (to_nested.overflow()) {
    ClVar->err = ErrorCode::TooLongList;  // 44
    return PredicateState::Error;        // 1
  }

  // ������ ��� ��������� ������: ���� ��� `?����.`
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

  // ����� ������� ������� � ������� �������� ����������� � �������� �����: �����
  // ������� ���������� �������� � ���� ������� ��� ������ �� ��������.
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

  // ���� ����, ������� ������� ������: � ����������� � ��� ��� ���������.
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
    // ��������� ������ (�����, ������, ����) � ���� �� ��, ��� � �����������.
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
    return PredicateState::Error;  // 1; ��������� ��������� �������� ����
  }
  if (err != ErrorCode::NoErrors) {
    ClVar->err = err;  // 44 ��� ����������: ����� ���������� ���, ��� �� ������
    return PredicateState::Error;  // 1
  }

  ClVar->findallused += used;
  size_t list = heap->append(recordemptylist());
  for (auto element = elements.rbegin(); element != elements.rend(); ++element) {
    list = heap->append(recordlist(*element, list));
  }
  // ���������� ������ ����� � ������� ��������, �� ���� � ����� 0.
  if (!unify(list, list_term, 0, frame, ClVar, heap)) {
    return PredicateState::No;  // 5
  }
  return PredicateState::Yes;  // 3
}
