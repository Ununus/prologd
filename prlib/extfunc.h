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
int occ(unsigned x, TScVar * ScVar, TClVar * ClVar, array * heap);//��������� ������ ���������� � ����������
float occf(unsigned x, TScVar * ScVar, TClVar * ClVar, array * heap);//��������� ������ ���������� � ����������

unsigned zap1(int num, unsigned arg, TScVar * ScVar,
			  TClVar * ClVar, array * heap);//���������� ������ num � arg ���������� ���������

//���������� ������ num1 � arg1 ���������� ���������
//���������� ������ num2 � arg2 ���������� ���������
unsigned zap2(int num1, int num2, unsigned arg1, unsigned arg2,
			  TScVar * ScVar,  TClVar * ClVar, array * heap);

unsigned zap1f(float num, unsigned arg,
			   TScVar * ScVar,  TClVar * ClVar, array * heap);//���������� float num � arg ���������� ���������

unsigned outfile(unsigned sw,
			   TScVar * ScVar,  TClVar * ClVar, array * heap);

unsigned prstint(unsigned sw, TScVar * ScVar, TClVar * ClVar,
				array * heap);   //������
unsigned priocod(unsigned sw, TClVar * ClVar);
unsigned prrdint(unsigned sw, TScVar * ScVar, 
				 TClVar * ClVar, array * heap);  //���� ������   !!!����� ���� ������
unsigned prmem(unsigned sw); //�� ������� ��� ���� ����� ����� �-���
unsigned prrdsym(unsigned sw, TScVar * ScVar, TClVar * Clvar,
				 array * heap);   //��������
unsigned prskol(unsigned sw, TScVar * ScVar, TClVar * ClVar,
				array * heap);
unsigned print(unsigned sw, TScVar * ScVar, 
			   TClVar * ClVar, array * heap); //�������������� � �����
unsigned prfloat(unsigned sw, TScVar * ScVar, 
			   TClVar * ClVar, array * heap); //�������������� � float
unsigned prterm(unsigned sw, TScVar * ScVar, 
			   TClVar * ClVar, array * heap);  //�� ����� ��� ���� ��� ������� 5
unsigned argone( unsigned name, TScVar * ScVar, 
			   TClVar * ClVar, array * heap);
unsigned prgt(TScVar * ScVar, TClVar * ClVar, array * heap);
//=========== ������� ����� ==============
int Inputstring(char *buf, int size, char * caption = 0);
int InputSymbol(char *buf);
int InputInt(int *n, char * caption=0);

//out for debug
void PrintTerm(baserecord * pbr, int Level, TScVar * ScVar, TClVar * ClVar, array * heap);
void PrintList(recordlist * pl, int Level, TScVar * ScVar,
			   TClVar * ClVar, array * heap);











#endif