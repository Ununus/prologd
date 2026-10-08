#define _CRT_SECURE_NO_WARNINGS

#include "prlib/scaner.h"
#include "prlib/pstructs.h"
#include "prlib/control.h"
#include "prlib/functions.h"
#include "prlib/encoding.h"
#include "prlib/keywords.h"

#include "preprocessor.h"

#ifdef PROLOGD_FOR_GUI
#include "host.hpp"
#include "protocol.hpp"
#endif

#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string.h>
#include <functional>
#include <utility>

constexpr char input_signal_char = '$';

std::string import_directory = "";

static const char *source_filename{ nullptr };
static const char *input_filename{ nullptr };
static const char *output_filename{ nullptr };
static const char *error_filename{ nullptr };

static std::unique_ptr<std::stringstream> source_file;
static std::unique_ptr<std::ifstream> input_file;
static std::unique_ptr<std::ofstream> output_file;
static std::unique_ptr<std::ofstream> error_file;

static std::stringstream *source_stream{ nullptr };
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

#ifdef PROLOGD_FOR_GUI
// ---- front-end sinks (see host.hpp) ----
static bool g_haveSinks = false;
static std::function<void(const std::string &)> g_sinkOutLine;
static std::function<void(const std::string &)> g_sinkOutRaw;
static std::function<void(const std::string &, int, const std::string &)> g_sinkError;
static std::function<void(bool)> g_sinkResult;
static std::function<std::string(const std::string &, bool)> g_sinkInput;
#endif

// Set around `scaner` so a diagnostic knows whether it came from the scanner.
static std::string g_errorKind = "runtime";
// The Yes/No of the question being answered, for a front-end that wants it as a
// `result` event.
static bool g_lastFound = false;

void print_cp1251_decoded_to_utf8(const char *str, std::ostream &out) {
  out << prologd::cp1251_to_utf8(str);
}

void decode_utf8_to_cp1251(std::string &str) {
  try {
    str = prologd::utf8_to_cp1251(str);
  } catch (const std::runtime_error &) {
    errout("Decoding error");
    throw 1;
  }
}
void prdout(bool value) {
#ifdef PROLOGD_FOR_GUI
  if (g_haveSinks) {
    g_lastFound = value;
    if (g_sinkResult) {
      g_sinkResult(value);
    }
    return;
  }
#endif
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
#ifdef PROLOGD_FOR_GUI
  if (g_haveSinks) {
    if (g_sinkOutLine) {
      g_sinkOutLine(prologd::cp1251_to_utf8(str) + "\n");
    }
    return;
  }
#endif
  print_cp1251_decoded_to_utf8(str, *output_stream);
  *output_stream << '\n';
  last_srv = true;
}
void usrout(const char *str) {
#ifdef PROLOGD_FOR_GUI
  if (g_haveSinks) {
    if (g_sinkOutRaw) {
      g_sinkOutRaw(prologd::cp1251_to_utf8(str));
    }
    return;
  }
#endif
  print_cp1251_decoded_to_utf8(str, *output_stream);
  last_srv = false;
}
void errout(const char *str) {
#ifdef PROLOGD_FOR_GUI
  if (g_haveSinks) {
    if (g_sinkError) {
      g_sinkError(prologd::cp1251_to_utf8(str), parsed_str_number, g_errorKind);
    }
    return;
  }
#endif
  if (!last_srv) {
    *error_stream << '\n';
  }
  *error_stream << parsed_str_number << ' ';
  print_cp1251_decoded_to_utf8(str, *error_stream);
  *error_stream << std::endl;
  last_srv = true;
}

namespace host {

static std::unique_ptr<array> g_heap;
static std::unique_ptr<TScVar> g_scVar;
static std::unique_ptr<TClVar> g_clVar;
static bool g_enableRunning = true;
static int g_initialError = 0;

#ifdef PROLOGD_FOR_GUI
void install(Sinks sinks) {
  g_haveSinks = true;
  g_sinkOutLine = std::move(sinks.out_line);
  g_sinkOutRaw = std::move(sinks.out_raw);
  g_sinkError = std::move(sinks.error);
  g_sinkResult = std::move(sinks.result);
  g_sinkInput = std::move(sinks.input);
}
#endif

void set_print_questions(bool on) {
  out_questions = on;
}

int start() {
  try {
    g_heap = std::make_unique<array>();
    g_scVar = std::make_unique<TScVar>();
    g_clVar = std::make_unique<TClVar>();
    g_clVar->PrSetting = std::make_unique<TPrSetting>();
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
  parsed_str_number = 0;
  g_initialError = static_cast<int>(buildin(g_scVar.get(), g_heap.get()));
  return 0;
}

int initial_error() {
  return g_initialError;
}

bool running() {
  return g_enableRunning;
}

void reset_line_counter() {
  parsed_str_number = 0;
}

int run_line(const std::string &utf8_line, bool &was_query, bool &found) {
  was_query = false;
  found = false;
  std::string line = utf8_line;
  ++parsed_str_number;
  if (line.empty()) {
    return 0;
  }
  try {
    decode_utf8_to_cp1251(line);
  } catch (...) {
    return static_cast<int>(ErrorCode::InvalidCharacter);
  }
  if (line[0] == input_signal_char) {
    line.erase(line.begin());
    inputed_strs.push_back(line);
    return 0;
  }

  char *cur_str = const_cast<char *>(line.c_str());
  int err = 0;
  g_errorKind = "parse";
  try {
    err = static_cast<int>(scaner(cur_str, g_scVar.get(), g_heap.get()));
    if (err == 0 && g_scVar->Query && g_scVar->EndOfClause) {
      g_errorKind = "runtime";
      was_query = true;
      g_lastFound = true;
      if (out_questions) {
#ifdef PROLOGD_FOR_GUI
        if (g_haveSinks) {
          pldout(cur_str);
        } else
#endif
        {
          if (!last_srv) {
            *output_stream << '\n';
          }
          print_cp1251_decoded_to_utf8(cur_str, *output_stream);
          *output_stream << '\n';
          last_srv = true;
        }
      }
      err = static_cast<int>(control(g_scVar.get(), g_clVar.get(), g_heap.get(), &g_enableRunning));
      g_scVar->Query = g_scVar->EndOfClause = false;
      found = g_lastFound && err == 0;
    }
  } catch (const std::bad_alloc &er) {
    g_errorKind = "runtime";
    errout(er.what());
  } catch (const std::runtime_error &er) {
    g_errorKind = "runtime";
    errout(er.what());
  } catch (...) {
    g_errorKind = "runtime";
    errout("Prolog failure");
  }
  return err;
}

void report_error(int code) {
  outerror(static_cast<ErrorCode>(code));
}

}  // namespace host

int runProlog() {
  const int init = host::start();
  if (init != 0) {
    return init;
  }
  int err = host::initial_error();
  while (err == 0 && host::running()) {
    std::string line;
    if (!std::getline(*source_stream, line)) {
      break;
    }
    bool was_query = false;
    bool found = false;
    err = host::run_line(line, was_query, found);
  }
  if (err != 0) {
    host::report_error(err);
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
      bool haveInput = false;
#ifdef PROLOGD_FOR_GUI
      if (g_haveSinks && g_sinkInput) {
        line = g_sinkInput(prologd::cp1251_to_utf8(caption), splitSpace);
        haveInput = !line.empty();
      } else
#endif
      {
        haveInput = static_cast<bool>(std::getline(*input_stream, line));
      }
      if (!haveInput) {
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

// Список занятых имён движка (keywords.h) — то, что интерфейс читает командой
// `--list-keywords`.  Строки идут в UTF-8: консоль переводит их так же, как и
// всё остальное, что печатает движок.
void printKeywords() {
  for (const prologd::ReservedName &name : prologd::reserved_names()) {
    for (const std::string &label : name.labels()) {
      std::cout << prologd::cp1251_to_utf8(label) << '\n';
    }
  }
}

void printHelp() {
  std::cout << "-s, --source=FILE  \tinterpret a file.\n";
  std::cout << "-d, --directory=DIR\tuse an import directory\n";
  std::cout << "-i, --input=FILE   \tuse a file to input.\n";
  std::cout << "-o, --output=FILE  \tuse a file to print the output.\n";
  std::cout << "-e, --error=FILE   \tuse a file to print an error.\n";
  std::cout << "-q, --questions    \tprint questions.\n";
  std::cout << "-h, --help         \tshow this help and exit.\n";
  std::cout << "--list-keywords    \tprint the reserved names of the engine and exit.\n";
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
  bool json_protocol = false;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printHelp();
      return 0;
    }
    if (strcmp(argv[i], "--list-keywords") == 0) {
      printKeywords();
      return 0;
    }
    if (strcmp(argv[i], "--protocol") == 0) {
      if (i + 1 >= argc) {
        std::cerr << "Missing value for --protocol" << std::endl;
        return 1;
      }
      const char *value = argv[++i];
      if (strcmp(value, "json") == 0) {
        json_protocol = true;
      } else if (strcmp(value, "text") != 0) {
        std::cerr << "Unknown protocol " << value << std::endl;
        return 1;
      }
      continue;
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
    if (strncmp(argv[i], "--protocol", sizeof("--protocol") - 1) == 0) {
      const char *value = eqptr + 1;
      if (strcmp(value, "json") == 0) {
        json_protocol = true;
      } else if (strcmp(value, "text") != 0) {
        std::cerr << "Unknown protocol " << value << std::endl;
        return 1;
      }
      continue;
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

  if (json_protocol) {
#ifndef PROLOGD_FOR_GUI
    std::cerr << "error: this build has no JSON protocol; configure with "
                 "-DPROLOGD_BUILD_FOR_GUI=ON"
              << std::endl;
    return 1;
#else
    if (input_filename || output_filename || error_filename) {
      std::cerr << "error: --protocol json owns stdin and stdout; drop -i/-o/-e" << std::endl;
      return 1;
    }
    output_stream = &std::cout;
    error_stream = &std::cerr;
    return prologd_cli::run_json_session(std::cin, std::cout);
#endif
  }

  try {
    std::stringstream& src = ( 
                                source_filename ? 
                                preprocessor_run(source_filename) : 
                                preprocessor_run_interactive(&std::cin)
                             );
    //source_file = std::make_unique<std::stringstream>(src);
    //source_stream = source_file.get();
    source_stream = &src;
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    return 1;
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
