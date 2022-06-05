#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <charconv>
#include "functions.h"
#include "pdefs.h"
//#include "extdecl.h"
#include "scaner.h"
//#include "control.h"
//#include "..\dinputdlg.h"
#include "extfunc.h"
constexpr int MAX_BUILD_PRED = 39;
constexpr int LEN_BUILD_LIBR = 9;

//=======�������������� ����������
const char* NamesOfPredicates[] =
{ "mod", "����",
  "������", "���_������", "!",
  "���", "��������", "�������", "�����",
  "������_��", "������_�", "���", "���","���",
  "����", "�����", "������", "������",
  "�������", "�����", "�����", "����",
  "��������", "�������", "�����", "����",
  "���", "���������", "����������", "��������",
  "�����", "�����", "�����", "����","��������", "���",
  "div","int", "float"
};

const char* Build_in_Libriary[] =
{ "��(x)<-���(x),!,����;",
  "��(_);",
  "���(x,_)<-���(x),!;",
  "���(_,x)<-���(x);",
  "�����(x,x);",
  "������(x,y)<-������(y,x);",
  "��������(x,y,z)<-���������(1,x,y,z);",
  "��<-�����(10),�����(13);",
  "�����(x,y)<-�����(x,1,y,x);",
};

int buildin(TScVar* ScVar, array* heap)  //����������� ���������� ����������
{
  int err = 0;

  for (int i = 0; i < MAX_BUILD_PRED; i++)
  {
    unsigned index = heap->apend((void*)NamesOfPredicates[i],
      (unsigned char)strlen(NamesOfPredicates[i]));
    if (index < 0)
      return 44;
    recordsconst ptr(index, (unsigned char)strlen(NamesOfPredicates[i]));
    index = heap->apend(&ptr, sizeof(recordsconst));
    if (index < 0)
      return 44;
    ScVar->tat[i] = index;
  }
  //freesymbol = MAX_BUILD_PRED;
  ScVar->nosymbol = MAX_BUILD_PRED;
  recordunknown* ptrunkn = new recordunknown();
  ScVar->hpunkn = heap->apend(ptrunkn, sizeof(recordunknown));//��������� �������� � heap
  if (ptrunkn)
    delete ptrunkn;
  if (ScVar->hpunkn < 0)
    return 44;
  recordemptylist* ptre = new recordemptylist();
  ScVar->hpempty = heap->apend(ptre, sizeof(recordemptylist)); //��������� ������� ������
  if (ptre)
    delete ptre;
  if (ScVar->hpempty < 0)
    return 44;

  heap->freeheap = heap->last;

  //extern char*Build_in_Libriary[] ;
  for (int i = 0; i < LEN_BUILD_LIBR && !err; i++)
  {
    char* p = (char*)Build_in_Libriary[i];
    err = scaner(p, ScVar, heap);
  }
  return err;
}

//�������������. ��� ���������� � ��������� ���������
int InitProlog()
{
  int err = 0;//��� ������
  return err;
}

int FreeProlog()
{
  return 0;
}


//������� ��� ���������� ��������� ��������� �������
int tokbb(TScVar* ScVar, array* heap)
{
  int err = 0;
  unsigned int& bptr = ScVar->bptr;
  unsigned int* buf = ScVar->buf;
  unsigned int* goal = ScVar->goal;
  unsigned int& gptr = ScVar->gptr;
  gptr--;
  if (gptr < 0 || gptr >= _maxgptr_ ||
    buf[goal[gptr]] != isbbeg)
    return 16;  //��� ���� ������ ��� �� issymbol
  unsigned int i = goal[gptr];//������ � ������� buf ����������� �� (
  recordsconst* ptr;
  if (buf[--i] > isbase)
    return 17;
  ptr = heap->GetPrecordsconst(buf[i]);
  //(recordsconst *)&heap->heaps[buf[i]];
  if (ptr->ident != issymbol)
    return 17;
  i += 2;
  if ((bptr - i) % 2)
    return 38;
  unsigned int n = (bptr - i) / 2;//����� ����������
  unsigned int ptrargs[1024];
  if (n > sizeof(ptrargs) / sizeof(unsigned int))
    return 2;
  /*
  unsigned int *ptrargs = (unsigned int *)calloc(n, sizeof(unsigned int));
  if (!ptrargs)
    return 2;
  */
  for (unsigned int j = 0; j < n; j++)
    if ((ptrargs[j] = buf[i + j * 2]) > isbase)
      err = 38;
  //��������� � ������
  unsigned int index = heap->apend(ptrargs, sizeof(unsigned int) * n);
  i -= 2;
  if (index < 0)
    err = 44;
  //::free(ptrargs);
  if (err)
    return err;
  recordfunction ptrf((unsigned char)n, buf[i], index);
  index = heap->apend(&ptrf, sizeof(recordfunction));
  if (index < 0)
    return 44;
  bptr = i;
  if (bptr + 1 < _maxbptr_)
    buf[bptr++] = index;
  else
    err = 22;
  return err;
}


int clause(TScVar* ScVar, array* heap)
{
  int err = 0;
  bool& Query = ScVar->Query;
  bool& EndOfClause = ScVar->EndOfClause;
  unsigned int& gptr = ScVar->gptr;
  unsigned int* buf = ScVar->buf;
  unsigned int& novar = ScVar->novar;
  unsigned char id = Query ? (unsigned char)isclauseq :
    (unsigned char)isclause;
  if (gptr != NULL) err = 16;
  //!!! ����� ����� �������� �� ������ ������
  baserecord* tp;
  unsigned int ntarget = 1;//����� �����
  int i = 1, j = 0;
  int n = 0;
  while (buf[n] != isend) n++;
  //�� ������ ����� ������ ���� ���� ��������� ��� func
  if (buf[0] >= heap->last)
  {
    err = 20;//������ ��� ������� �����
  }
  else
  {
    tp = heap->GetPbaserecord(buf[0]);
    //(baserecord *)&heap->heaps[buf[0]];
    if (tp->ident != isfunction && tp->ident != issymbol)
      err = 20;
  }
  while (i < n && !err)
  {
    if (buf[i] == iscomma || (buf[i] == isimpl && i == 1));
    else
      err = 20;
    if (buf[i - 1] >= heap->last)
      err = 20;
    else
    {
      tp = heap->GetPbaserecord(buf[i - 1]);
      //(baserecord *)&heap->heaps[buf[i - 1]];
      unsigned char ident = tp->ident;
      if (ident != isfunction && ident != issymbol)
        err = 20;
    }
    i += 2;
  }
  i = 1;
  while (buf[i] != isend && !err)
  {
    if (buf[++i] > isbase)
      err = 20;
    i++;
    ntarget++;
  }
  if (!err)
  {
    unsigned int ptarget[1024];
    unsigned int index;
    if ((ntarget + 1) * sizeof(unsigned int) > sizeof(ptarget))
      return 2;
    index = heap->apend(ptarget,
      sizeof(unsigned int) * (ntarget + 1));
    if (index < 0)
      return 44;
    unsigned int* _ptarget = heap->GetPunsigned(index);
    //(unsigned int *)&heap->heaps[index];
    recordclause* ptrpred = 0;
    recordclause* ptr = new recordclause(id,
      (unsigned int)0, novar, buf[0], index);
    index = heap->apend(ptr, sizeof(recordclause));
    if (ptr) delete ptr;
    if (index < 0)
      return 44;
    ptr = heap->GetPrecordclause(index);
    //(recordclause *)&heap->heaps[index];
      //������ ��������� ����
    i = 0; j = 0;
    while (j < (int)ntarget)
    {
      *(_ptarget + j++) = buf[i];
      i += 2;
    }
    *(_ptarget + j) = NULL;
    //�8������� ���������� ����������� �����������
    if (!Query)
    {
      // i=j=heap->indexOf(ptr)-1;j++;i--;//j-������ ������� ����������� � heap
      //� ptr - ��������� �� ����������� � index ������ ������� �����������
      recordfunction* pf = heap->GetPrecordfunction(ptr->head);
      //(recordfunction *)&heap->heaps[ptr->head];
      recordsconst* ps;
      if (pf->ident == issymbol)
        ps = (recordsconst*)pf;
      else
        ps = heap->GetPrecordsconst(pf->func);
      //(recordsconst *)&heap->heaps[pf->func];
      if (ps->begin != NULL && ps->begin != isnil)
      {
        ptrpred = heap->GetPrecordclause(ps->begin);
        //(recordclause *)&heap->heaps[ps->begin];
        while (ptrpred->next != NULL && ptrpred->next != isnil)
          ptrpred = heap->GetPrecordclause(ptrpred->next);
        //(recordclause *)&heap->heaps[ptrpred->next];
        ptrpred->next = index;
      }
      else
        ps->begin = index;
    }
    else
      heap->query = index;
  }

  if (!err)
    EndOfClause = true;

  return err;
}
//=======
int strings(char*& p, TScVar* ScVar, array* heap)
{
  int err = 0;
  p++;
  int i;
  for (i = 0; (*(p + i) && *(p + i) != '\"'); i++);
  if (*(p + i) == '\"')
    err = wrsconst(p, i, ScVar, heap);//err=savestring(i);
  else
    err = 3;
  p += i;
  return err;
}



int num(char*& p, TScVar* ScVar, array* heap)
//������ ������ ������ ����� ��� ���� + -
{
  int err = 0;
  unsigned int& bptr = ScVar->bptr;
  unsigned int* buf = ScVar->buf;
  char bufnum[1024];
  float valuef = 0;
  int valuei = 0;
  int punkt = 0, e = 0;
  int i = 0;
  if (*p == '+' || *p == '-')
    bufnum[i++] = *p;
  for (; ((isdigitrus(bufnum[i] = *(p + i))) || (*(p + i) == 'e' && punkt) ||
    (*(p + i) == 'E' && punkt)
    || (*(p + i) == '-' && e) || (*(p + i) == '+' && e)
    || (*(p + i) == '.')); i++)
  {
    if ((*(p + i) == 'e' || *(p + i) == 'E') && punkt)
      e++;
    if (*(p + i) == '.')
      punkt++;
  }
  if (punkt > 1 || e > 1) err = 4;
  if (!err)
  {
    unsigned int index;
    bufnum[i] = 0;
    if (punkt || e) {
      //std::from_chars(bufnum, bufnum + sizeof(bufnum), valuef); // � gcc �� �����������
      valuef = static_cast<float>(atof(bufnum));
    }
    else {
      std::from_chars(bufnum, bufnum + sizeof(bufnum), valuei);
      //valuei = atoi(bufnum);
    }
    if ((valuef == valuei) && (i > 1 ||
      (i == 1 && (*p != '0'))))
      err = 4;//�� ������ ������ �����
    else
    {
      if (punkt || e)
      {
        recordfloat ptr(valuef);
        index = heap->apend(&ptr, sizeof(recordfloat));
        if (index < 0)
          err = 44;
        else
          if (bptr + 1 < _maxbptr_)
            buf[bptr++] = index;
          else err = 22;
      }
      else
      {
        recordinteger ptr(valuei);
        index = heap->apend(&ptr, sizeof(recordinteger));
        if (index < 0)
          return 44;
        if (bptr + 1 < _maxbptr_)
          buf[bptr++] = index;
        else
          err = 22;
      }
    }
  }
  p += i - 1;
  return err;
}

bool isalphaang(char symb)
{
  if ((symb >= 'A' && symb <= 'Z') ||
    (symb >= 'a' && symb <= 'z'))
    return true;
  return false;
}

bool isalphanumang(char symb)
{
  if ((symb >= 'A' && symb <= 'Z') ||
    (symb >= 'a' && symb <= 'z') ||
    (symb >= '0' && symb <= '9'))
    return true;
  return false;
}

bool isdigitrus(char symb)
{
  if (symb >= '0' && symb <= '9')
    return true;
  return false;
}


bool isalpharus(char symb)
{
  if ((symb >= '�' && symb <= '�') ||
    (symb >= '�' && symb <= '�') ||
    symb == '�' || symb == '�')
    return true;
  return false;
}

bool issvar(char*& p, unsigned int& len)
{
  if (isdigitrus(*p))
    return false; // first symbol must be no digit
  for (len = 0; *(p + len) != 0 &&
    (isalphanumang(*(p + len)) || isalpharus(*(p + len))
      || *(p + len) == '_'); len++);
  if (!len)
    return false;
  if (len == 1)
  {
    if (*(p + len) == '\'') len++;
    return true;
  }
  else
  {
    if (*(p + len) == '\'')
    {
      len++;
      return true;
    }
  }
  return false;
}

bool issconst(char*& p, unsigned int& len)
{
  len = 0;
  if ((*p >= '0' && *p <= '9') || *p == '-' || *p == '+')
    return false;
  for (len = 0; *(p + len) != 0 &&
    (isalphanumang(*(p + len)) || isalpharus(*(p + len))
      || *(p + len) == '_'); len++);
  if (!len)
    return false;
  if (len == 1)
  {
    if (*(p + len) == '\'') len++;
    return false;
  }
  else
  {
    if (*(p + len) == '\'')
    {
      len++;
      return false;
    }
  }
  return true;
}

int symvar(char*& p)       //����� ����� ����������
{
  return 0;
}

//============

int implic(char*& p, TScVar* ScVar, array* heap)
{
  int err = 0;

  if (*(p + 1) != '-')
    err = 6;
  else
    if (ScVar->gptr || ScVar->right)
      err = 7;
    else
      if (ScVar->bptr != 1)
        err = 8;
      else
      {
        p++;
        ScVar->right = true;
        ScVar->buf[ScVar->bptr++] = isimpl;
      }
  return err;
}
//===========
unsigned prioritet(unsigned ch)
{
  unsigned value = 0;
  switch (ch)
  {
  case ismod:  //������� �� �������
  case isdiv:  //������������� �������
  case ismult:
  case isslash:
    value = 4;	break;
  case isbbeg:
    value = 1;	break;
  case isbend:
    value = 2;	break;
  case isplus:
  case isminus:
    value = 3;	break;
  default:
    value = 0;
  }
  return value;
}

//=============
int arexpr(TScVar* ScVar, array* heap)
{
  int err = 0;
  unsigned obuf[maxlinelen];
  unsigned o;
  //unsigned ch;
  unsigned stac[maxlinelen];
  unsigned st, n, p;//������ � �����,����� ��,���������
  ScVar->bptr = ScVar->exprip;
  //========�������� ���������� � �� ���
  int sk = 0;//������
  while (ScVar->buf[ScVar->bptr] != isnil && !err)
  {
    unsigned pel = ScVar->buf[ScVar->bptr - 1];//���������� ������� � ��-� ���-��
    switch (ScVar->buf[ScVar->bptr])
    {
    case isbbeg:// '('
    {
      if (pel == isexpress || pel == isbbeg || pel == ismult ||
        pel == isslash || pel == isplus || pel == isminus ||
        pel == ismod || pel == isdiv)
        sk++;
      else
        err = 32;
    }	break;
    case isbend://')'
    {
      if ((pel == isbend || pel < isbase) && sk)
        sk--;
      else
        err = 32;
    }	break;
    case ismod:
    case isdiv:
    case ismult:
    case isslash:
    case isplus:
    case isminus:
    {
      if (pel != isbend && pel > isbase)
        err = 32;
    }	break;
    default:
      if (ScVar->buf[ScVar->bptr] < isbase &&
        (pel == isbbeg || pel == ismult || pel == isslash ||
          pel == isplus || pel == isminus ||
          pel == ismod || pel == isdiv))
        break;
      else
        err = 32;
    }
    ScVar->bptr++;
  }
  if (sk)
    err = 32;
  ScVar->bptr = ScVar->exprip;
  stac[st = 0] = 0;
  st++;
  stac[st++] = ScVar->buf[ScVar->bptr++];
  o = 0;
  n = 0;
  while (ScVar->buf[ScVar->bptr] != isnil && !err)
  {
    if (ScVar->buf[ScVar->bptr] == isbend &&
      stac[st - 1] == isbbeg)
    {
      st--;
      ScVar->bptr++;
      continue;
    }//��������� �� ������ ������
    if ((p = prioritet(ScVar->buf[ScVar->bptr])) == 0)
    {
      obuf[o++] = ScVar->buf[ScVar->bptr++];
      n++;
    }
    else
    {
      if (p <= prioritet(stac[st - 1]) && p > 1)
      {
        while (n > 1 && p <= prioritet(stac[st - 1]) && stac[st - 1])
        {
          obuf[o++] = stac[--st];
          n--;
        }
        if (p == 2 && stac[st - 1] == isbbeg)
        {
          st--;
          ScVar->bptr++;
        }
        else
          stac[st++] = ScVar->buf[ScVar->bptr++];
      }
      else
        stac[st++] = ScVar->buf[ScVar->bptr++];
    }
  }
  if (err)
    return err;
  obuf[o] = 0;
  stac[st] = 0;
  /*
  unsigned *ptrar = (unsigned *)calloc(o, sizeof(unsigned));
  if (!ptrar)
    return 2;
  for (unsigned i = 0; i < o; *(ptrar + i) = obuf[i], i++);
  unsigned index = heap->apend(ptrar, sizeof(unsigned) * o);
  ::free(ptrar);
  */
  unsigned index = heap->apend(obuf, sizeof(unsigned) * o);
  if (index < 0)
    return 44;
  recordexpression* ptr = new recordexpression(o, index);//ptrar,o-1);
  index = heap->apend(ptr, sizeof(recordexpression));
  if (ptr)
    delete ptr;
  if (index < 0)
    return 44;
  ScVar->bptr = ScVar->exprip - 1;
  ScVar->buf[ScVar->bptr++] = index;
  return err;
}


//==============
int expbeg(TScVar* ScVar, array* heap)
{
  int err = 0;                         //��������� ����������� � ������
  if (!ScVar->exprip)
  {
    ScVar->buf[ScVar->bptr++] = isexpress;
    ScVar->buf[ScVar->exprip = ScVar->bptr++] = isbbeg;
  }
  else
  {
    ScVar->buf[ScVar->bptr++] = isbend;
    ScVar->buf[ScVar->bptr] = isnil;
    err = arexpr(ScVar, heap);
    ScVar->exprip = 0;
  }
  return err;
}


int lbeg(char*& p, TScVar* ScVar, array* heap)
{
  int err = 0;
  if (ScVar->gptr <= 0)
    err = 9;
  else
  {
    if (*(p + 1) == ']')
    {
      if (ScVar->bptr + 1 < _maxbptr_)
        ScVar->buf[ScVar->bptr++] = ScVar->hpempty;
      else
        err = 22;
      p++;
    }
    else
    {
      ScVar->goal[ScVar->gptr++] = ScVar->bptr;
      ScVar->buf[ScVar->bptr++] = islbeg;
    }
  }
  return err;
}


//========
int list(TScVar* ScVar, array* heap)
{
  int err = 0;
  unsigned index;
  unsigned indexlast;
  recordlist* ptr;
  ScVar->gptr--;
  if (ScVar->gptr <= 0 || ScVar->gptr >= _maxgptr_ ||
    ScVar->buf[ScVar->goal[ScVar->gptr]] != islbeg)
    err = 14;
  else
  {
    ScVar->bptr = ScVar->goal[ScVar->gptr];
    ScVar->buf[ScVar->bptr] = heap->last;
    do
    {
      if (ScVar->buf[++ScVar->bptr] > isbase)
        err = 15;//�� ���������� ������
      recordlist pl(ScVar->buf[ScVar->bptr],
        heap->last + sizeof(recordlist));//!!! ��� �� ��������
      index = heap->apend(&pl, sizeof(recordlist));
      if (index < 0)
        err = 44;
    } while (ScVar->buf[++ScVar->bptr] == iscomma && !err);
    if (!err)
    {
      switch (ScVar->buf[ScVar->bptr])
      {
      case islend:
      {	recordemptylist ptre;
      indexlast = index;
      index = heap->apend(&ptre, sizeof(recordemptylist));
      if (index < 0)
        return 44;
      recordlist* prl = heap->GetPrecordlist(indexlast);
      prl->link = index;
      }	break;
      case isstick:
        if (ScVar->buf[++ScVar->bptr] > isbase ||
          ScVar->buf[ScVar->bptr + 1] != islend)
          err = 15;// �� ������ ��������� ������
        else
        {
          ptr = heap->GetPrecordlist(index);
          //(recordlist*)&heap->heaps[index];
//						recordlist prl(ScVar->buf[ScVar->bptr], ScVar->hpempty);
//						index = heap->apend(&prl, sizeof(recordlist));
          ptr->link = ScVar->buf[ScVar->bptr];
        }
        break;
      }
    }
  }
  ScVar->bptr = ScVar->goal[ScVar->gptr] + 1;
  return err;
}

//========

int arsgn(TScVar* ScVar, unsigned int i)
{
  int err = 0;
  if (!ScVar->exprip)
    err = 18;
  else
    ScVar->buf[ScVar->bptr++] = i;
  return err;
}

//=====������� ��� ������ ========================================
//  ��� ����������� �������� � �����
int variabletable(char*& p, unsigned int len, TScVar* ScVar,
  array* heap)
{
  int err = 0;
  recordvar* ptr;
  unsigned int k;
  unsigned int& novar = ScVar->novar;
  unsigned int* tvar = ScVar->tvar;
  unsigned int* buf = ScVar->buf;
  unsigned int& bptr = ScVar->bptr;
  for (k = 0; k < novar; k++)
  {
    ptr = heap->GetPrecordvar(tvar[k]);
    //(recordvar *)&heap->heaps[tvar[k]];
    char* name = heap->GetPchar(ptr->ptrsymb);
    //(char *)&heap->heaps[ptr->ptrsymb];
    if (((unsigned char)len == ptr->length) &&
      (!strncmp(name, p, len)))
      break;
  }
  if (k == novar)//����� ���������� ���
  {
    unsigned index = heap->apend(p, len);
    if (index < 0) return 42;
    recordvar ptr(index, (unsigned char)len, novar);// ������ ���������� ����� � 1
    index = heap->apend(&ptr, sizeof(recordvar));
    if (index < 0) return 44;
    tvar[novar++] = index;
  }
  if (bptr + 1 < _maxbptr_)
    buf[bptr++] = tvar[k];
  else
    return 13;
  return 0;
}

int wrsconst(char*& p, unsigned int len, TScVar* ScVar,
  array* heap)
{
  int err = 0;
  unsigned int& bptr = ScVar->bptr;
  unsigned int* buf = ScVar->buf;
  unsigned int& nosymbol = ScVar->nosymbol;
  unsigned int* tat = ScVar->tat;
  recordsconst* ptr;
  unsigned int k;
  for (k = 0; k < nosymbol; k++)
  {
    ptr = heap->GetPrecordsconst(tat[k]);
    //(recordsconst *)&heap->heaps[tat[k]];
    char* name = heap->GetPchar(ptr->ptrsymb);
    //(char *)&heap->heaps[ptr->ptrsymb];
    if (((unsigned char)len == ptr->length) &&
      (!strncmp(name, p, len))) break;
  }
  if (k == nosymbol)//����� sconst ���
  {
    unsigned index = heap->apend(p, len);
    if (index < 0) return 44;
    //        strncpy(psconst,lptr,i);
    recordsconst ptr(index, (unsigned char)len);
    index = heap->apend(&ptr, sizeof(recordsconst));
    if (index < 0)
      return 44;
    if (nosymbol + 1 < _maxsymbol_)
      tat[nosymbol++] = index;
    else err = 23;
  }
  if (!err)
  {
    if (bptr + 1 < _maxbptr_)
      buf[bptr++] = (tat[k] == hpmod) ? ismod :
      (tat[k] == hpdiv) ? isdiv : tat[k];
    else err = 22;
  }
  return err;
}
