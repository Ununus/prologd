#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <charconv>
#include <random>
#include <thread>
#include "pdefs.h"
#include "pstructs.h"
#include "extfunc.h"
#include "control.h"
#include "functions.h"

extern void Rectangle(int x1, int y1, int x2, int y2, unsigned color);
extern void MoveTo_LineTo(int x1, int y1, int x2, int y2, unsigned color);
extern void FloodFill(int x, int y, unsigned color);
extern void vertical(int x1, int y, int x2, int color);
extern void horisontal(int x, int y1, int y2, int color);
extern void ClearView(unsigned color = 15);
extern void SetPixel(int x, int y, unsigned color);
extern unsigned GetPixel(int x, int y);
extern void Ellipse(int x1, int y1, int x2, int y2, unsigned color);
extern int InputStringFromDialog(char* Buf, size_t BufSize,
  char* pCaption);

unsigned argnull(unsigned name, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  switch (name)
  {
  case hpfail:
    return 5;//ложь
  case hptrace:
    ClVar->PrSetting->Trace = true;
    //            sroptionsout.optionsrun |= tracce;//трасса
    break;
  case hpnottr:
    ClVar->PrSetting->Trace = false;
    //sroptionsout.optionsrun = (soptionsout.optionsrun | tracce) ^ tracce;
    break;
  case hpcut:
    ClVar->scptr = ClVar->parent;       //"!"
    //   zero();
    if (ClVar->parent)
      to_stac(ClVar->st_con, ClVar->parent - 1, isnil);
  }
  return 3;
}

//extern unsigned *goal;
//extern unsigned frame2;
//extern occur_term(unsigned *,unsigned *);
void conarg(unsigned numb, unsigned h, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  unsigned i, term, frame;
  recordfunction* pf = heap->GetPrecordfunction(h);
  //(recordfunction*)&heap->heaps[h];

  unsigned* ptr = heap->GetPunsigned(pf->ptrarg);
  //(unsigned *)&heap->heaps[pf->ptrarg];
  for (i = 0; i < numb; i++)
  {
    frame = ClVar->frame2;
    pf = heap->GetPrecordfunction(h);
    //(recordfunction*)&heap->heaps[h];
    ptr = heap->GetPunsigned(pf->ptrarg);
    //(unsigned *)&heap->heaps[pf->ptrarg];
    term = ptr[i];
    ScVar->goal[i] = occur_term(&term, &frame, ClVar, heap);
    ScVar->goal[maxarity + i] = term;
    ScVar->goal[maxarity * 2 + i] = frame;
  }

}
//====================================содрано=============

//extern unsigned head;
//extern unsigned frame2;
//extern unsigned newclause;
//extern char *wr_line;
//extern char *wptr;
char* occ_line(int x, char* lnwr, TScVar* ScVar, TClVar* ClVar, array* heap)
  //получение строки связанной с переменной
{
  char* ad;
  recordstring* ps = heap->GetPrecordstring(ScVar->goal[maxarity + x]);
  //(recordstring*)&heap->heaps[ScVar->goal[maxarity + x]];
  ad = heap->GetPchar(ps->ptrsymb);
  //(char *)&heap->heaps[ps->ptrsymb];
  for (x = 0; x < static_cast<int>(ps->length); x++)
    *(lnwr + x) = *(ad + x);
  *(lnwr + x) = 0;

  return lnwr;
}

int prcall(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)
// выполнение предиката ВЫП
{

  unsigned w;
  switch (sw)
  {
  case 8:
  {   recordfunction* p = heap->GetPrecordfunction(ScVar->goal[maxarity]);
  //(recordfunction *)&heap->heaps[ScVar->goal[maxarity]];
  w = p->func;
  }   break;
  case 4:
    w = ScVar->goal[maxarity];             // symbol
    break;

  default:
    outerror(24);
    return 1;//r_t_e(41);                // невыполнимая функция
  }
  if (w < heap->freeheap)
  {
    if (w == hpassrt)
    {
      outerror(24);
      return 1;
    } //r_t_e(41);
    ClVar->head = ScVar->goal[maxarity];
    ClVar->frame2 = ScVar->goal[2 * maxarity];
    return 6;            // встроенный предикат
  }

  // невстроенный предикат под call
  recordsconst* ps = heap->GetPrecordsconst(w);
  //(recordsconst *)&heap->heaps[w];
  if (ps->begin && ps->begin != isnil)
  {
    ClVar->newclause = ps->begin;
    if (ClVar->newclause)
    {
      heap->pnclause = heap->GetPrecordclause(ClVar->newclause);
      //(recordclause *)&heap->heaps[ClVar->newclause];
      heap->pncltarget = heap->GetPunsigned(heap->pnclause->ptrtarget);
      //(unsigned *)&heap->heaps[heap->pnclause->ptrtarget];
    }
    return 4;
  }
  else
  {
    outerror(24);
    return 1;
  }//r_t_e(41) не выполнимый предикат ВЫП
  return 1;
}    //ok

unsigned zap3(const char* str, unsigned arg, TScVar* ScVar, TClVar* ClVar, array* heap)
  //запись строки и унификация его с arg
{//unsigned index=heap->apend(str,strlen(str));
  if (!str)
  {
    outerror(45);
    return 5;
  }
  int l;
  unsigned bakindex = heap->last;
  unsigned index = heap->apend(const_cast<char*>(str), l = strlen(str));
  if (!index)
  {
    outerror(44);
    return 5;
  }
  recordstring rs(index, (unsigned char)l);
  index = heap->apend(&rs, sizeof(recordstring));
  if (!index)
  {
    outerror(44);
    return 5;
  }
  recordfunction* pfunction = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction*)&heap->heaps[ClVar->head];
  unsigned* ptr = heap->GetPunsigned(pfunction->ptrarg);
  //(unsigned*)&heap->heaps[pfunction->ptrarg];
  if (unify(index, ptr[arg - 1], ClVar->frame2, ClVar->frame2, ClVar, heap))
    return 3;
  heap->last = bakindex;
  return 5;
}

unsigned zap1(int num, unsigned arg, TScVar* ScVar, TClVar* ClVar, array* heap)
//унификация целого num с arg аргументом предиката
{
  recordinteger pi(num);
  unsigned bakindex = heap->last;
  unsigned index = heap->apend(&pi, sizeof(recordinteger));
  if (!index)
  {
    outerror(44);
    return 5;
  }
  // unsigned *ptr=(unsigned*)heap->heaps[head-1];
  recordfunction* pfunction = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction*)&heap->heaps[ClVar->head];
  unsigned* ptr = heap->GetPunsigned(pfunction->ptrarg);
  //(unsigned*)&heap->heaps[pfunction->ptrarg];
  if (unify(index, ptr[arg - 1], ClVar->frame2, ClVar->frame2, ClVar, heap))
    return 3;
  heap->last = bakindex;
  return 5;
}

unsigned zap2(int num1, int num2, int arg1, int arg2, TScVar* ScVar, TClVar* ClVar, array* heap)
//унификация целого num1 с arg1 аргументом предиката
//унификация целого num2 с arg2 аргументом предиката
{
  recordinteger pi1(num1);
  recordinteger pi2(num2);
  unsigned bakindex = heap->last;
  unsigned index1 = heap->apend(&pi1, sizeof(recordinteger));
  unsigned index2 = heap->apend(&pi2, sizeof(recordinteger));
  if (!index1 || !index2)
  {
    outerror(44);
    return 1;
  }
  // unsigned *ptr=(unsigned*)&heap->heaps[head-1];
  recordfunction* pfunction = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction*)&heap->heaps[ClVar->head];
  unsigned* ptr = heap->GetPunsigned(pfunction->ptrarg);
  //(unsigned*)&heap->heaps[pfunction->ptrarg];
  if (unify(index1, ptr[arg1 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap) &&
    unify(index2, ptr[arg2 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap))
    return 3;
  heap->last = bakindex;
  return 5;
}

unsigned zap1f(float num, unsigned arg, TScVar* ScVar, TClVar* ClVar, array* heap)
  //унификация float num с arg аргументом предиката
{
  recordfloat pf(num);
  unsigned bakindex = heap->last;
  unsigned index = heap->apend(&pf, sizeof(recordfloat));
  if (!index)
  {
    outerror(44);
    return 5;
  }
  // unsigned *ptr=(unsigned*)&heap->heaps[head-1];
  recordfunction* pfunction = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction*)&heap->heaps[ClVar->head];
  unsigned* ptr = heap->GetPunsigned(pfunction->ptrarg);
  //(unsigned*)&heap->heaps[pfunction->ptrarg];
  if (unify(index, ptr[arg - 1], ClVar->frame2, ClVar->frame2, ClVar, heap))
    return 3;
  heap->last = bakindex;
  return 5;
}

unsigned zap2f(float num1, float num2, int arg1, int arg2, TScVar* ScVar, TClVar* ClVar, array* heap)
//унификация float num1 с arg1 аргументом предиката
//унификация float num2 с arg2 аргументом предиката
{
  recordfloat pf1(num1);
  recordfloat pf2(num2);
  unsigned bakindex = heap->last;
  unsigned index1 = heap->apend(&pf1, sizeof(recordfloat));
  unsigned index2 = heap->apend(&pf2, sizeof(recordfloat));
  if (!index1 || !index2)
  {
    outerror(44);
    return 1;
  }
  //  unsigned *ptr=(unsigned*)&heap->heaps[head-1];
  recordfunction* pfunction = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction*)&heap->heaps[ClVar->head];
  unsigned* ptr = heap->GetPunsigned(pfunction->ptrarg);
  //(unsigned*)&heap->heaps[pfunction->ptrarg];
  if (unify(index1, ptr[arg1 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap) &&
    unify(index2, ptr[arg2 - 1], ClVar->frame2, ClVar->frame2, ClVar, heap))
    return 3;
  heap->last = bakindex;
  return 5;
}

int prrandom(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)
// выполнение предиката СЛУЧ
{
  int n, m;
  static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  static std::uniform_int_distribution<int> dst;
  switch (sw)
  {
  case 7:   // целое: инициализация генератора
    n = occ(0, ScVar, ClVar, heap);
    rng = std::mt19937(n);
    return 3;
  case 5:   // переменная: получить новое значение
    return zap1(rng(), 1, ScVar, ClVar, heap);
  case 577:   // переменная: получить новое значение
    n = occ(1, ScVar, ClVar, heap);
    m = occ(2, ScVar, ClVar, heap);
    if (n > m) {
      outerror(24); // TODO: error
      return 1;
    }
    dst = std::uniform_int_distribution<int>(n, m);
    return zap1(dst(rng), 1, ScVar, ClVar, heap);
  default:
    outerror(24); return 1;//!!!r_t_e(71);
  }
}

/*
bool see( void ) { return false; };
bool mytell( void ){ return false; };
*/
//=========================конец содранного
unsigned outfile(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  char str[129];

  switch (sw)
  {
  case 9:
  case 4: //char const
    occ_line(0, str, ScVar, ClVar, heap);
    ClVar->PrSetting->out.close();
    ClVar->PrSetting->name_out_file.clear();
    if (str[0] == 0) {
      return 3;
    }

    //АВ: пытаемся открыть файл для записи.
    ClVar->PrSetting->out.open(str);
    if (!ClVar->PrSetting->out.is_open()) {
      outerror(43);
      return 5; //r_t_e_(не могу открыть файл)
    }
    ClVar->PrSetting->name_out_file = std::string(str);
    return 3;
  case 5: //char* ps=newStr(namefileout);
    return zap3(ClVar->PrSetting->name_out_file.c_str(), 1,
      ScVar, ClVar, heap);

  default:
    return 1;//r_t_e(45); ошибка при открытии файла
  }
  return 1;
}

unsigned infile(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  char str[129];

  switch (sw)
  {
  case 9:
  case 4:          // char const
    occ_line(0, str, ScVar, ClVar, heap);
    ClVar->PrSetting->in.close();
    ClVar->PrSetting->name_in_file.clear();
    if (str[0] == 0) {
      break;
    }

    ClVar->PrSetting->in.open(str);
    if (!ClVar->PrSetting->in.is_open())
    {
      outerror(43);
      return 5; //r_t_e_(не могу открыть файл)
    }
    ClVar->PrSetting->name_in_file = std::string(str);
    return 3;

  case 5: //char* ps=newStr(namefilein);
    return zap3(ClVar->PrSetting->name_in_file.c_str(), 1, ScVar, ClVar, heap);

  default:
    return 1;//r_t_e(45); ошибка при открытии файла
  }
  return 1;
}




int occ(unsigned x, TScVar* ScVar, TClVar* ClVar, array* heap)
//получение целого связанного с переменной
{
  recordinteger* pint =
    heap->GetPrecordinteger(ScVar->goal[maxarity + x]);
  //(recordinteger *)&heap->heaps[ScVar->goal[maxarity + x]];
  return pint->value;
}

float occf(unsigned x, TScVar* ScVar, TClVar* ClVar, array* heap)
//получение вещесв связанного с переменной
{
  recordfloat* pf = heap->GetPrecordfloat(ScVar->goal[maxarity + x]);
  //	(recordfloat *)&heap->heaps[ScVar->goal[maxarity + x]];
  return pf->value;
}

unsigned priocod(unsigned sw, TScVar* ScVar, TClVar* ClVar,
  array* heap)
{
  switch (sw)
  {
  case 7://целое !!!посмотреть на ввод вещественных этого делать нельзя
  {
    if (ClVar->PrSetting->out.is_open()) {
      ClVar->PrSetting->out << char(occ(0, ScVar, ClVar, heap));
    }
    return 3;
  }
  case 5://переменная
  {
    char w;
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return 5;
      }
    }
    InputSymbol(&w);
    return zap1(w, 1, ScVar, ClVar, heap);
  }
//  case 1://анонимка
//    if (ClVar->PrSetting->fin)
//      fgetc(ClVar->PrSetting->fin);//сдвинуть указатель файла ???
//    return 3;
  default:
    outerror(24);
    return 1;//r_t_e(44);
  }
  return 1;
}

//ввод целого   !!!нужно ввод вещств
//АВ: ВВОДЦЕЛ
unsigned prrdint(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  int w{};
  char str0[129]{};
  const char* caption = "Введите целое";

  switch (sw)
  {
  case 7: // Целое
  case 74: // Целое, "caption"
  case 79: // Целое, caption
  {   // Если введённое число совпало с аргументом то ИСТИНА иначе ЛОЖЬ
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return 5;
      }
    }
    else
    {
      if (sw == 74 || sw == 79) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (InputInt(&w, str0)) {
          return 5;
        }
      } else {
        if (InputInt(&w, caption)) {
          return 5;
        }
      }
    }
    return (occ(0, ScVar, ClVar, heap) == w) ? 3 : 5;
  }
  case 5:  // Переменная
  case 54: // Переменная, "caption"
  case 59: // Переменная, caption

  case 1:
  case 14:
  case 19:
  {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in >> w)) {
        return 5;
      }
    }
    else {
      if (sw == 54 || sw == 59 || sw == 14 || sw == 19) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (InputInt(&w, str0)) {
          return 5;
        }
      } else {
        if (InputInt(&w, caption)) {
          return 5;
        }
      }
    }
    return zap1(w, 1, ScVar, ClVar, heap);
  }
  default:
    break;
  }
  outerror(24);
  return 1; // rte
}

unsigned prrdsym(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)   //вводсимв
{
  char str0[255]{}, str1[255]{}, str2[255]{};
  switch (sw)
  {
  case 9:
  case 4:
  case 99: /*то же самое что и предыдущий пункт но с заголовком диалогового окна*/
  case 94:
  case 49:
  case 44:
  {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in.get(str2, 255))) {
        break;
      }
    }
    else
    {
      if (sw == 99 || sw == 94 || sw == 49 || sw == 44) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputstring(str2, sizeof(str2), str0)) { //по cancel вернет 1;
          break;
        }
      } else {
        if (Inputstring(str2, sizeof(str2))) { //по cancel вернет 1;
          break;
        }
      }
    }
    return (strcmp(occ_line(0, str1, ScVar, ClVar, heap), str2)) ? 5 : 3;
  }
  case 5:
  case 59:
  case 54:
  {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in.get(str2, 255))) {
        break;
      }
    }
    else
    {
      if (sw == 59 || sw == 54) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputstring(str2, sizeof(str2), str0)) { //по cancel вернет 1;
          break;
        }
      } else {
        if (Inputstring(str2, sizeof(str2))) { //по cancel вернет 1;
          break;
        }
      }
    }
    return zap3(str2, 1, ScVar, ClVar, heap);
  }
  case 1: //анонимка
  case 19:
  case 14:
  {
    if (ClVar->PrSetting->in.is_open()) {
      if (!(ClVar->PrSetting->in.get(str2, 255))) {
        break;
      }
    }
    else
    {
      if (sw == 19 || sw == 14) {
        occ_line(1, str0, ScVar, ClVar, heap);
        if (Inputstring(str2, sizeof(str2), str0)) { //по cancel вернет 1;
          break;
        }
      } else {
        if (Inputstring(str2, sizeof(str2))) { //по cancel вернет 1;
          break;
        }
      }
    }
    return 3;
  }
  default:
    outerror(24);
    return 1;//r_t_e
  }
  return 5;
}

unsigned print(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap) //преобразование в целое
{
  switch (sw)
  {
  case 7:
    return 3;    //уже целое
  case 6:
    return zap1(static_cast<int>(occf(0, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  }
  outerror(36);
  return 1;
}

unsigned prfloat(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap) //преобразование в float
{
  switch (sw)
  {
  case 7:
    return zap1f(static_cast<float>(occ(0, ScVar, ClVar, heap)), 1, ScVar, ClVar, heap);
  case 6:
    return 3;
  }
  outerror(37);
  return 1;
}

unsigned prwait(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap) // жди
{
  switch (sw)
  {
  case 7:
    break;
  default:
    outerror(24);
    return 1;
  }
  int n = occ(0, ScVar, ClVar, heap);
  if (n > 0) {
    std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(n));
  }
  return 3;
}

unsigned argone(unsigned name, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  unsigned sw;
  conarg(1, ClVar->head, ScVar, ClVar, heap);
  sw = ScVar->goal[0];
  switch (name)
  {
  case hpcall: return prcall(sw, ScVar, ClVar, heap); // ВЫП
  case hprdsym: return prrdsym(sw, ScVar, ClVar, heap); // ВВОДСИМВ
  case hprdint: return prrdint(sw, ScVar, ClVar, heap); // ВВОДЦЕЛ
  case hpiocod: return priocod(sw, ScVar, ClVar, heap); // ВВКОД
  case hpsee: return infile(sw, ScVar, ClVar, heap);  // ЧТЕНИЕ_ИЗ

  case hptell: return outfile(sw, ScVar, ClVar, heap); // ЗАПИСЬ_В
  case hpvar: return (sw == 5) ? 3 : 5;  // ПЕР
  case hpint: return (sw == 7) ? 3 : 5;  // ЦЕЛ
  case hpfloat: return (sw == 6) ? 3 : 5;  //вещественное
  case hpsym: return (sw == 4) ? 3 : 5;  // СИМВ

  case hpwait: return prwait(sw, ScVar, ClVar, heap);  // ЖДИ

  case hprand: return prrandom(sw, ScVar, ClVar, heap); // СЛУЧ
  case hp_int: return print(sw, ScVar, ClVar, heap);  //преобразование int
  case hp_float: return prfloat(sw, ScVar, ClVar, heap); //преобразование float
  }
  return 1;
}

unsigned prgt(TScVar* ScVar, TClVar* ClVar, array* heap)
{
  int a[2];
  int ind = 0;
  float af[2];
  for (int i = 0; i < 2; i++)
  {
    switch (ScVar->goal[i])
    {
    case 7:
      ind += 1 * (i + 1);
      a[i] = occ(i, ScVar, ClVar, heap);
      break; //ind==1 || 2
    case 6:
      ind += 3 * (i + 1);
      af[i] = occf(i, ScVar, ClVar, heap);
      break;//ind==3 || 6
    default:
      ind = 10;
    }
  }
  switch (ind)
  {
  case 3:
    return (a[0] > a[1]) ? 3 : 5;
  case 7:
    af[0] = (float)a[0];
    break;
  case 5:
    af[1] = (float)a[1];
    break;
  case 9: break;
  default:
    outerror(24);   return 1;//r_t_e(51)//в больше не число
  }
  return (af[0] > af[1]) ? 3 : 5;
}

int GetStrFromList(char* Buf, size_t BufSize, baserecord* tp,
  TScVar* ScVar, TClVar* ClVar, array* heap)
{//составить строку символов из элементов tp - tp должен быть list 

  char* p = Buf;
  int len = 0;
  baserecord* pb;
  recordlist* plist;
  while (tp->ident == islist)
  {
    plist = (recordlist*)tp;
    pb = heap->GetPbaserecord(plist->head);
    switch (pb->ident)
    {
    case isvar:
    {
      unsigned _Term = plist->head;
      unsigned _Frame = ClVar->frame2;
      unsigned a = occur_term(&_Term, &_Frame,
        ClVar, heap);
      pb = heap->GetPbaserecord(_Term);
      if (pb->ident == islist)
      {
        int _len =
          GetStrFromList(p, BufSize - len, pb,
            ScVar, ClVar, heap);
        if (_len < 0)
          return -1;
        len += _len;
        p += _len;
      }
      if (pb->ident == isinteger)
      {
        if (BufSize - len - 1 < 0)
        {
          outerror(44);
          return -1;
        }
        recordinteger* pint = (recordinteger*)pb;
        *(p + len) = (char)pint->value;
        len++;
      }
    }	break;
    case isemptylist:
      break;
    case islist:
    {
      int _len = GetStrFromList(p, BufSize - len, pb,
        ScVar, ClVar, heap);
      if (_len < 0)
        return -1;
      len += _len;
      p += _len;
    }	break;
    case isinteger:
    {	if (BufSize - len - 1 < 0)
    {
      outerror(44);
      return -1;
    }
    recordinteger* pint = (recordinteger*)pb;
    *(p + len) = (char)pint->value;
    len++;
    }	break;
    default:
    {
      outerror(15);
      return -1;
    }//R_t_e последний не []
    }
    tp = heap->GetPbaserecord(plist->link);
  }
  /*
  if (tp->ident != isemptylist)
  {
    outerror(15);
    return -1;
  }//R_t_e последний не []
  */
  *(p + len) = 0;
  return len;
}

unsigned prstlst(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  unsigned k, i = 0;
  char lnwr[maxlinelen];
  baserecord* tp = 0;
  //recordlist* plist;
  //recordinteger* pint;
  //recordvar* pvar;
  switch (sw)
  {
  case 93:// string list
  case 43:// issymbol list
  case 42:// issymbol []
  case 53:// isvar list
  case 52:// isvar []
  {
    tp = heap->GetPbaserecord(ScVar->goal[maxarity + 1]);
    //(baserecord *)&heap->heaps[ScVar->goal[maxarity + 1]];
    int len = GetStrFromList(lnwr, sizeof(lnwr), tp,
      ScVar, ClVar, heap);
    /*
    while (tp->ident == islist)
    {
      plist = (recordlist *)tp;
      pvar = heap->GetPrecordvar(plist->head);
      pint = heap->GetPrecordinteger(plist->head);
        //(recordinteger *)&heap->heaps[plist->head];
      if (pvar->ident == isvar)
      {	unsigned _Term = plist->head;
        unsigned _Frame = ClVar->frame2;
        unsigned a = occur_term(&_Term, &_Frame,
          ClVar, heap);
        pint = heap->GetPrecordinteger(_Term);
        if (a == 7)
        {
          pint = heap->GetPrecordinteger(_Term);
        }
      }
      if (pint->ident != isinteger)
      {
        outerror(24);
        return 1;
      }//r_t_e нельзя преобразовать
      lnwr[i++] = (char)pint->value;
      tp = heap->GetPbaserecord(plist->link);
        //(baserecord *)&heap->heaps[plist->link];
    }
    if (tp->ident != isemptylist)
    {
      outerror(24);
      return 1;
    }//R_t_e последний не []
    */
    if (len < 0)
      return 1;
    lnwr[len] = 0;
    //char *str=newStr(lnwr);
    //char * str = lnwr;
    //if (!str) 
    //	break;
    switch (sw)
    {
    case 53: //heap->insert(str);
      return zap3(lnwr, 1, ScVar, ClVar, heap);
    case 43:
    case 93:
      char str[maxlinelen];
      occ_line(0, lnwr, ScVar, ClVar, heap);
      k = strncmp(str, lnwr, strlen(str));
      return (k == NULL) ? 3 : 5;
    }
  }
  case 95:       //   str/sym var
  case 45:
  {
    occ_line(0, lnwr, ScVar, ClVar, heap);
    k = strlen(lnwr);
    //unsigned *ptr=(unsigned *)&heap->heaps[head-1];//цели в предложении
    recordfunction* pfunction = heap->GetPrecordfunction(ClVar->head);
    //(recordfunction*)&heap->heaps[ClVar->head];
    unsigned* ptr = heap->GetPunsigned(pfunction->ptrarg);
    //(unsigned*)&heap->heaps[pfunction->ptrarg];
    if (!k)
    {
      recordemptylist pempty;
      unsigned index = heap->apend(&pempty, sizeof(recordemptylist));
      if (!index)
      {
        outerror(44);
        return 1;
      }
      unify(index, ptr[1], ClVar->frame2, ClVar->frame2, ClVar, heap);
      return 3;
    }
    else //строка не пустая
    {
      unsigned j = heap->last, index1, index2;
      for (i = 0; i < k; i++)
      {
        recordinteger pi((unsigned char)lnwr[i]); //2006/10/17
        index1 = heap->apend(&pi, sizeof(recordinteger));
        recordlist pl(index1, heap->last + sizeof(recordinteger)
          + sizeof(recordlist));
        index2 = heap->apend(&pl, sizeof(recordlist));
      }
      if (!index1 || !index2)
      {
        outerror(44);
        return 1;
      }//r_t_e нет памяти
      recordemptylist pempty;
      unsigned index = heap->apend(&pempty, sizeof(recordemptylist));
      if (!index)
      {
        outerror(44);
        return 1;
      }//r_t_e нет памяти
      recordlist* plst = heap->GetPrecordlist(index2);
      //(recordlist*)&heap->heaps[index2];
      plst->link = index;
      unify(j + sizeof(recordinteger), ptr[1],
        ClVar->frame2, ClVar->frame2, ClVar, heap);
      return 3;
    }
  }
  default:
    outerror(24);
    return 1;//R_t_e
  }
  return 5;//!!! посмотреть на код возврата
}

unsigned prstint(unsigned sw, TScVar* ScVar, TClVar* ClVar,
  array* heap)   //стрцел
{//int i;
  long w{};
  char lnwr[maxlinelen]{};
  switch (sw)
  {
  case 97:   //str int
  case 95:   //str var
  case 47:   //symb int
  case 45:   //symb var
  {
    occ_line(0, lnwr, ScVar, ClVar, heap);
    std::from_chars(lnwr, lnwr + sizeof(lnwr), w);
    //w = atol(lnwr); //!!!не сделан контроль ошибки конверитирования
    if (sw == 95 || sw == 45)
      return zap1(w, 2, ScVar, ClVar, heap);
    return (w == occ(1, ScVar, ClVar, heap)) ? 3 : 5;
  }
  case 57:   //var int  возможно нужно var float
  {
    std::to_chars(lnwr, lnwr + maxlinelen, occ(1, ScVar, ClVar, heap));
    // Заменено на to_chars, но не поверено
    //_ltoa(occ(1, ScVar, ClVar, heap), lnwr, 10);
    //char *str=newStr(lnwr);

    return zap3(lnwr, 1, ScVar, ClVar, heap);
  }
  case 56:
  {
    float value = occf(1, ScVar, ClVar, heap);
    sprintf(lnwr, "%f", value);
    //std::to_chars(lnwr, lnwr + maxlinelen, value); // в gcc не реализовано
    // Заменено на, но не поверено
    //int dec, sign, ndig = 5;
    //char* p = lnwr;
    //p = _fcvt(value, ndig, &dec, &sign);
    //int j = 0, i;
    //if (sign)
    //  lnwr[j++] = '-';
    //for (i = 0; (i < dec && i + j < maxlinelen);
    //  lnwr[j + i] = p[i], i++);
    //lnwr[j + i] = '.';
    //j++;
    //for (; (i + j < maxlinelen && p[i]); lnwr[i + j] = p[i], i++);
    //lnwr[j + i] = NULL;

    //str=newStr(lnwr);
    return zap3(lnwr, 1, ScVar, ClVar, heap);
  }
  default:
  {
    outerror(24);
    return 1;
  }//R_t_e
  }// outerror(2);return 1;//R_T_e нет памяти
}

bool digit(char a)
{
  return (isdigitrus(a) == NULL) ? false : true;
}

bool letter(char a)
{
  return (isalpharus(a) || isalphaang(a)) ? true : false;
}

unsigned whatisit(unsigned sw, bool (*f)(char), unsigned i,
  TScVar* ScVar, TClVar* ClVar, array* heap)//насчет unsigned i ???
{
  char lnwr[maxlinelen];                              //может быть unsignedi !!!
  int w;
  occ_line(0, lnwr, ScVar, ClVar, heap);
  int len = strlen(lnwr);
  switch (sw)
  {
  case 97: //str int
  case 47:
    w = occ(1, ScVar, ClVar, heap);
    if (w && len >= w)
      return (*f)(lnwr[--w]) ? 3 : 5;
    break;
  case 95://str var
  case 45:
    for (w = 0; w < len && !(*f)(lnwr[w]); w++);
    if (w < len)
      return zap1(w + 1, 2, ScVar, ClVar, heap);
    break;
  default:
    return (i == i) ? 1 : 1;//r_t_e w код ошибки (обработка цифры или буквы)
  }                // i использовать для r_t_e
  return 5;
}

unsigned prskol(unsigned sw, TScVar* ScVar, TClVar* ClVar,
  array* heap)
{
  int i = 0;
  recordsconst* ps = heap->GetPrecordsconst(ScVar->goal[maxarity]);
  //(recordsconst *)&heap->heaps[ScVar->goal[maxarity]];
  if (ps->begin != NULL && ps->begin != isnil)
  {
    recordclause* pcl = heap->GetPrecordclause(ps->begin);
    //(recordclause *)&heap->heaps[ps->begin];
    i++;
    while (pcl->next != isnil && pcl->next != NULL)
    {
      i++;
      pcl = heap->GetPrecordclause(pcl->next);
      //(recordclause *)&heap->heaps[pcl->next];
    }
  }
  switch (sw)
  {
  case 45: //str var
    return zap1(i, 2, ScVar, ClVar, heap);
  case 47: //str int
    return (i == occ(1, ScVar, ClVar, heap)) ? 3 : 5;
  }
  outerror(24);
  return 1;//r_t_e
}

unsigned prterm(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)  //терм
{
  baserecord* tp;
  recordlist* plist;
  int i = 0;
  switch (sw)
  {
  case 83:
  case 43:
  case 93:
  case 53:
  {
    tp = heap->GetPbaserecord(ScVar->goal[maxarity + 1]);
    //(baserecord *)&heap->heaps[ScVar->goal[maxarity+1]];
    if (tp->ident == islist)
    {
      plist = (recordlist*)tp;
      baserecord* test = heap->GetPbaserecord(plist->head);
      //(baserecord *)&heap->heaps[plist->head];
      if (test->ident != issymbol)
        return 5;
    }
    plist = (recordlist*)tp;
    while (tp->ident == islist)
    {
      plist = (recordlist*)tp;
      tp = heap->GetPbaserecord(plist->link);
      //(baserecord *)&heap->heaps[plist->link];
      i++;//число элементов в списке
    }
    if (tp->ident != isemptylist)
    {
      outerror(24);
      return 1;
    }//R_t_e последний не []
    if (i == 1)
    {
      recordfunction* pf = heap->GetPrecordfunction(ClVar->head);
      //(recordfunction*)&heap->heaps[ClVar->head];
      unsigned* ptr = heap->GetPunsigned(pf->ptrarg);
      //(unsigned *)&heap->heaps[pf->ptrarg];
      if (unify(ptr[0], plist->head, ClVar->frame2,
        ClVar->frame2, ClVar, heap))
        return 3;
      //				heap->last=bakindex; //??????
      return 5;
    }
    //сделать запись функции с аргументами из списка
    unsigned n = i - 1;//число целей в новой функции
    unsigned int* ptrargs = new unsigned[n];
    //!!! нужно другой код возврата
    //сообщение о том что нет памяти
    if (!ptrargs)
      return 5;
    plist = heap->GetPrecordlist(ScVar->goal[maxarity + 1]);
    //(recordlist *)&heap->heaps[ScVar->goal[maxarity + 1]];
    unsigned funcsymb = plist->head;
    i = 0;
    plist = heap->GetPrecordlist(plist->link);
    //(recordlist *)&heap->heaps[plist->link];
    while (plist->ident == islist)
    {
      ptrargs[i] = plist->head;
      plist = heap->GetPrecordlist(plist->link);
      //(recordlist *)&heap->heaps[plist->link];
      i++;//число элементов в списке
    }
    unsigned int index =
      heap->apend(ptrargs, sizeof(unsigned int) * n);
    delete[] ptrargs;
    if (!index)
    {
      outerror(44);
      return 5;
    }
    recordfunction ptrf((unsigned char)n, funcsymb, index);
    index = heap->apend(&ptrf, sizeof(recordfunction));
    if (!index)
    {
      outerror(44);
      return 5;
    }
    recordfunction* pf = heap->GetPrecordfunction(ClVar->head);
    //(recordfunction*)&heap->heaps[ClVar->head];
    unsigned* ptr = heap->GetPunsigned(pf->ptrarg);
    //(unsigned *)&heap->heaps[pf->ptrarg];

    if (unify(ptr[0], index, ClVar->frame2, ClVar->frame2, ClVar, heap))
      return 3;
    //          heap->last=bakindex; //????????
    return 5;
  }
  case 85: //func var
  {//func -> list
    recordfunction* rf = heap->GetPrecordfunction(ScVar->goal[maxarity]);
    //(recordfunction*)&heap->heaps[ScVar->goal[maxarity]];
    unsigned narg = 0;//rf->narg;
    unsigned oldindex = heap->last;
    recordlist pl(rf->func, oldindex + sizeof(recordlist));
    unsigned* ptrarg = heap->GetPunsigned(rf->ptrarg);
    //(unsigned *)&heap->heaps[rf->ptrarg];
    unsigned tlist = heap->apend(&pl, sizeof(recordlist));
    if (!tlist)
    {
      outerror(44);
      return 5;
    }
    while (narg < (unsigned)rf->narg)
    {
      pl.head = ptrarg[narg];
      pl.link = heap->last + sizeof(recordlist);
      if (!heap->apend(&pl, sizeof(recordlist)))
      {
        outerror(44);
        return 5;
      }
      narg++;
    }
    recordemptylist ptre;
    if (!heap->apend(&ptre, sizeof(recordemptylist)))
    {
      outerror(44);
      return 5;
    }
    rf = heap->GetPrecordfunction(ClVar->head);
    //(recordfunction*)&heap->heaps[ClVar->head];
    ptrarg = heap->GetPunsigned(rf->ptrarg);
    //(unsigned *)&heap->heaps[rf->ptrarg];
    return (unify(ptrarg[1], tlist,
      ClVar->frame2, ClVar->frame2, ClVar, heap)) ? 3 : 5;
  }
  case 95://symb var
  case 45://str var -над этим надо подумать ??
  {//symb -> list
    unsigned oldindex = heap->last;
    recordlist pl(ScVar->goal[maxarity],
      oldindex + sizeof(recordlist));
    unsigned tlist = heap->apend(&pl, sizeof(recordlist));
    if (!tlist)
    {
      outerror(44);
      return 5;
    }
    recordemptylist ptre;
    if (!heap->apend(&ptre, sizeof(recordemptylist)))
    {
      outerror(44);
      return 5;
    }
    recordfunction* rf = heap->GetPrecordfunction(ClVar->head);
    //(recordfunction*)&heap->heaps[ClVar->head];
    unsigned* ptrarg = heap->GetPunsigned(rf->ptrarg);
    //(unsigned *)&heap->heaps[rf->ptrarg];
    return (unify(ptrarg[1], tlist,
      ClVar->frame2, ClVar->frame2, ClVar, heap)) ? 3 : 5;
  }
  }
  return 5;
}

unsigned prdel(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  if (sw != 47)
  {
    outerror(24);
    return 1;
  }//r_t_e
  unsigned i = occ(1, ScVar, ClVar, heap);
  recordsconst* ps = heap->GetPrecordsconst(ScVar->goal[maxarity]);
  //(recordsconst *)&heap->heaps[ScVar->goal[maxarity]];
  if (ps->begin == isnil || ps->begin == NULL)
    return 5;//нет предложения
  recordclause* pcpred = 0;
  recordclause* pc = heap->GetPrecordclause(ps->begin);
  //(recordclause *)&heap->heaps[ps->begin];
  unsigned w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w++)
  {
    pcpred = pc;
    pc = heap->GetPrecordclause(pc->next);
    //(recordclause *)&heap->heaps[pc->next];
  }

  if (w == 1)//первое предложение  даже если последнее
    ps->begin = pc->next;
  else {
    if (!pcpred) {
      pcpred = heap->GetPrecordclause(ps->begin);
    }
    pcpred->next = pc->next;
  }
  return 3;
}

unsigned argtwo(unsigned name, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  unsigned sw;
  conarg(2, ClVar->head, ScVar, ClVar, heap);
  sw = ScVar->goal[0] * 10 + ScVar->goal[1];
  switch (name)
  {
  case hpgt: return prgt(ScVar, ClVar, heap);         // БОЛЬШЕ
  case hpstint: return prstint(sw, ScVar, ClVar, heap);      // СТРЦЕЛ
  case hpstlst: return prstlst(sw, ScVar, ClVar, heap);      // СТРСПИС
  case hplettr: return whatisit(sw, letter, 55, ScVar, ClVar, heap);// БУКВА
  case hpdigit: return whatisit(sw, digit, 56, ScVar, ClVar, heap);  // ЦИФРА
  case hpterm: return prterm(sw, ScVar, ClVar, heap);       // ТЕРМ    return 5;
  case hpdel: return prdel(sw, ScVar, ClVar, heap);        // УДАЛ
  case hpskol: return prskol(sw, ScVar, ClVar, heap);       // СКОЛЬКО
  case hprdsym: return prrdsym(sw, ScVar, ClVar, heap);      // ВВОДСИМВ (с заголовком)
  case hprdint: return prrdint(sw, ScVar, ClVar, heap);      // ВВОДЦЕЛ
  }
  return 1;
}

unsigned prset(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)//точка
{
  //	if ( !canvas) return 3;
  int xy, x1, x2, y1, y2, color;
  switch (sw)
  {
  case 777:
  {   x1 = occ(0, ScVar, ClVar, heap);
  x2 = x1 + 5;
  y1 = occ(1, ScVar, ClVar, heap);
  y2 = y1 + 5;
  color = occ(2, ScVar, ClVar, heap);
  SetPixel(x1, y1, color);
  /*
  if (color > -1 && color < _MaxColors_)
        {   color = colorstable[color];
        }
        canvas->Pixels[x1][y1]=(TColor)color;
  */
  }   break;
  case 775:
  {   x1 = occ(0, ScVar, ClVar, heap);
  y1 = occ(1, ScVar, ClVar, heap);
  color = GetPixel(x1, y1);
  /*
        for (int i = 0; i < _MaxColors_; i++)
            if (color == colorstable[i])
                color = i;
      */
  return zap1(color, 3, ScVar, ClVar, heap);
  }
  case 757:
  {   xy = occ(0, ScVar, ClVar, heap);
  color = occ(2, ScVar, ClVar, heap);
  x1 = xy;
  x2 = xy;
  y1 = 0;
  y2 = maxgry;
  /*
        if (color > -1 && color < _MaxColors_)
        {   color = colorstable[color];
        }
        canvas->Pen->Color=(TColor)color;
        canvas->MoveTo(x1, y1);
        canvas->LineTo(x2, y2);
  */
  vertical(x1, y1, x2, color);
  }   break;
  case 577:
  {   xy = occ(1, ScVar, ClVar, heap);
  y1 = xy;
  y2 = xy;
  x1 = 0;
  x2 = maxgrx;
  /*
        if (color > -1 && color < _MaxColors_)
        {   color = colorstable[color];
        }
        canvas->Pen->Color=(TColor)color;
        canvas->MoveTo(x1,y1);
        canvas->LineTo(x2,y2);
  */
  color = 0;
  horisontal(x1, y1, y2, color);
  }   break;
  case 557:
  {   color = occ(2, ScVar, ClVar, heap);
  /*
        x1 = 0;
  y1 = 0;
  x2 = maxgrx;
  y2 = maxgry;
        if (color > -1 && color < _MaxColors_)
        {   color = colorstable[color];
        }
        canvas->Pen->Color=(TColor)color;
        canvas->Brush->Color=canvas->Pen->Color;
        canvas->Rectangle(x1,y1,x2,y2);
  */
  ClearView(color);
  }   break;
  default:
  {
    outerror(24);
    return 1;
  }//r_t_e(не вып предик точка
  }
  return 3;
}

unsigned prapp(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)//сцеп
{
  char wrln1[maxlinelen], wrln2[maxlinelen];
  unsigned w, i;
  switch (sw)
  {
  case 444:
  case 449:
  case 494:
  case 499:
  case 944:
  case 949:
  case 994:
  case 999://все str str str
  case 445:
  case 495:
  case 945:
  case 995://str str var
    occ_line(0, wrln1, ScVar, ClVar, heap);
    occ_line(1, wrln2, ScVar, ClVar, heap);
    strcat(wrln1, wrln2);
    if ((w = strlen(wrln1)) >= maxlinelen)
    {
      outerror(24); return 1;
    }
    if (sw == 445 || sw == 495 || sw == 945 || sw == 995) //если переменная
    {
      return zap3(wrln1, 3, ScVar, ClVar, heap);
    }
    occ_line(2, wrln2, ScVar, ClVar, heap);
    return (!strncmp(wrln1, wrln2, strlen(wrln1))) ? 3 : 5;
  case 454:
  case 459:
  case 954://str var str
  case 959:
    occ_line(0, wrln1, ScVar, ClVar, heap);
    occ_line(2, wrln2, ScVar, ClVar, heap);
    w = strlen(wrln1);
    i = strlen(wrln2);
    //если lnwr1 входит в lnwr2 и причем спереди

    if (w <= i && !strncmp(wrln1, wrln2, w)) //2006/10/17
    {
      return zap3(&wrln2[w], 2, ScVar, ClVar, heap);
    }   break;
  case 544:
  case 549:
  case 594://var str str
  case 599:
    occ_line(1, wrln1, ScVar, ClVar, heap);
    occ_line(2, wrln2, ScVar, ClVar, heap);
    w = strlen(wrln1);
    i = strlen(wrln2);
    if (i > w && !strncmp(wrln1, &wrln2[i - w], w))
    {
      wrln2[i - w] = NULL;
      return zap3((char*)wrln2, 1, ScVar, ClVar, heap);
    }
    break;
  default:
    outerror(24);
    return 1;//r_t_e
  }
  return 5;
}

// две функции работают в паре. они используют общую
//глобальную переменную count_var и общий массив tvar
//первой стартует VarOnList
int count_var;//
int VarOnList(baserecord* pb, TScVar* ScVar, TClVar* ClVar, array* heap);//подсчет переменных в списке вернет 1 если ошибка

int VarOnFunc(baserecord* pb, TScVar* ScVar, TClVar* ClVar, array* heap)//подсчет переменных в функции вернет 1 если ошибка
{
  if (pb->ident != isfunction)
    return 1;//ошибка
  recordfunction* pf = (recordfunction*)pb;
  unsigned* ptrarg = heap->GetPunsigned(pf->ptrarg);
  //(unsigned *)&heap->heaps[pf->ptrarg];
  for (unsigned i = 0; i < (unsigned)pf->narg; i++)
  {
    baserecord* tp = heap->GetPbaserecord(ptrarg[i]);
    //(baserecord *)&heap->heaps[ptrarg[i]];
    switch (tp->ident)
    {
    case isfunction:
      if (VarOnFunc((baserecord*)tp, ScVar, ClVar, heap))
        return 1;//ошибка
      break;
    case islist:
      if (VarOnList((baserecord*)tp, ScVar, ClVar, heap))
        return 1;//ошибка
      break;
    case isvar:
      count_var++;
      break;
    }
  }
  return 0;
};

int VarOnList(baserecord* pb, TScVar* ScVar, TClVar* ClVar,
  array* heap)//подсчет переменных в списке вернет 1 если ошибка
{
  if (pb->ident != islist)
    return 1;
  recordlist* pl = (recordlist*)pb;
  do
  {
    recordvar* pv = heap->GetPrecordvar(pl->head);
    //(recordvar *)&heap->heaps[pl->head];
    switch (pv->ident)
    {
    case isfunction:
      if (VarOnFunc((baserecord*)pv, ScVar, ClVar, heap))
        return 1;//ошибка
      break;
    case isvar:
      count_var++;
      break;
    }
    pl = heap->GetPrecordlist(pl->link);
    //(recordlist *)&heap->heaps[pl->link];
    unsigned char ident = pl->ident;
    ident = ident + 0;
  } while (pl->ident == islist);
  return 0;
};

int VarFromFunc(baserecord* pbr);
int VarFromList(baserecord* pbr);
int VarFromFunc(baserecord* pbr)
{
  return 0;
}

//подготовка целей для предиката доб
// поместит цели в массив bpt

unsigned target_number;

unsigned int prepare_target_from_list(unsigned term, TScVar* ScVar,
  TClVar* ClVar, array* heap);

unsigned int prepare_target_from_var(unsigned term, TScVar* ScVar,
  TClVar* ClVar, array* heap)
{
  if (!term || term == isnil)
    return 1;
  unsigned error = 0;
  recordvar* pv = heap->GetPrecordvar(term);
  //(recordvar *)&heap->heaps[term];
  if (pv->ident != isvar)
    return 1;
  unsigned frame = ClVar->frame2;
  if (!occur_term(&term, &frame, ClVar, heap))
    return 1;
  if (term == isnil || !term)
    return 1;
  pv = heap->GetPrecordvar(term);
  //(recordvar *)&heap->heaps[term];
  switch (pv->ident)
  {
  case islist:
  {   error = prepare_target_from_list(term, ScVar, ClVar, heap);
  if (error != 0)
    return 1;
  break;
  }
  case isvar:
  {   error = prepare_target_from_var(term, ScVar, ClVar, heap);
  if (error != 0)
    return 1;
  break;
  }
  case issymbol:
  case isfunction:
  {   ClVar->bpt[target_number++] = term;
  }   break;
  default:
    return 1;
  }
  return 0;
}

unsigned int prepare_target_from_list(unsigned term, TScVar* ScVar,
  TClVar* ClVar, array* heap)
{
  if (!term || term == isnil)
    return 1;
  unsigned error = 0;
  recordlist* pl = heap->GetPrecordlist(term);
  //(recordlist *)&heap->heaps[term];
  if (pl->ident != islist)
    return 1;
  while (pl->ident == islist) //выбирають цели из списка
  {
    baserecord* tp = heap->GetPbaserecord(pl->head);
    //(baserecord *)&heap->heaps[pl->head];
    switch (tp->ident)
    {
    case islist:
    {   error = prepare_target_from_list(pl->head, ScVar, ClVar, heap);
    if (error != 0)
      return 1;
    break;
    }
    case isvar:
    {   error = prepare_target_from_var(pl->head,
      ScVar, ClVar, heap);
    if (error != 0)
      return 1;
    break;
    }
    case issymbol:
    case isfunction:
      ClVar->bpt[target_number++] = pl->head;
      break;
    default:
      return 1;
    }
    pl = heap->GetPrecordlist(pl->head);
    //(recordlist *)&heap->heaps[pl->link];
  }
  return 0;//ok
}

unsigned int prepare_target(unsigned term, TScVar* ScVar, TClVar* ClVar,
  array* heap)
{
  if (!term || term == isnil)
    return 1;
  baserecord* pt = heap->GetPbaserecord(term);
  //(baserecord *)&heap->heaps[term];
//    recordlist * pl=(recordlist *)&heap->heaps[term];
  if (!ClVar->bpt)
    return 1;
  target_number = 0;
  switch (pt->ident)
  {
  case islist:
    return prepare_target_from_list(term, ScVar, ClVar, heap);
  case isvar:
    return prepare_target_from_var(term, ScVar, ClVar, heap);
  default:
    return 1;
  }
  /*
      if (pl->ident!=islist)
          return 1;
      return prepare_target_from_list(term);
  */
}

//2006/10/24
void PrintFunction(recordfunction* prf, int Level, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char* p = pBuf + Level;
  sprintf(p, "PrintFunction: Level %d", Level);
  out(pBuf);
  baserecord* pbr = heap->GetPbaserecord(prf->func);
  //(baserecord *)&heap->heaps[prf->func];
  sprintf(p, "func: (%d)", prf->func);
  out(pBuf);
  PrintTerm(pbr, Level + 1, ScVar, ClVar, heap);
  unsigned* ptrarg = heap->GetPunsigned(prf->ptrarg);
  //(unsigned *)&heap->heaps[prf->ptrarg];
  sprintf(p, "arguments: (%d)", prf->narg);
  out(pBuf);
  for (int i = 0; i < static_cast<int>(prf->narg); i++)
  {
    sprintf(p, "argument %d (%d)", i, ptrarg[i]);
    out(pBuf);
    baserecord* pbr = heap->GetPbaserecord(ptrarg[i]);
    //(baserecord *)&heap->heaps[ptrarg[i]];
    PrintTerm(pbr, Level + 1, ScVar, ClVar, heap);
  }
}

void PrintRecordsconst(recordsconst* pbr, int Level, TScVar* Scvar, TClVar* ClVar, array* heap)
{
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char* p = pBuf + Level;
  sprintf(p, "Printsconst: Level %d", Level);
  char* pc = heap->GetPchar(pbr->ptrsymb);
  //(char *)&heap->heaps[pbr->ptrsymb];
  sprintf(p, "sconst: ");
  int len = strlen(p);
  strncpy(p + len, pc, pbr->length);
  *(p + len + pbr->length) = 0;
  out(pBuf);
}

void PrintVar(recordvar* pbr, int Level, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char* p = pBuf + Level;
  sprintf(p, "PrintVar: Level %d", Level);
  out(pBuf);
  char* pc = heap->GetPchar(pbr->ptrsymb);
  //(char *)&heap->heaps[pbr->ptrsymb];
  sprintf(p, "Name: ");
  int len = strlen(p);
  strncpy(p + len, pc, pbr->length);
  *(p + len + pbr->length) = 0;
  out(pBuf);
  sprintf(p, "num of clause: %d", pbr->num);
  out(pBuf);
}

void PrintInteger(recordinteger* pbr, int Level)
{
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char* p = pBuf + Level;
  sprintf(p, "PrintInteger: Level %d", Level);
  out(pBuf);
  sprintf(p, "Value: %d", pbr->value);
  out(pBuf);
}

void PrintTerm(baserecord* pbr, int Level, TScVar* ScVar,
  TClVar* ClVar, array* heap)
{
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char* p = pBuf + Level;
  sprintf(p, "PrintTerm: Level %d", Level);
  out(pBuf);
  int ident = pbr->ident;
  switch (ident)
  {
  case isfunction:
    PrintFunction((recordfunction*)pbr, Level + 1,
      ScVar, ClVar, heap);
    return;
  case issymbol:
    PrintRecordsconst((recordsconst*)pbr, Level + 1,
      ScVar, ClVar, heap);
    return;
  case isvar:
    PrintVar((recordvar*)pbr, Level + 1, ScVar, ClVar, heap);
    return;
  case isinteger:
    PrintInteger((recordinteger*)pbr, Level + 1);
    return;
  case islist:
    PrintList((recordlist*)pbr, Level + 1, ScVar,
      ClVar, heap);
    return;
  default:
    sprintf(pBuf, "unknown id of term: %d", ident);
    out(pBuf);
  }
}

void PrintList(recordlist* pl, int Level, TScVar* ScVar,
  TClVar* ClVar, array* heap)
{
  char pBuf[255];
  memset(pBuf, ' ', Level);
  char* p = pBuf + Level;
  sprintf(p, "PrintList: Level %d", Level);
  out(pBuf);
  while (pl->ident == islist)
  {
    baserecord* pb = heap->GetPbaserecord(pl->head);
    PrintTerm(pb, Level + 1, ScVar, ClVar, heap);
    pl = heap->GetPrecordlist(pl->link);
  }
}
recordfunction* FindFuncFromFunc(baserecord* pbr)
{
  recordfunction* rf = (recordfunction*)0;
  return rf;
}

recordfunction* FindFuncFromTerm(baserecord* pbr)
{
  out(const_cast<char*>("FindFundFromterm"));
  int ident = pbr->ident;
  switch (pbr->ident)
  {
  case isfunction:
  {
    return FindFuncFromFunc(pbr);
  }   break;
  }
  return 0;
}

unsigned GetConstTerm(unsigned Term, unsigned Frame,
  TClVar* ClVar, array* heap)
{
  unsigned _Term = Term;
  unsigned _Frame = Frame;
  do
  {
    baserecord* br = heap->GetPbaserecord(_Term);
    //(baserecord*)&heap->heaps[_Term];
    int ident = br->ident;
    switch (ident)
    {
    case isvar:
    {
      unsigned j;
      unsigned _PrevTerm = _Term;
      bound(&_Term, &_Frame, &j, ClVar, heap);
      if (_Term == isnil)
      {
        return _PrevTerm;
      }
    }   break;
    case issymbol:
    case isinteger:
    case isfloat:
    case isstring:
      return _Term;
    default:
      return 0;
    }
  } while (1);
}

int GetVarsFromFunction(recordvar* Vars[], int VarCount,
  recordfunction* pf, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  if (pf->ident != isfunction)
    return -1;//ошибка
  int Count = 0;
  unsigned* ptrarg = heap->GetPunsigned(pf->ptrarg);
  //(unsigned *)&heap->heaps[pf->ptrarg];
  for (unsigned i = 0; i < (unsigned)pf->narg; i++)
  {
    baserecord* tp = heap->GetPbaserecord(ptrarg[i]);
    //(baserecord *)&heap->heaps[ptrarg[i]];
    switch (tp->ident)
    {
    case isvar:
      Vars[VarCount + Count++] = (recordvar*)tp;
      break;
    case isinteger:
      break;
      /*
      case isfunction:
          if (VarOnFunc((baserecord*)tp))
              return 1;//ошибка
          break;
      case islist:
          if (VarOnList((baserecord*)tp))
              return 1;//ошибка
          break;
      */
    default:
      char _Buf[255];
      sprintf(_Buf, "GetVarCountFromFunction: unknown term %d",
        tp->ident);
      out(_Buf);
      return -1;
    }
  }
  return Count;
};

int GetVarCountFromClause(recordclause* rc, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  //    recordclause * rc = (recordclause*)&heap->heaps[index];
  int VarCount = 0;
  if (rc->ident != isclause)
  {
    out(const_cast<char*>("No clause in func GetVarCountFromCluse"));
    return -1;
  }

  unsigned* target = heap->GetPunsigned(rc->ptrtarget);
  //(unsigned *)&heap->heaps[rc->ptrtarget];
  int i = 0;
  recordvar* Vars[128];
  while (*(target + i))
  {
    int Count = 0;
    baserecord* br = heap->GetPbaserecord(*(target + i));
    //(baserecord*)&heap->heaps[*(target + i)];
    switch (br->ident)
    {
    case isfunction:
    {   Count = GetVarsFromFunction(Vars, VarCount,
      (recordfunction*)br, ScVar, ClVar, heap);
    if (Count < 0)
      return -1;
    VarCount += Count;
    }   break;
    default:
      char _Buf[255];
      sprintf(_Buf, "GetVarCountFromClause: unknown term %d",
        br->ident);
      out(_Buf);
      return -1;
    }
    i++;
  }
  int newCount = VarCount;
  /*
  for (int j = 0; VarCount - 1; j++)
  {
      for (int k = j + 1; k < VarCount; k++)
      {
          recordvar * var1 = Vars[j];
          recordvar * var2 = Vars[k];
          if (var1->ptrsymb == var2->ptrsymb && var1->num == var2->num)
              newCount--;
      }
  }
  */
  return VarCount;
}

unsigned prassrt(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)//доб
{
  unsigned old_index = heap->last;
  unsigned error = 0;
  baserecord* tp;
  recordsconst* ps;
  recordlist* pl;
  recordfunction* pf;
  recordclause* pfirstclause;//первое предложение данного типа
  recordclause* pclause;//предложение на место которого вставиться требуемое
  unsigned int index;
  unsigned int* ptarget;//указатель на массив с целями
  unsigned nvar = 0;//число переменных в предложении(глоб перем)

  unsigned ntarget = 1;//число целей в новом предложении
  switch (sw)
  {
  case 427:
  case 927:
  case 827:// _,[],int  //рассматриваем предложение как факт
                   // в новом предолжении переменных не может быть
  {
    unsigned m = maxarity;
    tp = heap->GetPbaserecord(ScVar->goal[maxarity]);
    //(baserecord *)&heap->heaps[ScVar->goal[maxarity]];
    if (tp->ident != isfunction) // может быть либо функцией или переменной,конкретизированной функцией возможно что нужно поискать функцию
    {
      out(const_cast<char*>("предикт 'ДОБ': АРГ1 не функция"));
      return 1;
    }
    recordfunction* prf = (recordfunction*)tp;
    unsigned int* parg = new unsigned[prf->narg];
    if (!parg)
    {
      outerror(2);
      return 5;
    }
    unsigned* prf_arg = heap->GetPunsigned(prf->ptrarg);
    //(unsigned *)&heap->heaps[prf->ptrarg];
    for (int i = 0; i < static_cast<int>(prf->narg); i++)
    {
      //достать константы из агрументов функции.
      unsigned Arg = GetConstTerm(*(prf_arg + i),
        ClVar->frame2, ClVar, heap);
      *(parg + i) = Arg;
    }
    unsigned ArgIndex = heap->apend(parg, sizeof(unsigned) * prf->narg);
    delete[] parg;
    if (!ArgIndex)
    {
      outerror(44);
      return 5;
    }
    recordfunction NewFunction(prf->narg, prf->func, ArgIndex);
    unsigned findex = heap->apend(&NewFunction, sizeof(recordfunction));
    int id = tp->ident;
    ptarget = new unsigned[2];//последний будет 0
    if (!ptarget) { outerror(2); return 5; }
    ptarget[0] = findex; ptarget[1] = NULL;
    index = heap->apend(ptarget, sizeof(unsigned int) * 2);
    delete[] ptarget;
    if (!index)
    {
      outerror(44);
      return 5;
    }
    recordclause rc((unsigned)isclause, (unsigned)NULL, 0, findex, index);
    index = heap->apend(&rc, sizeof(recordclause));
    if (!index)
    {
      outerror(44);
      return 5;
    }
  }   break;
  case 437:
  case 937:
  case 837:  //_,list,int
  {   tp = heap->GetPbaserecord(ScVar->goal[maxarity + 1]);
  //(baserecord *)&heap->heaps[ScVar->goal[maxarity + 1]];
      // нужно подсчитать число переменных
  count_var = 0;
  if (VarOnList(tp, ScVar, ClVar, heap) != 0)//подсчет числа переменных в новом предложении
  {
    out(const_cast<char*>("Ошибка при подсчете числа переменных"));
    return 1;
  }
  nvar += count_var;//еще нужно подсчитать число перем в голове
//            out("Переменные подсчитаны");
            //подготовка целей
  unsigned _maxarity = maxarity;
  error = prepare_target(ScVar->goal[maxarity + 1],
    ScVar, ClVar, heap);
  if (error)
  {
    out(const_cast<char*>("Ошибка при поиске целей для ДОБ"));
    return 1;
  }

  ntarget = target_number + 1;
  ptarget = new unsigned[ntarget + 1];
  if (!ptarget)
  {
    outerror(2);
    return 1;
  }

  pl = heap->GetPrecordlist(ScVar->goal[maxarity + 1]);
  //(recordlist *)&heap->heaps[ScVar->goal[maxarity + 1]];
  int i = 0;
  ptarget[i++] = ScVar->goal[maxarity];//head
  while (i < (int)ntarget)
  {
    ptarget[i] = ClVar->bpt[i - 1];
    i++;
  }
  ptarget[i] = (unsigned)NULL;
  i = 0;
  while (ptarget[i])
  {
    unsigned arg[255];
    recordfunction* prf = heap->GetPrecordfunction(ptarget[i]);
    //(recordfunction *)&heap->heaps[ptarget[i]];
    unsigned ptrarg = prf->ptrarg;
    unsigned* prf_arg = heap->GetPunsigned(prf->ptrarg);
    //(unsigned *)&heap->heaps[prf->ptrarg];
    for (int j = 0; j < static_cast<int>(prf->narg); j++)
    {
      //достать константы из агрументов функции.
      unsigned Arg = GetConstTerm(*(prf_arg + j),
        ClVar->frame2, ClVar, heap);
      Arg = 0;
      if (Arg)
      {   /*
          recordvar * prv = (recordvar *)&heap->heaps[Arg];
          if (prv->ident == isvar)
          {
              recordvar newvar(prv->ptrsymb, prv->length,
                                                  prv->num);
              unsigned vindex = heap->apend(&newvar, sizeof(recordvar));
              arg[j] = vindex;
          }
          else
          */
        arg[j] = Arg;
      }
      else
        arg[j] = *(prf_arg + j);
    }
    unsigned ArgIndex = heap->apend(arg, sizeof(unsigned) * prf->narg);
    if (!ArgIndex)
    {
      outerror(44);
      return 5;
    }
    recordfunction NewFunction(prf->narg, prf->func, ArgIndex);
    unsigned findex = heap->apend(&NewFunction, sizeof(recordfunction));
    //if (!unify(ptarget[i], findex, frame2, frame2))
    //    return 1;
    ptarget[i] = findex;
    i++;
  }

  index = heap->apend(ptarget, sizeof(unsigned) * (ntarget + 1));
  if (index)
  {
    recordclause rc(isclause, (unsigned)NULL, 5,//nvar,
      ScVar->goal[maxarity], index);
    int Count = GetVarCountFromClause(&rc, ScVar, ClVar, heap);
    if (Count < 0)
    {
      out(const_cast<char*>("Var count calculation in new clause failure"));
      return 5;
    }
    rc.nvars = Count;
    index = heap->apend(&rc, sizeof(recordclause));
  }
  break;
  }
  default: out(const_cast<char*>("Недопустимый тип аргументов в 'ДОБ'")); return 1;
  }
  if (!index)
  {
    heap->last = old_index;
    outerror(44); return 5;
  }
  //включим в цепочку предложений
  int number = occ(2, ScVar, ClVar, heap);//куда следует воткнуть новое
  //поищем первое предложение
  tp = heap->GetPbaserecord(ScVar->goal[maxarity]);
  //(baserecord *)&heap->heaps[ScVar->goal[maxarity]];
  if (tp->ident == issymbol)
    ps = (recordsconst*)tp;
  else
  {
    if (tp->ident != isfunction)
    {
      outerror(24);
      return 1;
    }
    pf = (recordfunction*)tp;
    recordfunction* _pf = pf;
    ps = heap->GetPrecordsconst(pf->func);
    //(recordsconst *)&heap->heaps[pf->func];
    recordsconst* _ps = ps;

    //теперь поискать сколько переменных в предложении

  }
  if (ps->begin != NULL && ps->begin != isnil)
    pfirstclause = heap->GetPrecordclause(ps->begin);
  //(recordclause *)&heap->heaps[ps->begin];
  else pfirstclause = NULL;//нет таких предложения
//===========
  recordclause* newcl = heap->GetPrecordclause(index);
  //(recordclause *)&heap->heaps[index];
  if (number < 2 || !pfirstclause) //поставим новое предложение первым в списке
  {
    newcl->next = ps->begin;
    ps->begin = index;//стало первым
  }
  else //придется втыкать в средину или конец
  {
    int i = 1; pclause = pfirstclause;
    while (i < number - 1 && pclause->next && pclause->next != isnil)
      if (pclause->next && pclause->next != isnil)
      {
        pclause = heap->GetPrecordclause(pclause->next);
        //(recordclause *)&heap->heaps[pclause->next];
        i++;
      }
    if (i == number - 1)
      newcl->next = pclause->next;
    pclause->next = index;
  }
  return 3;
}

unsigned pradd(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap) //СЛОЖЕНИЕ
{
  long l[4];
  float f[4];
  bool fl = false;//если будет true то обработка вещ иначе целых
  int i;
  for (i = 0; i < 3; i++)
  {
    if (ScVar->goal[i] == 6) fl = true;
  }
  for (i = 0; i < 3; i++)
  {
    switch (ScVar->goal[i])
    {
    case 6:
      f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true)
      {
        f[i] = (float)occ(i, ScVar, ClVar, heap);
        switch (i)
        {
        case 0:
          sw -= 100;
          break;
        case 1:
          sw -= 10;
          break;
        case 2:
          sw -= 1;
        }
      }
      else l[i] = occ(i, ScVar, ClVar, heap);
    }
  }
  switch (sw)
  {
  case 777:
    return (l[0] + l[1] == l[2]) ? 3 : 5; //все целые  ццц
  case 666:
    return (f[0] + f[1] == f[2]) ? 3 : 5; //все веществ ffff
  case 775:
    return zap1(l[0] + l[1], 3, ScVar, ClVar, heap);
  case 665:
    return zap1f(f[0] + f[1], 3, ScVar, ClVar, heap);
  case 577:
    return zap1(l[2] - l[1], 1, ScVar, ClVar, heap);
  case 566:
    return zap1f(f[2] - f[1], 1, ScVar, ClVar, heap);
  case 757:
    return zap1(l[2] - l[0], 2, ScVar, ClVar, heap);
  case 656:
    return zap1f(f[2] / f[0], 2, ScVar, ClVar, heap);
  }
  outerror(31);
  return 1;
}

unsigned prmul3(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)     //умножение   в goal[0].. 1 .. идетифик параметр
{
  long l[3];
  float f[3];
  bool fl = false;//если будет true то обработка вещ иначе целых
  int i;
  for (i = 0; i < 3; i++)
  {
    if (ScVar->goal[i] == 6)
      fl = true;
  }
  for (i = 0; i < 4; i++)
  {
    switch (ScVar->goal[i])
    {
    case 6:
      f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true)
      {
        f[i] = (float)occ(i, ScVar, ClVar, heap);
        switch (i)
        {
        case 0:
          sw -= 100;	break;
        case 1:
          sw -= 10;	break;
        case 2:
          sw -= 1;
        }
      }
      else l[i] = occ(i, ScVar, ClVar, heap);
    }
  }
  switch (sw)
  {
  case 777:return (l[0] * l[1] == l[2]) ? 3 : 5; //все целые  ццц
  case 666:return (f[0] * f[1] == f[2]) ? 3 : 5; //все веществ ffff
  case 775:return zap1(l[0] * l[1], 3, ScVar, ClVar, heap);
  case 665:return zap1f(f[0] * f[1], 3, ScVar, ClVar, heap);
  case 577:
    if (l[1] == 0)
      return 5;
    if (l[2] % l[1] == 0)
      return zap1(l[2] / l[1], 1, ScVar, ClVar, heap);
    f[2] = (float)l[1];
    f[1] = (float)l[1];
  case 566:
    if (f[1] == 0)
      return 5;
    return zap1f(f[2] / f[1], 1, ScVar, ClVar, heap);
  case 757:
    if (l[0] == 0)
      return 5;
    if (l[2] % l[0] == 0)
      return zap1(l[2] / l[0], 2, ScVar, ClVar, heap);
    f[0] = (float)l[0];
    f[2] = (float)l[2];
  case 656:
    if (f[0] == 0)
      return 5;
    return zap1f(f[2] / f[0], 2, ScVar, ClVar, heap);
  }
  outerror(30);
  return 1;//r_t_e не вып пред умножение
}

unsigned prpaint(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap);/* закраска */

unsigned argthree(unsigned name, TScVar* ScVar,
  TClVar* ClVar, array* heap)
{
  unsigned sw;
  conarg(3, ClVar->head, ScVar, ClVar, heap);
  sw = 100 * ScVar->goal[0] + 10 * ScVar->goal[1] + ScVar->goal[2];
  switch (name)
  {
  case hpmul:  return prmul3(sw, ScVar, ClVar, heap);//умножение
  case hpset:  return prset(sw, ScVar, ClVar, heap);//точка
  case hpapp:  return prapp(sw, ScVar, ClVar, heap);//сцеп
  case hpassrt:return prassrt(sw, ScVar, ClVar, heap);//доб
  case hpadd:return pradd(sw, ScVar, ClVar, heap); //сложение
  case hppaint:return prpaint(sw, ScVar, ClVar, heap);//закраска
  case hprand: return prrandom(sw, ScVar, ClVar, heap); // СЛУЧ
  }
  return 0;
}

unsigned prmul(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)     //умножение   в goal[0].. 1 .. идетифик параметр
{
  long l[4];
  float f[4];
  bool fl = false;//если будет true то обработка вещ иначе целых
  int i;
  for (i = 0; i < 4; i++)
  {
    if (ScVar->goal[i] == 6)
      fl = true;
  }
  for (i = 0; i < 4; i++)
  {
    switch (ScVar->goal[i])
    {
    case 6: f[i] = occf(i, ScVar, ClVar, heap); break;
    case 7:
      if (fl == true)
      {
        f[i] = (float)occ(i, ScVar, ClVar, heap);
        switch (i)
        {
        case 0:
          sw -= 1000; break;
        case 1:
          sw -= 100; break;
        case 2:
          sw -= 10; break;
        case 3:
          sw -= 1;
        }
      }
      else l[i] = occ(i, ScVar, ClVar, heap);
    }
  }
  /* switch(sw)     //так было до введения float
   {case 7777:return (occ(0)*occ(1)+occ(2)==occ(3))?3:5; //все целые  цццц
    case 6666:return (occf(0)*occf(1)+occf(2)==occf(3))?3:5; //все веществ ffff
    case 5777:if (occ(1))
              {int work;
               if ((work=occ(3)-occ(2))%occ(1)!=0) return 5;
                return zap1(work/occ(1),1);
              }break;

    case 7577:if (occ(0))                     //цпцц
              {int work;
              if ((work=occ(3)-occ(2)) % occ(0)!=0) return 5;
              return zap1(work/occ(0),2);}break;

    case 7757:return zap1(occ(3)-occ(0)*occ(1),3);    //цпцц
    case 7775:return zap1(occ(0)*occ(1)+occ(2),4);
    case 7557:if (occ(0)) return zap2(occ(3)%occ(0),occ(3)/occ(0),3,2);break;
    case 5757:if (occ(1)) return zap2(occ(3)%occ(1),occ(3)/occ(1),3,1);break;

   }*/

  switch (sw)
  {
  case 7777: return (l[0] * l[1] + l[2] == l[3]) ? 3 : 5; //все целые  цццц
  case 6666: return (f[0] * f[1] + f[2] == f[3]) ? 3 : 5; //все веществ ffff
  case 5777:
    if (l[1])
    {
      long work;
      if ((work = l[3] - l[2]) % l[1] != 0)
        return 5;
      return zap1(work / l[1], 1, ScVar, ClVar, heap);
    }	break;

  case 7577:
    if (l[0])                     //цпцц
    {
      long work;
      if ((work = l[3] - l[2]) % l[0] != 0)
        return 5;
      return zap1(work / l[0], 2, ScVar, ClVar, heap);
    }	break;

  case 7757: return zap1(l[3] - l[0] * l[1], 3, ScVar, ClVar, heap);    //цпцц
  case 6656: return zap1f(f[3] - f[0] * f[1], 3, ScVar, ClVar, heap);    //цпцц
  case 7775: return zap1(l[0] * l[1] + l[2], 4, ScVar, ClVar, heap);
  case 6665: return zap1f(f[0] * f[1] + f[2], 4, ScVar, ClVar, heap);
  case 7557:
    if (l[0])
      return zap2(l[3] % l[0], l[3] / l[0], 3, 2, ScVar, ClVar, heap);
    break;
  case 5757:
    if (l[1])
      return zap2(l[3] % l[1], l[3] / l[1], 3, 1, ScVar, ClVar, heap);
    break;
  case 6767://if (f[1]) return zap2f(f[3]%f[1],f[3]/f[1],3,1);break;
  case 6556://if (f[0]) return zap2(f[3]%f[0],f[3]/f[0],3,2);break;
  case 5666:/*if (f[1])
          {float work;
           if ((work=f[3]-f[2])%f[1]!=0) return 5;
            return zap1f(work/f[1],1);
          }break; */
  case 6566:/*if (f[0])                     //цпцц
          {float work;
          if ((work=f[3]-f[2]) % f[0]!=0) return 5;
          return zap1f(work/f[0],2);}break;*/
    outerror(29);
    return 1;
  }
  outerror(30);
  return 1;//r_t_e не вып пред умножение
}


unsigned prcircl(unsigned sw, TScVar* ScVar, TClVar* ClVar,
  array* heap)
{

  //if (!canvas) return 3;
  int x1, x2, y1, y2, r, color;

  switch (sw)
  {
  case 7777:
  {   x1 = occ(0, ScVar, ClVar, heap);
  y1 = occ(1, ScVar, ClVar, heap);
  r = occ(2, ScVar, ClVar, heap);
  color = occ(3, ScVar, ClVar, heap);
  /*
        if (color > -1 && color < _MaxColors_)
        {   color = colorstable[color];
        }
        canvas->Pen->Color=(TColor)color;
        canvas->Ellipse(x1 - r, y1 - r, x1 + r, y1+r);
  */
  //x2 = y2 = 0;
  //Ellipse(x1, y1, x2, y2, color);
  Ellipse(x1, y1, r, r, color);
  }   break;

  case 7577:
  {   color = occ(3, ScVar, ClVar, heap);
  r = occ(2, ScVar, ClVar, heap);
  x1 = occ(0, ScVar, ClVar, heap) - r;
  y1 = 0;
  x2 = x1 + r + r;
  //y2 = maxgry;
  vertical(x1, y1, x2, color);
  /*
        if (color > -1 && color < _MaxColors_)
        {   color = colorstable[color];
        }
        canvas->Pen->Color = (TColor)color;
        canvas->Brush->Color = canvas->Pen->Color;
        canvas->Rectangle(x1,y1,x2,y2);
  */
  }   break;
  case 5777:
  {   color = occ(3, ScVar, ClVar, heap);
  r = occ(2, ScVar, ClVar, heap);
  x1 = 0;
  y1 = occ(1, ScVar, ClVar, heap) - r;
  //x2 = maxgrx;
  y2 = y1 + r + r;
  /*
        if (color > -1 && color < _MaxColors_)
        {   color = colorstable[color];
        }
        canvas->Pen->Color=(TColor)color;
        canvas->Brush->Color=canvas->Pen->Color;
        canvas->Rectangle(x1,y1,x2,y2);
  */
  horisontal(x1, y1, y2, color);
  }   break;
  case 7757:
  case 7557:
  case 5757:
  case 5557:
  case 5577:
  {   color = occ(3, ScVar, ClVar, heap);
  //x1 = 0; y1 = 0;
  //x2 = maxgrx; y2 = maxgry;
/*
      if (color > -1 && color < _MaxColors_)
      {   color = colorstable[color];
      }
      canvas->Brush->Color=canvas->Pen->Color;
      canvas->Rectangle(x1,y1,x2,y2);
*/
  ClearView(color);
  }   break;
  default:
    outerror(24);
    return 1;//r_t_e
  }
  return 3;
}

unsigned prpaint(unsigned sw, TScVar* ScVar, TClVar* ClVar, array* heap)/* закраска */
{
  int x, y, color;
  if (sw != 777)
  {
    outerror(24);
    return 1;
  }
  x = occ(0, ScVar, ClVar, heap);
  y = occ(1, ScVar, ClVar, heap);
  color = occ(2, ScVar, ClVar, heap);
  FloodFill(x, y, color);
  return 3;
}

unsigned prcopy(unsigned sw, TScVar* ScVar, TClVar* ClVar,
  array* heap)
{
  char str1[maxlinelen], str2[maxlinelen];
  int i1, i2, i3, i4;
  switch (sw)
  {
  case 4775:
  case 9775://str int int var
    occ_line(0, str1, ScVar, ClVar, heap);
    i1 = occ(1, ScVar, ClVar, heap);
    i2 = occ(2, ScVar, ClVar, heap);
    i3 = strlen(str1);
    if (i1 > 0 && i2 > 0 && i3 >= i1 + i2 - 1)
    {
      str1[i1 + i2 - 1] = NULL;
      //char *s=newStr(&str1[i1-1]);
      return zap3(&str1[i1 - 1], 4, ScVar, ClVar, heap);
    }	break;
  case 4774:
  case 4779:
  case 9774:
  case 9779://str int int str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = occ(1, ScVar, ClVar, heap);
    i2 = occ(2, ScVar, ClVar, heap);
    i3 = strlen(str1);
    i4 = strlen(str2);
    return (i1 > 0 && i2 > 0 && i2 == i4 &&
      i3 >= i1 + i4 - 1 &&
      !strncmp(&str1[i1 - 1], str2, i2)) ? 3 : 5;

  case 4574:
  case 4579:
  case 9574:
  case 9579://str var int str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = occ(2, ScVar, ClVar, heap);
    i2 = strlen(str1);
    i3 = strlen(str2);
    if (i1 > 0 && i2 > i1 + i3 - 1 && i3 == i1)
    {
      for (i4 = i2 - i3; i4 >= 0 &&
        strncmp(&str1[i4], str2, i3); i4--);
      if (i4 >= 0)
        return zap1(i4 + 1, 2, ScVar, ClVar, heap);
    }	break;
  case 4554:
  case 4559:
  case 9554:
  case 9559://str var var str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = strlen(str1);
    i2 = strlen(str2);
    if (i1 >= i2)
    {
      int result = 1;
      for (i3 = i1 - i2; i3 >= 0 && result != 0; i3--)
        result = strncmp(&str1[i3], str2, i2);
      if (!result)
        return zap2(i3 + 2, i2, 2, 3, ScVar, ClVar, heap);
    }
    break;
  case 4754:
  case 4759:
  case 9754:
  case 9759://str int var str
    occ_line(0, str1, ScVar, ClVar, heap);
    occ_line(3, str2, ScVar, ClVar, heap);
    i1 = strlen(str1);
    i2 = strlen(str2);
    i3 = occ(1, ScVar, ClVar, heap);
    if ((i3 > 0) && (i1 >= i2 + i3 - 1) &&
      (!strncmp(&str1[i3 - 1], str2, i2)))
      return zap1(i2, 3, ScVar, ClVar, heap);
    break;
  default:
    outerror(24);
    return 1;//r_t_e
  }
  return 5;
}

unsigned prclaus(unsigned sw, TScVar* ScVar,
  TClVar* ClVar, array* heap)
{
  baserecord* tp;
  unsigned err = 0;
  unsigned index = 0;
  //типы аргументов каждого по отдельности
  unsigned sw1;//первый
  unsigned sw2;//второй
  unsigned sw3;//третий
  unsigned sw4 = sw;

  //разбор типов аргументов

  sw1 = sw4 / 1000;
  sw4 = sw4 - sw1 * 1000;
  sw2 = sw4 / 100;
  sw4 = sw4 - sw2 * 100;
  sw3 = sw4 / 10;
  sw4 = sw4 - sw3 * 10;

  //проверка типов
  if (sw1 != 4 && sw1 != 9)
    err = 1;//symb/str
  if (sw2 != 7)
    err = 2;           //int
  if (sw3 != 5 && sw3 != 4 && sw3 != 8)
    err = 3;//var/sybm/func
  if (sw4 != 5 && sw4 != 2 && sw4 != 3)
    err = 4;//var/list/emptylist
  if (err != 0)
  {
    out(const_cast<char*>("Неверный тип аргументов в 'ПРЕДЛ'."));//как нибудь раскрою номер параметра
    return 5;
  }

  //поищем заданное предложение
  unsigned i = occ(1, ScVar, ClVar, heap);//номер заданного предл
  recordsconst* ps = heap->GetPrecordsconst(ScVar->goal[maxarity]);
  //(recordsconst *)&heap->heaps[ScVar->goal[maxarity]];
  if (ps->begin == isnil || ps->begin == NULL)
    return 5;//нет предложения
  recordclause* pc = heap->GetPrecordclause(ps->begin);
  //(recordclause *)&heap->heaps[ps->begin];
  unsigned w;
  for (w = 1; pc->next != isnil && pc->next != NULL && w < i; w++)
  {
    pc = heap->GetPrecordclause(pc->next);
    //(recordclause *)&heap->heaps[pc->next];
  }
  if (w != i)
    return 5;//предл с данным номером отсутствует

  //нашли нужное предложение
  recordfunction* pf = heap->GetPrecordfunction(ClVar->head);
  //(recordfunction*)&heap->heaps[ClVar->head];
//аргументы функции предл
  unsigned* ptr = heap->GetPunsigned(pf->ptrarg);
  //(unsigned *)&heap->heaps[pf->ptrarg];
//цели найденного предложения
  unsigned* p = heap->GetPunsigned(pc->ptrtarget);
  //(unsigned *)&heap->heaps[pc->ptrtarget];

// запомним состояние
  unsigned oldindex = heap->last;
  ClVar->oldtptr = ClVar->tptr;
  ClVar->frame1 = ClVar->oldsvptr = ClVar->svptr;
  ClVar->svptr = ClVar->frame1 + pc->nvars;//это число переменных в предл
  if (ClVar->svptr > ClVar->vmaxstack && expand_stack(ClVar) != 0)
  {
    out(const_cast<char*>("Не достаточно памяти в стеке переменных"));
    return 5;
  }
  //унификация головы предлож с переменной или ...
  if (!unify(p[0], ptr[2], ClVar->frame1, ClVar->frame2, ClVar, heap))
  {
    ClVar->svptr = ClVar->oldsvptr;
    heap->last = oldindex; //??????
    zero(ClVar);
    return 5;
  }

  // получим список из целей найденного предложения
  tp = heap->GetPbaserecord(pc->head);
  //(baserecord *)&heap->heaps[pc->head];
  unsigned tlist = 0;
  switch (tp->ident)
  {
  case isfunction:
  {
    recordfunction* rf = (recordfunction*)tp;
    unsigned i = 1;
    unsigned oldindex = heap->last;
    recordlist pl(rf->func, oldindex + sizeof(recordlist));
    while (p[i] != 0 && p[i] != isnil && !err)
    {
      pl.head = p[i];
      pl.link = heap->last + sizeof(recordlist);
      index = heap->apend(&pl, sizeof(recordlist));
      if (!index)
      {
        outerror(44);
        return 5;
      }

      //           if (!(index=heap->apend(&pl,sizeof(recordlist)))) err=44;
      if (tlist == 0)
        tlist = index;
      i++;
    }
  }
  case issymbol:
  {
    if (!err)
    {
      recordemptylist ptre;
      index = heap->apend(&ptre, sizeof(recordemptylist));
      if (!index)
      {
        outerror(44);
        return 5;
      }

      //				if (!(index=heap->apend(&ptre,sizeof(recordemptylist)))) err=2;
      else
        if (tlist == 0)
          tlist = index;
    }
    if (!err)
    {
      if (!unify(tlist, ptr[3], ClVar->frame1,
        ClVar->frame2, ClVar, heap))
      {
        ClVar->svptr = ClVar->oldsvptr;
        heap->last = oldindex; //?????
        zero(ClVar);
        return 5;
      }
      if (sw4 == 2 || sw4 == 3)
      {
        ClVar->svptr = ClVar->oldsvptr;
        heap->last = oldindex; //?????
        zero(ClVar);
      }

      return 3;//удачная унификация
    }	break;
  }
  }
  return 5;
}

unsigned argfour(unsigned name, TScVar* ScVar, TClVar* ClVar, array* heap)
{
  unsigned sw;
  conarg(4, ClVar->head, ScVar, ClVar, heap);
  sw = ScVar->goal[0] * 1000 + ScVar->goal[1] * 100 +
    ScVar->goal[2] * 10 + ScVar->goal[3];
  switch (name)
  {
  case hpmul:return prmul(sw, ScVar, ClVar, heap);//умножение
  case hpcircl:return prcircl(sw, ScVar, ClVar, heap);//окружность
  case hpcopy:return prcopy(sw, ScVar, ClVar, heap);//копия
  case hpclaus:return prclaus(sw, ScVar, ClVar, heap);//предл
  }
  return 0;
}

unsigned prger(unsigned long sw, TScVar* ScVar, TClVar* ClVar,
  array* heap)
{
  //	if (/*!bitmap ||*/ !canvas) return 3;
  // int xy;
  int x1, x2, y1, y2;
  int color;
  switch (sw)
  {
  case 77777:
  {
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = occ(1, ScVar, ClVar, heap);
    x2 = occ(2, ScVar, ClVar, heap);
    y2 = occ(3, ScVar, ClVar, heap);
    color = occ(4, ScVar, ClVar, heap);
    MoveTo_LineTo(x1, y1, x2, y2, color);
    /*
    SetPenColor(color);
    MoveTo(x1, y1);
    LineTo(x2, y2);
    */
    //!!!gbuf->addobject(new Tgrline(occ(0),occ(1),occ(2),occ(3),occ(4)));
    //setcolor(occ(4));line(occ(0),occ(1),occ(2),occ(3));
  }	break;
  case 77757:
    vertical(occ(0, ScVar, ClVar, heap),
      occ(1, ScVar, ClVar, heap),
      occ(2, ScVar, ClVar, heap),
      occ(4, ScVar, ClVar, heap));
    break;
  case 77577:
    horisontal(occ(0, ScVar, ClVar, heap),
      occ(1, ScVar, ClVar, heap),
      occ(3, ScVar, ClVar, heap),
      occ(4, ScVar, ClVar, heap));
    break;
  case 75777:
    vertical(occ(2, ScVar, ClVar, heap),
      occ(3, ScVar, ClVar, heap),
      occ(0, ScVar, ClVar, heap),
      occ(4, ScVar, ClVar, heap));
    break;
  case 57777:
    horisontal(occ(2, ScVar, ClVar, heap),
      occ(3, ScVar, ClVar, heap),
      occ(1, ScVar, ClVar, heap),
      occ(4, ScVar, ClVar, heap));
    break;
    //=================

  case 55777:
  case 77557:
  case 75557:
  case 57557:
  case 55757:
  case 55577:
  case 55557:
  {
    color = occ(4, ScVar, ClVar, heap);
    x1 = 0; y1 = 0; x2 = maxgrx; y2 = maxgry;
    //SetPenAndBrushColor(color);
    Rectangle(x1, y1, x2, y2, color);
  }	break;

  case 75757:
  {
    color = occ(4, ScVar, ClVar, heap);
    x1 = occ(0, ScVar, ClVar, heap);
    y1 = 0; x2 = occ(2, ScVar, ClVar, heap);
    y2 = maxgry;
    //SetPenAndBrushColor(color);
    Rectangle(x1, y1, x2, y2, color);
  }   break;

  case 57577:
  {
    color = occ(4, ScVar, ClVar, heap);
    x1 = 0;
    y1 = occ(1, ScVar, ClVar, heap);
    x2 = maxgrx;
    y2 = occ(3, ScVar, ClVar, heap);
    //SetPenAndBrushColor(color);
    Rectangle(x1, y1, x2, y2, color);
  }   break;
  case 75577:
  {	vertical(0, occ(3, ScVar, ClVar, heap),
    occ(0, ScVar, ClVar, heap),
    occ(4, ScVar, ClVar, heap));
  vertical(maxgrx, occ(3, ScVar, ClVar, heap),
    occ(0, ScVar, ClVar, heap),
    occ(4, ScVar, ClVar, heap));
  }	break;
  case 57757:
  {	vertical(0, occ(1, ScVar, ClVar, heap),
    occ(2, ScVar, ClVar, heap),
    occ(4, ScVar, ClVar, heap));
  vertical(maxgrx, occ(1, ScVar, ClVar, heap),
    occ(2, ScVar, ClVar, heap),
    occ(4, ScVar, ClVar, heap));
  }	break;
  default:
    outerror(24);
    return 1;//R_T_E
  }
  return 3;
}

unsigned argfive(unsigned name, TScVar* ScVar, TClVar* ClVar,
  array* heap)
{
  unsigned long sw;
  conarg(5, ClVar->head, ScVar, ClVar, heap);
  sw = (unsigned long)(10000l * ScVar->goal[0]);
  sw += 1000 * ScVar->goal[1];
  sw += 100 * ScVar->goal[2];
  sw += 10 * ScVar->goal[3];
  sw += ScVar->goal[4];
  switch (name)
  {
  case hpline:
    return prger(sw, ScVar, ClVar, heap);//линия
  }
  return 1;
}


bool bpred(unsigned name, unsigned narg)
{
  switch (name)
  {
  case hpfail:
  case hptrace:
  case hpnottr:
  case hpcut: return (narg == 0) ? true : false;//0
  case hp_int:
  case hp_float:
  case hpcall:
    //    case hprdint:
  case hpiocod:
  case hpsee:
  case hptell:
  case hpvar:
  case hpint:
  case hpfloat:
  case hpsym:
  case hpwait: return (narg == 1) ? true : false;//1

  case hprdint:
  case hprdsym: return (narg == 1 || narg == 2) ? true : false;

  case hprand: return (narg == 1 || narg == 3) ? true : false;

  case hpgt:
  case hpstint:
  case hpstlst:
  case hplettr:
  case hpdigit:
  case hpterm:
  case hpdel:
  case hpskol: return (narg == 2) ? true : false;//2

  case hpadd:
  case hpset:
  case hpapp:
  case hppaint:
  case hpassrt: return (narg == 3) ? true : false;//3

  case hpmul: return (narg == 3 || narg == 4) ? true : false;
  case hpcircl:
  case hpcopy:
  case hpclaus: return (narg == 4) ? true : false;//4

  case hpline: return (narg == 5) ? true : false;//5
  }
  return false;//нечего сюда лезть
}

//=========== функции ввода ==============

int Inputstring(char* buf, int size, char* caption)
{
  int err = 0;
  char* pCaption = const_cast<char*>("Введите строку");
  if (caption)
    pCaption = caption;
  int _err = InputStringFromDialog(buf, size, pCaption);
  if (!_err)
    out(buf);
  else
    err = 1;//
  return err;
}

int InputSymbol(char* c)
{
  int err = 0;
  char* pCaption = const_cast<char*>("Введите символ");
  char Buf[2];
  err = InputStringFromDialog(Buf, sizeof(Buf), pCaption);
  if (!err)
  {
    Buf[1] = 0;
    *c = Buf[0];
    out(Buf);
    return 0;
  }
  return 1;
}

int InputInt(int* n, const char* caption)
{
  int err = 2;       //!!! нужна проверка на правильность ввода
//  char* pCaption = const_cast<char*>("Введите целое");
//  if (caption)
//    pCaption = caption;
  char Buf[255];
  while (err == 2)
  {
    int _err = InputStringFromDialog(Buf, sizeof(Buf), const_cast<char*>(caption));
    if (!_err)
    {
      if (sscanf(Buf, "%d", n) != 1)
        continue;
      err = 0;
      out(Buf);
    }
    else err = 1;//
  }
  return err;
}




