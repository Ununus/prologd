#ifndef controlh
#define controlh

#include "pstructs.h"

void bound(unsigned *t, unsigned *f, unsigned *j, TClVar *ClVar, array *heap);
void OnControl(TClVar *ClVar, array *heap);
ErrorCode control(TScVar *ScVar, TClVar *ClVar, array *heap, bool *EnableRunning);
bool unify(unsigned tt1, unsigned tt2, unsigned ff1, unsigned ff2, TClVar *, array *);
unsigned char calculation(unsigned term, unsigned frame, long *i, float *f, TClVar *ClVar, array *heap);
void outerror(ErrorCode err);
void zero(TClVar *ClVar);
void to_stac(unsigned *b, unsigned index, unsigned value);
unsigned occur_term(unsigned *TERM, unsigned *FRAME, TClVar *ClVar, array *heap);
unsigned write_term(unsigned TERM, unsigned FRAME, unsigned W, unsigned j, TScVar *ScVar, TClVar *ClVar, array *heap);
// прологовский вывод
PredicateState prout(TScVar *ScVar, TClVar *ClVar, array *heap);  
// расширить стеки
int expand_stack(TClVar *);                                 
void st_4(TScVar *ScVar, TClVar *ClVar, array *heap);
#endif