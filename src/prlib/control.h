#ifndef controlh
#define controlh
#define PrintVarCount
#undef PrintVarCount
#define _tracce
#undef _tracce
void bound(unsigned* t, unsigned* f, unsigned* j,
  TClVar* ClVar, array* heap);
void OnControl(TClVar* ClVar, array* heap);
int control(TScVar* ScVar, TClVar* ClVar, array* heap, bool* EnableRunning);
bool unify(unsigned tt1, unsigned tt2, unsigned ff1, unsigned ff2,
  TClVar*, array*);
unsigned char calculation(unsigned term, unsigned frame, long* i,
  float* f, TClVar* ClVar, array* heap);
void outerror(int err);
void out(const char* str);
void errout(const char* str);
void zero(TClVar* ClVar);
void to_stac(unsigned* b, unsigned index, unsigned value);
unsigned occur_term(unsigned* TERM, unsigned* FRAME, TClVar* ClVar,
  array* heap);
unsigned write_term(unsigned TERM, unsigned FRAME, unsigned W,
  unsigned j, TScVar* ScVar, TClVar* ClVar, array* heap);
//extern unsigned write_term(unsigned TERM,unsigned FRAME,unsigned w,unsigned j);//,FILE *d=NULL);
unsigned prout(TScVar* ScVar, TClVar* ClVar, array* heap);      //прологовский вывод
int expand_stack(TClVar*);    //расширить стеки
void st_4(TScVar* ScVar, TClVar* ClVar, array* heap);
#ifdef _DEBUG
void PrintProgram();
#endif

#endif