// The console of a test: the hooks the engine prints and reads through, and the
// little driver that feeds it a program.
//
// The engine of this build is the old Пролог-Д, so the text that crosses this
// boundary is CP1251 (the dialect's alphabet); a test writes UTF-8 — it is a
// source file compiled as such — and the conversions are prlib's own
// (`prlib/encoding.h`), the same ones the console and the JSON session use.

#include "test_helpers.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "prlib/control.h"
#include "prlib/encoding.h"
#include "prlib/functions.h"
#include "prlib/pstructs.h"
#include "prlib/scaner.h"

namespace prologd_test {
namespace {

//: A line of the program that carries input, the way the console reads it.
constexpr char kInputSignal = '$';

//: What the hooks below collect (and what `run` clears before a program).
std::string g_output;
std::string g_errors;
std::vector<std::string> g_queued;
std::vector<std::string> g_answers;
std::size_t g_answer = 0;
std::stringstream g_rest;
int g_line = 0;
bool g_found = true;
std::vector<std::string> g_graphics;

//: Whether the last thing written ended a line — the console keeps the same flag
//: and answers on a line of its own after a program printed something.
bool g_at_line_start = true;

//: The engine writes CP1251 - that is the alphabet it reads - and a test is
//: written in UTF-8.  Everything this harness says to the engine is converted
//: here; what the engine says back is converted in the hooks below.
std::string cp1251(const std::string &utf8) {
  return prologd::utf8_to_cp1251(utf8);
}

//: The next line the program (`$` lines) or the caller supplies.  False when
//: there is none left, which is not an error of the harness: the console tells
//: the engine and lets the goal fail, and so does this.
bool nextInputLine(std::string &line) {
  if (!g_queued.empty()) {
    line = g_queued.front();
    g_queued.erase(g_queued.begin());
    return true;
  }
  if (g_answer < g_answers.size()) {
    line = prologd::utf8_to_cp1251(g_answers[g_answer++]);
    return true;
  }
  return false;
}

std::string number(long long value) {
  const std::string text = std::to_string(value);
  return text;
}

}  // namespace
}  // namespace prologd_test

// ---- the hooks the engine calls -------------------------------------------
// These mirror `prologdc.cpp` so that a test reads what the console prints: a
// line the engine writes keeps its newline, an answer starts on a line of its
// own, and a diagnostic carries the number of the line it was found on.
void pldout(const char *str) {
  prologd_test::g_output += prologd::cp1251_to_utf8(str);
  prologd_test::g_output += '\n';
  prologd_test::g_at_line_start = true;
}

void usrout(const char *str) {
  prologd_test::g_output += prologd::cp1251_to_utf8(str);
  prologd_test::g_at_line_start = false;
}

void errout(const char *str) {
  if (!prologd_test::g_at_line_start) {
    prologd_test::g_errors += '\n';
  }
  prologd_test::g_errors += std::to_string(prologd_test::g_line);
  prologd_test::g_errors += ' ';
  prologd_test::g_errors += prologd::cp1251_to_utf8(str);
  prologd_test::g_errors += '\n';
  prologd_test::g_at_line_start = true;
}

void prdout(bool value) {
  if (!prologd_test::g_at_line_start) {
    prologd_test::g_output += '\n';
  }
  prologd_test::g_output += value ? "ДА\n" : "НЕТ\n";
  prologd_test::g_at_line_start = true;
  prologd_test::g_found = value;
}

std::string InputStringFromDialog(const char *caption, bool splitSpace) {
  (void)caption;  // a test does not show the question
  std::string answer;
  const bool have = splitSpace ? static_cast<bool>(prologd_test::g_rest >> answer)
                               : static_cast<bool>(std::getline(prologd_test::g_rest, answer));
  if (!have) {
    std::string queued;
    if (prologd_test::nextInputLine(queued)) {
      prologd_test::g_rest = std::stringstream(queued);
      return InputStringFromDialog(caption, splitSpace);
    }
    errout(prologd_test::cp1251("Недостаточно входных данных").c_str());
    return {};
  }
  return answer;
}

// The picture of this build: the console draws nothing, and neither does a test.
// Every call is written down instead, which is the only way a drawing predicate
// can be seen at all.
void Rectangle(long long x1, long long y1, long long x2, long long y2, unsigned color) {
  prologd_test::g_graphics.push_back("rectangle(" + prologd_test::number(x1) + "," +
                                     prologd_test::number(y1) + "," +
                                     prologd_test::number(x2) + "," +
                                     prologd_test::number(y2) + "," +
                                     prologd_test::number(color) + ")");
}

void MoveTo_LineTo(long long x1, long long y1, long long x2, long long y2, unsigned color) {
  prologd_test::g_graphics.push_back("lineTo(" + prologd_test::number(x1) + "," +
                                     prologd_test::number(y1) + "," +
                                     prologd_test::number(x2) + "," +
                                     prologd_test::number(y2) + "," +
                                     prologd_test::number(color) + ")");
}

void FloodFill(long long x, long long y, unsigned color) {
  prologd_test::g_graphics.push_back("floodFill(" + prologd_test::number(x) + "," +
                                     prologd_test::number(y) + "," +
                                     prologd_test::number(color) + ")");
}

void vertical(long long x1, long long y, long long x2, long long color) {
  prologd_test::g_graphics.push_back("vertical(" + prologd_test::number(x1) + "," +
                                     prologd_test::number(y) + "," +
                                     prologd_test::number(x2) + "," +
                                     prologd_test::number(color) + ")");
}

void horisontal(long long x, long long y1, long long y2, long long color) {
  prologd_test::g_graphics.push_back("horizontal(" + prologd_test::number(x) + "," +
                                     prologd_test::number(y1) + "," +
                                     prologd_test::number(y2) + "," +
                                     prologd_test::number(color) + ")");
}

void ClearView(unsigned color) {
  prologd_test::g_graphics.push_back("clearView(" + prologd_test::number(color) + ")");
}

void Ellipse(long long x1, long long y1, long long x2, long long y2, unsigned color) {
  prologd_test::g_graphics.push_back("ellipse(" + prologd_test::number(x1) + "," +
                                     prologd_test::number(y1) + "," +
                                     prologd_test::number(x2) + "," +
                                     prologd_test::number(y2) + "," +
                                     prologd_test::number(color) + ")");
}

void SetPixel(long long x, long long y, unsigned color) {
  prologd_test::g_graphics.push_back("setPixel(" + prologd_test::number(x) + "," +
                                     prologd_test::number(y) + "," +
                                     prologd_test::number(color) + ")");
}

unsigned GetPixel(long long x, long long y) {
  (void)x;
  (void)y;
  return 0;  // an empty screen is black, as the console's stub says
}

// ---- the driver ------------------------------------------------------------
namespace prologd_test {

Run run(const std::string &program, const std::vector<std::string> &input, bool questions) {
  g_output.clear();
  g_errors.clear();
  g_queued.clear();
  g_answers = input;
  g_answer = 0;
  g_rest = std::stringstream();
  g_line = 0;
  g_found = true;
  g_graphics.clear();
  g_at_line_start = true;

  auto heap = std::make_unique<array>();
  auto scVar = std::make_unique<TScVar>();
  auto clVar = std::make_unique<TClVar>();
  clVar->PrSetting = std::make_unique<TPrSetting>();

  ErrorCode error = buildin(scVar.get(), heap.get());
  std::istringstream source(program);
  std::string line;
  while (error == ErrorCode::NoErrors && std::getline(source, line)) {
    ++g_line;
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (line.empty()) {
      continue;
    }
    std::string text;
    try {
      text = prologd::utf8_to_cp1251(line);
    } catch (const std::runtime_error &) {
      error = ErrorCode::InvalidCharacter;
      break;
    }
    if (text[0] == kInputSignal) {
      g_queued.push_back(text.substr(1));
      continue;
    }
    if (!questions && text[0] == '?') {
      continue;
    }
    char *statement = const_cast<char *>(text.c_str());
    try {
      error = scaner(statement, scVar.get(), heap.get());
      if (error == ErrorCode::NoErrors && scVar->Query && scVar->EndOfClause) {
        bool running = true;
        g_found = true;  // as run_line does before a question
        error = control(scVar.get(), clVar.get(), heap.get(), &running);
        scVar->Query = scVar->EndOfClause = false;
      }
    } catch (const std::bad_alloc &) {
      error = ErrorCode::NotEnoughMemoryToRunTheProgram;
    } catch (const std::exception &) {
      error = ErrorCode::AnExceptionOccurredDuringExecution;
    } catch (...) {
      error = ErrorCode::AnExceptionOccurredDuringExecution;
    }
  }

  Run result;
  result.output = g_output;
  result.errors = g_errors;
  result.line = g_line;
  result.found = g_found;
  result.error_code = static_cast<int>(error);
  if (error != ErrorCode::NoErrors) {
    // The console's own last word, and the text a test wants to see when a run
    // does not end cleanly.
    outerror(error);
    result.errors = g_errors;
  }
  return result;
}

std::string question(const std::string &goal) {
  return run("?" + goal + ".\n").output;
}

const std::vector<std::string> &graphics_calls() {
  return g_graphics;
}

}  // namespace prologd_test
