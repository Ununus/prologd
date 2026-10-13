#include <functional>
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

// ������� �������� �������: `machine` ������� �� �����, ����� ��������� �
// Error, � ��� ������ ��� ���� ������� ������ (control.cpp).
void stop_search(TClVar *machine) {
  machine->stat = PredicateState::Error;  // 1;
}

// ������ ��� ��������� ������: ���� ���� � ����� � ���������� (��� � �������).
size_t build_query(size_t goal, size_t nvars, array *heap) {
  const size_t targets = heap->append<size_t>(0, 2);
  heap->GetPunsigned(targets)[0] = goal;
  heap->GetPunsigned(targets)[1] = 0;
  return heap->append(recordclause(isclauseq, 0, nvars, goal, targets));
}

// ������ ����� ���� ��������� �������.  ���� ����, ������� ���� ������, �� ���
// ����� �����������: � ����������� � ��� ��� ���������.
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

// ��������� ������ (�����, ������, ����) �� ����� ���������� ��������: ���
// ����������� ����, ��� ���� ������, � �������� � ���� ��������� ������.
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
  (void)sw;  // ��������� �������� bpred, ���� � ����������
  for (size_t i = 0; i < 2; ++i) {
    if (ScVar->vgoal[i] != kArgFunction && ScVar->vgoal[i] != kArgSymbol) {
      throw std::runtime_error("�������: ������������ ��� ����������");
    }
  }

  // ��������� ������� �� ����� ���� (��� � ��������): � ���������� ��� �������� �
  // vgoal ����� � ����� ��� ���, � �� ���� ������� � �����������.
  const size_t arguments = heap->GetPrecordfunction(ClVar->head)->ptrarg;
  const size_t *ptr = heap->GetPunsigned(arguments);
  const size_t frame = ClVar->frame2;  // ���� ���������� ����

  // ��� ���� ���������� ����� ������������: ����� ���������� �������� ���� � ��
  // �� ������, ������� ����������� ������� ����� ���������.
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
  bool inner_reported = false;  // �������� ��������� ��������� ���� ����� ������

  // �������� ��������� � ���������� ������ �������: ����� ���� ��� �����, �������
  // ������� ��������� ���������� �� ������ ��.
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
      stop_search(&inner);  // ������ ������ ������
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
      stop_search(&nested);  // ���������� ������ ������� ��� ������ �� �������
    }
  };

  ErrorCode err = ErrorCode::NoErrors;
  {
    SettingsGuard settings(ClVar, &nested);
    const size_t query = build_query(condition, to_nested.used(), heap);
    err = run_goal(query, &nested_sc, &nested, heap, ClVar->enableRunning, check);
  }

  if (inner_reported) {
    return PredicateState::Error;  // 1; ��������� ��������� �������� ���������
  }
  if (inner_err != ErrorCode::NoErrors) {
    ClVar->err = inner_err;
    return PredicateState::Error;  // 1
  }
  if (nested.builtin_error) {
    return PredicateState::Error;  // 1; ��������� ��������� �������� �������
  }
  if (err != ErrorCode::NoErrors) {
    ClVar->err = err;
    return PredicateState::Error;  // 1
  }
  return counterexample ? PredicateState::No : PredicateState::Yes;  // 5 : 3
}
