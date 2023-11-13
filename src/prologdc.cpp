#define _CRT_SECURE_NO_WARNINGS

#include "prlib/scaner.h"
#include "prlib/pstructs.h"
#include "prlib/control.h"
#include "prlib/functions.h"

#include "preprocessor.h"

#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string.h>

constexpr char input_signal_char = '$';

std::string import_directory = "";

static const char *source_filename{ nullptr };
static const char *input_filename{ nullptr };
static const char *output_filename{ nullptr };
static const char *error_filename{ nullptr };

static std::unique_ptr<std::ifstream> source_file;
static std::unique_ptr<std::ifstream> input_file;
static std::unique_ptr<std::ofstream> output_file;
static std::unique_ptr<std::ofstream> error_file;

static std::istream *source_stream{ nullptr };
static std::istream *input_stream{ nullptr };
// TODO: если открываем один и тот же файл для stdout и stderr, то не работает
// Нужно сделать если output_filename == output_filename, то указатели на один и тот же поток
static std::ostream *output_stream{ nullptr };
static std::ostream *error_stream{ nullptr };

static bool out_questions{ false };
static int parsed_str_number{ 0 };

static std::list<std::string> inputed_strs;
static std::stringstream cur_str_stream;

// Для контроля перевода строк
// Работает только с одним потоком вывода
static bool last_srv = true;

void print_cp1251_decoded_to_utf8(const char *str, std::ostream &out) {
  for (const char *c = str; *c; ++c) {
    if (*c == 'ё') {
      out << static_cast<unsigned char>(-47);
      out << static_cast<unsigned char>(-111);
    } else if (*c == 'Ё') {
      out << static_cast<unsigned char>(-48);
      out << static_cast<unsigned char>(-127);
    } else if (*c >= 'А' && *c <= 'я') {
      auto v = static_cast<unsigned char>(*c - 'А') + 1040u;
      unsigned char c1 = 192u + (v >> 6u);
      unsigned char c2 = (v & 63u) + 128u;
      out << c1 << c2;
    } else {
      out << static_cast<unsigned char>(*c & 127u);
    }
  }
  // out << '\n';
}

void decode_utf8_to_cp1251(std::string &str) {
  if (str.empty())
    return;
  size_t si = 0;
  for (size_t i = 0; i < str.size();) {
    if (i + 1 < str.size() && str[i] == -48 && str[i + 1] == -127) {
      str[si++] = 'Ё';
      i += 2;
    } else if (i + 1 < str.size() && str[i] == -47 && str[i + 1] == -111) {
      str[si++] = 'ё';
      i += 2;
    } else if ((str[i] & 128u) == 0) {
      str[si++] = str[i++];
    } else if ((str[i] & 224u) == 192u && i + 1 != str.size()) {
      str[si++] = ((str[i] ^ 192u) << 6u) + (str[i + 1] & 63u) - 1040u - 64u;
      i += 2;
    } else {
      errout("Decoding error");
      throw 1;
    }
  }
  str.resize(si);
}
void prdout(bool value) {
  if (!last_srv) {
    *output_stream << '\n';
  }
  if (value) {
    print_cp1251_decoded_to_utf8("ДА", *output_stream);
  } else {
    print_cp1251_decoded_to_utf8("НЕТ", *output_stream);
  }
  *output_stream << '\n';
  last_srv = true;
}
void pldout(const char *str) {
  print_cp1251_decoded_to_utf8(str, *output_stream);
  *output_stream << '\n';
  last_srv = true;
}
void usrout(const char *str) {
  print_cp1251_decoded_to_utf8(str, *output_stream);
  last_srv = false;
}
void errout(const char *str) {
  if (!last_srv) {
    *error_stream << '\n';
  }
  *error_stream << parsed_str_number << ' ';
  print_cp1251_decoded_to_utf8(str, *error_stream);
  *error_stream << std::endl;
  last_srv = true;
}

int runProlog() {
  ErrorCode err = ErrorCode::NoErrors;
  std::unique_ptr<array> heap;
  std::unique_ptr<TScVar> ScVar;
  std::unique_ptr<TClVar> ClVar;
  try {  // инициализация переменных, необходимых для работы интерпретатора
    heap = std::make_unique<array>();
    ScVar = std::make_unique<TScVar>();
    ClVar = std::make_unique<TClVar>();
    ClVar->PrSetting = std::make_unique<TPrSetting>();
  } catch (const std::bad_alloc &er) {
    errout(er.what());
    return 1;
  } catch (const std::runtime_error &er) {
    errout(er.what());
    return 2;
  } catch (...) {
    errout("Prolog initialization failure");
    return 3;
  }
  bool EnableRunning = true;
  err = buildin(ScVar.get(), heap.get());
  parsed_str_number = 0;
  while (err == ErrorCode::NoErrors && EnableRunning) {  // трансляция построчно
    std::string line;
    if (!std::getline(*source_stream, line)) {
      break;
    }
    ++parsed_str_number;
    if (line.empty()) {
      continue;
    }
    decode_utf8_to_cp1251(line);
    if (line[0] == input_signal_char) {
      line.erase(line.begin());
      inputed_strs.push_back(line);
      continue;
    }

    char *cur_str = const_cast<char *>(line.c_str());
    try {
      err = scaner(cur_str, ScVar.get(), heap.get());
      if (err == ErrorCode::NoErrors && ScVar->Query && ScVar->EndOfClause)  // если конец предложения и вопрос то
      {
        if (out_questions) {
          if (!last_srv) {
            *output_stream << '\n';
          }
          print_cp1251_decoded_to_utf8(cur_str, *output_stream);
          *output_stream << '\n';
          last_srv = true;
        }
        err = control(ScVar.get(), ClVar.get(), heap.get(), &EnableRunning);
        ScVar->Query = ScVar->EndOfClause = false;  // на выполнение
      }
    } catch (const std::bad_alloc &er) {
      errout(er.what());
    } catch (const std::runtime_error &er) {
      errout(er.what());
    } catch (...) {
      errout("Prolog failure");
    }
  }
  if (err != ErrorCode::NoErrors) {
    outerror(err);
  }
  return 0;
}

std::string InputStringFromDialog(const char *caption, bool splitSpace) {
  std::string line;
  bool sss = false;
  if (splitSpace) {
    if (cur_str_stream >> line) {
      sss = true;
    }
  } else {
    if (std::getline(cur_str_stream, line)) {
      sss = true;
    }
  }
  if (!sss) {
    if (!inputed_strs.empty()) {
      cur_str_stream = std::stringstream(inputed_strs.front());
      inputed_strs.pop_front();
    } else {
      if (!std::getline(*input_stream, line)) {
        errout("Недостаточно входных данных");
        return {};
      }
      decode_utf8_to_cp1251(line);
      cur_str_stream = std::stringstream(line);
    }
    return InputStringFromDialog(caption, splitSpace);
  }
  if (out_questions) {
    pldout(caption);
  }
  return line;
}

void Rectangle(long long, long long, long long, long long, unsigned) {}
void MoveTo_LineTo(long long, long long, long long, long long, unsigned) {}
void FloodFill(long long, long long, unsigned) {}
void vertical(long long, long long, long long, long long) {}
void horisontal(long long, long long, long long, long long) {}
void ClearView(unsigned) {}
void Ellipse(long long, long long, long long, long long, unsigned) {}
void SetPixel(long long, long long, unsigned) {}
unsigned GetPixel(long long, long long) {
  return 0;
}

void printHelp() {
  std::cout << "-s, --source=FILE  \tinterpret a file.\n";
  std::cout << "-d, --directory=DIR\tuse an import directory\n";
  std::cout << "-i, --input=FILE   \tuse a file to input.\n";
  std::cout << "-o, --output=FILE  \tuse a file to print the output.\n";
  std::cout << "-e, --error=FILE   \tuse a file to print an error.\n";
  std::cout << "-q, --questions    \tprint questions.\n";
  std::cout << "-h, --help         \tshow this help and exit.\n";
  std::cout << "\n";
  std::cout << "If files are not specified, then standard streams will be used.\n";
  std::cout << "\n";
  std::cout << "If in the source there is a string starting with the $ symbol,\n"
               "  then it will be used for input.\n";
  std::cout << "If there are not enough such string for input, the program\n"
               "  will read strings from the input stream.\n";
  std::cout << '\n';
  std::cout << "If the qustions option is set, then the program prints\n"
               "  questions that were in the source code.\n";
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printHelp();
      return 0;
    }
    if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--qustions") == 0) {
      out_questions = true;
      continue;
    }
    char *eqptr = strchr(argv[i], '=');
    if (!eqptr || !*(eqptr + 1)) {
      std::cerr << "Invalid argument " << argv[i] << std::endl;
      return 1;
    }
    if (strncmp(argv[i], "-s", sizeof("-s") - 1) == 0 || strncmp(argv[i], "--source", sizeof("--source") - 1) == 0) {
      source_filename = eqptr + 1;
      continue;
    }
    if (strncmp(argv[i], "-i", sizeof("-i") - 1) == 0 || strncmp(argv[i], "--input", sizeof("--input") - 1) == 0) {
      input_filename = eqptr + 1;
      continue;
    }
    if (strncmp(argv[i], "-o", sizeof("-o") - 1) == 0 || strncmp(argv[i], "--output", sizeof("--output") - 1) == 0) {
      output_filename = eqptr + 1;
      continue;
    }
    if (strncmp(argv[i], "-e", sizeof("-e") - 1) == 0 || strncmp(argv[i], "--error", sizeof("--error") - 1) == 0) {
      error_filename = eqptr + 1;
      continue;
    }
	if (strncmp(argv[i], "-d", sizeof("-d") - 1) == 0 || strncmp(argv[i], "--directory", sizeof("--directory") - 1) == 0)
	{
      import_directory = eqptr + 1;
      if (import_directory.back() != '/')
        import_directory += '/';
      continue;
	}
    std::cerr << "Invalid argument " << argv[i] << std::endl;
    return 1;
  }
  if (source_filename) {
    std::string src;
    try {
      src = preprocessor_run(source_filename);
	} catch (const std::runtime_error &err) {
      std::cerr << err.what() << std::endl;
      return 1;
	}
    source_file = std::make_unique<std::ifstream>(src.c_str());
    if (!source_file->is_open()) {
      std::cerr << "Cannot open file " << source_filename << std::endl;
      return 1;
    }
    source_stream = source_file.get();
  } else {
    source_stream = &std::cin;
  }
  if (input_filename) {
    input_file = std::make_unique<std::ifstream>(input_filename);
    if (!input_file->is_open()) {
      std::cerr << "Cannot open file " << input_filename << std::endl;
      return 1;
    }
    input_stream = input_file.get();
  } else {
    input_stream = &std::cin;
  }
  if (output_filename) {
    output_file = std::make_unique<std::ofstream>(output_filename);
    if (!output_file->is_open()) {
      std::cerr << "Cannot open file " << output_filename << std::endl;
      return 1;
    }
    output_stream = output_file.get();
  } else {
    output_stream = &std::cout;
  }
  if (error_filename) {
    error_file = std::make_unique<std::ofstream>(error_filename);
    if (!error_file->is_open()) {
      std::cerr << "Cannot open file " << error_filename << std::endl;
      return 1;
    }
    error_stream = error_file.get();
  } else {
    error_stream = &std::cerr;
  }

  return runProlog();
}
