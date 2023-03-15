#pragma once

#include "pstructs.h"

void bound(size_t *t, size_t *f, size_t *j, TClVar *ClVar, array *heap);
ErrorCode control(TScVar *ScVar, TClVar *ClVar, array *heap, bool *EnableRunning);
bool unify(size_t tt1, size_t tt2, size_t ff1, size_t ff2, TClVar *, array *);
size_t calculation(size_t term, size_t frame, IntegerType *i, FloatType *f, TClVar *ClVar, array *heap);
void outerror(ErrorCode err);
void zero(TClVar *ClVar);
void to_stac(size_t *b, size_t index, size_t value);
size_t occur_term(size_t *TERM, size_t *FRAME, TClVar *ClVar, array *heap);
size_t write_term(size_t TERM, size_t FRAME, size_t W, size_t j, TScVar *ScVar, TClVar *ClVar, array *heap);
// прологовский вывод
PredicateState prout(TScVar *ScVar, TClVar *ClVar, array *heap);
// расширить стеки
int expand_stack(TClVar *);
