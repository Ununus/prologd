#ifndef defsh
#define defsh
//#include <limits.h>
//�������������� �������� ���������
#define isvar 0
#define issymbol 1
#define isinteger 2
#define isfloat 3
#define isexpression 4
#define isunknown 5
#define isemptylist 6
#define islist 7
#define isfunction 8
#define isstring 9
#define isclause 10
#define isclauseq 11  // gue
#define iscut 12
// ��� ����� ������
#define tscreen 0
#define gscreen 1
#define printer 2
#define outfiles 3
#define userfiles 4
// ��� ����� ����������
#define tracce 1    //������
#define outque 2    //����� �������
#define autosave 4  //��� ����������� ��� ����� ����������
#define maxarity 5  //���� max ������� ����������� ���������
//=================================

//��������� ��� ������ � ������ buf
#define isnil INT_MAX            //�� ������ ������
#define isbase INT_MAX - 17      //
#define iscomma INT_MAX - 16     //     ,
#define isstick INT_MAX - 15     //     |
#define isbbeg INT_MAX - 14      //     (
#define isbend INT_MAX - 13      //     )
#define isimpl INT_MAX - 12      // :-  <-
#define isend INT_MAX - 11       // . ;
#define isexpress INT_MAX - 10   // #
#define islbeg INT_MAX - 9       // [
#define islend INT_MAX - 8       // ]
#define isunitminus INT_MAX - 7  // -
#define isminus INT_MAX - 6      // -
#define isslash INT_MAX - 5      // /
#define ismult INT_MAX - 4       // *
#define isplus INT_MAX - 3       // +
#define ismod INT_MAX - 2        // mod
#define isdiv INT_MAX - 1        // div
#define no_memory_out 1          //��� ������ ��� ������
#define no_memory_stac 2         //������������ ����� ���������

#define maxlinelen 1024  //����� ������ ���������,� ������ ������������� ������

#define maxbf 65536
#define maxstaccalc 3200
//#define maxbf 2048       //������ ����� ����������
//#define maxstaccalc 100  //���� ��� ���������� �� ���
#define maxgrx 1024  // max ���������� ��� ��������� ������� X
#define maxgry 768   //                                     Y

//==============��������������� ����������� ������� ======
#define pbakground 1  //���
#define ppixel 2      //�����
#define pline 3       //�����
#define pcirkle 4     //����������
#define ppaint 5      //��������
#endif
