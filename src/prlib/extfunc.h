#ifndef extfunch
#define extfunch

unsigned argnull(unsigned name, TScVar * ScVar, TClVar * ClVar, array * heap);
unsigned prout();
bool bpred(unsigned name, unsigned narg);
unsigned argone(unsigned name, TScVar * ScVar, TClVar * ClVar, array * heap);
unsigned argtwo(unsigned name, TScVar * ScVar, TClVar * ClVar, array * heap);
unsigned argthree(unsigned name, TScVar * ScVar, TClVar * ClVar, array * heap);
unsigned argfour(unsigned name, TScVar * ScVar, TClVar * ClVar, array * heap);
unsigned argfive(unsigned name, TScVar * ScVar, TClVar * ClVar,
				 array * heap);
void phon(int a, int b);
int occ(unsigned x, TScVar * ScVar, TClVar * ClVar, array * heap);//получение целого связанного с переменной
float occf(unsigned x, TScVar * ScVar, TClVar * ClVar, array * heap);//получение вещесв связанного с переменной

unsigned zap1(int num, unsigned arg, TScVar * ScVar,
			  TClVar * ClVar, array * heap);//унификация целого num с arg аргументом предиката

//унификация целого num1 с arg1 аргументом предиката
//унификация целого num2 с arg2 аргументом предиката
unsigned zap2(int num1, int num2, unsigned arg1, unsigned arg2,
			  TScVar * ScVar,  TClVar * ClVar, array * heap);

unsigned zap1f(float num, unsigned arg,
			   TScVar * ScVar,  TClVar * ClVar, array * heap);//унификация float num с arg аргументом предиката

unsigned outfile(unsigned sw,
			   TScVar * ScVar,  TClVar * ClVar, array * heap);

unsigned prstint(unsigned sw, TScVar * ScVar, TClVar * ClVar,
				array * heap);   //стрцел
unsigned priocod(unsigned sw, TClVar * ClVar);
unsigned prrdint(unsigned sw, TScVar * ScVar, 
				 TClVar * ClVar, array * heap);  //ввод целого   !!!нужно ввод вещств
unsigned prmem(unsigned sw); //не понимаю для чего нужна такая ф-ция
unsigned prrdsym(unsigned sw, TScVar * ScVar, TClVar * Clvar,
				 array * heap);   //вводсимв
unsigned prskol(unsigned sw, TScVar * ScVar, TClVar * ClVar,
				array * heap);
unsigned print(unsigned sw, TScVar * ScVar, 
			   TClVar * ClVar, array * heap); //преобразование в целое
unsigned prfloat(unsigned sw, TScVar * ScVar, 
			   TClVar * ClVar, array * heap); //преобразование в float
unsigned prterm(unsigned sw, TScVar * ScVar, 
			   TClVar * ClVar, array * heap);  //не понял для чего это поэтому 5
unsigned argone( unsigned name, TScVar * ScVar, 
			   TClVar * ClVar, array * heap);
unsigned prgt(TScVar * ScVar, TClVar * ClVar, array * heap);
//=========== функции ввода ==============
int Inputstring(char *buf, int size, char * caption = 0);
int InputSymbol(char *buf);
int InputInt(int *n, char * caption=0);

//out for debug
void PrintTerm(baserecord * pbr, int Level, TScVar * ScVar, TClVar * ClVar, array * heap);
void PrintList(recordlist * pl, int Level, TScVar * ScVar,
			   TClVar * ClVar, array * heap);











#endif