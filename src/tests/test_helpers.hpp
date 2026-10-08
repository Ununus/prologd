#pragma once

#include <string>
#include <vector>

// The engine as a test sees it: a program in, what it printed and what it
// answered out.  The interpreter has no console of its own — it prints through
// `pldout`/`usrout`/`errout` and asks through `InputStringFromDialog` — so these
// tests provide those hooks (`test_host.cpp`) and drive it the way prologdc.cpp
// does: one statement per line, `$` lines feeding the program's input.
namespace prologd_test {

//: What one run produced, in UTF-8.
struct Run {
  //: Everything the program printed, answers («ДА», «НЕТ», `имя=значение`) and
  //: the program's own `ВЫВОД` alike, in the order they were written.
  std::string output;
  //: What the interpreter sent to its error stream (a diagnostic, and the text of
  //: the error code the run ended with).
  std::string errors;
  //: The error code the run ended with; `0` is `ErrorCode::NoErrors`.
  int error_code = 0;
  //: The source line the run was on when it ended (1-based).
  int line = 0;
  //: The answer of the last question that ran.
  bool found = true;

  //: The run was clean: no error code, no diagnostic.
  bool ok() const { return error_code == 0 && errors.empty(); }
};

// Runs one program in the dialect this engine reads: a variable is one letter
// (an apostrophe may follow), a bare word is a constant, a `?`-line is a question
// and a `$`-line is a line of the program's input.  `input` answers the questions
// the program asks when its `$`-lines run out; `questions` false only *defines*
// the program, without running its `?`-lines.
Run run(const std::string &program,
        const std::vector<std::string> &input = {},
        bool questions = true);

//: Runs one question of its own and answers what it printed.
//: `question("СЛОЖЕНИЕ(1,2,х)")` is `?СЛОЖЕНИЕ(1,2,х).` and answers `х=3\n`.
std::string question(const std::string &goal);

//: The graphics primitives the last run called, one line each (`setPixel(1,1,3)`).
//: The engine of this console build draws nothing, so the hooks are the only
//: place a drawing predicate can be seen at all.
const std::vector<std::string> &graphics_calls();

}  // namespace prologd_test
