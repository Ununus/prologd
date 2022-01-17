#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#include "prlib/scaner.h"
#include "prlib/pstructs.h"
#include "prlib/control.h"
#include "prlib/functions.h"

#include <memory>
#include <string>
#include <charconv>
#include <fstream>

//#pragma comment(lib, "legacy_stdio_definitions.lib")
//#pragma comment(lib, "libr/prlib.lib")

std::unique_ptr<std::ifstream> program_file;
std::unique_ptr<std::ifstream> input_file;
std::unique_ptr<std::ofstream> out_file;
std::unique_ptr<std::ofstream> err_file;
int Nstr;

//void RunProlog(int string_count, char** text, bool out_question = true, bool trace = false)
void RunProlog(bool out_question = true)
{
  int serr = 0, cerr = 0;
  std::unique_ptr<array> heap;
  std::unique_ptr<TScVar> ScVar;
  std::unique_ptr<TClVar> ClVar;
  try
  { // инициализаци€ переменных, необходимых дл€ работы
    heap = std::make_unique<array>(_maxarray_);
    ScVar = std::make_unique<TScVar>();
    ClVar = std::make_unique<TClVar>();
    ClVar->PrSetting = new TPrSetting;
    ClVar->PrSetting->OutQue = out_question;
    ClVar->PrSetting->Tracce = false;
    ClVar->PrSetting->fin = ClVar->PrSetting->fout = 0;
    ClVar->PrSetting->NameInFile = ClVar->PrSetting->NameOutFile = 0;

    if (!heap->size || !ScVar->InitOk || !ClVar->InitOk) throw 1;
    ClVar->ch = 15;
    ClVar->fn = 0;
  }
  catch (...) {
    out(const_cast<char*>("Prolog initialization failure"));
    if (ClVar && ClVar->PrSetting) delete ClVar->PrSetting;
    return;
  }
  bool EnableRunning = true;
  serr = buildin(ScVar.get(), heap.get());
  for (Nstr = 0; !serr && !cerr && EnableRunning; Nstr++) {
    // трансл€ци€ построчно
    std::string line;
    if (!std::getline(*program_file, line)) {
      break;
    }
    char* p = const_cast<char*>(line.c_str()); // текуща€ строка
    try
    {
      serr = scaner(p, ScVar.get(), heap.get());

      if (!serr && ScVar->Query && ScVar->EndOfClause) //если конец предложени€ и вопрос то
      {
        if (out_question) //вывод вопроса
        {
          out(const_cast<char*>(""));
          out(p);
        }
        cerr = control(ScVar.get(), ClVar.get(), heap.get(), &EnableRunning);
        ScVar->Query = ScVar->EndOfClause = false;     //на выполнение
      }
    }
    catch (...) {
      errout(const_cast<char*>("ќшибка исполнени€"));
      if (ClVar && ClVar->PrSetting) delete ClVar->PrSetting;
      return;
    }
  }
}

void out(char* str) {
  //puts(str);
  *out_file << str << '\n';
}
void errout(char* str) {
  char number[8]{};
  std::to_chars(number, number + 8, Nstr);
  *err_file << number << " - " << str << '\n';
}
int InputStringFromDialog(char* buf, size_t size, char *caption) {
  //out(caption);
  std::string line;
  if (!std::getline(*input_file, line)) {
    errout(const_cast<char*>("ќжидаемый ввод не обнаружен"));
    return 1;
  }
  if (size <= 0) return 1;
  int to = size - 1;
  if (static_cast<int>(line.size()) < to) to = static_cast<int>(line.size());
  for (int i = 0; i < to; ++i) {
    buf[i] = line[i];
  }
  buf[to] = 0;
  return 0;
}
unsigned int GetPixel(int, int) {
  return 0;
}
void ClearView(unsigned int) {
}
void Ellipse(int, int, int, int, unsigned int) {
}
void FloodFill(int, int, unsigned int) {
}
void HideGWindow(void) {
}
void horisontal(int, int, int, int) {
}
void MoveTo_LineTo(int, int, int, int, unsigned int) {
}
void Rectangle(int, int, int, int, unsigned int) {
}
void SetPixel(int, int, unsigned int) {
}
void ShowGWindow(bool) {
}
void vertical(int, int, int, int) {
}


int main(int argv, char** argc)
{
  if (argv < 6) {
    puts("Too few arguments");
    puts("argv[1] - 't' or '1' - out_");
    puts("argv[2] - path to the program file");
    puts("argv[3] - path to the input file");
    puts("argv[4] - path to the output file");
    puts("argv[5] - path to the error file");
    return 0;
  }
  program_file = std::make_unique<std::ifstream>(argc[2]);
  if (!program_file->is_open()) {
    printf("Can't open %s\n", argc[2]);
    return 0;
  }
  input_file = std::make_unique<std::ifstream>(argc[3]);
  if (!input_file->is_open()) {
    printf("Can't open %s\n", argc[3]);
    return 0;
  }
  out_file = std::make_unique<std::ofstream>(argc[4]);
  if (!out_file->is_open()) {
    printf("Can't open %s\n", argc[4]);
    return 0;
  }
  err_file = std::make_unique<std::ofstream>(argc[5]);
  if (!err_file->is_open()) {
    printf("Can't open %s\n", argc[5]);
    return 0;
  }
  bool out_question = (argc[1][0] == 't' || argc[1][0] == '1');
  RunProlog(out_question);
}
