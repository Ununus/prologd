#pragma once

#include <functional>
#include <string>

// The bridge between the interpreter (prlib, driven by prologdc.cpp) and a
// front-end.  The console writes to its streams; the JSON session (protocol.cpp)
// installs its own sinks and turns the same calls into events.
//
// The interpreter itself speaks CP1251; every value that crosses this boundary is
// UTF-8, so a front-end never sees the engine's internal encoding.
namespace host {

// Where the interpreter sends its text and where it asks for input.
struct Sinks {
  // `pldout`: one finished line, the trailing '\n' included.
  std::function<void(const std::string &utf8)> out_line;
  // `usrout`: text without a '\n'.
  std::function<void(const std::string &utf8)> out_raw;
  // `errout`: a diagnostic, the source line it belongs to (0 when unknown), and
  // its kind ("parse" while the scanner is reading, "runtime" otherwise).
  std::function<void(const std::string &utf8, int line, const std::string &kind)> error;
  // `prdout`: Yes/No of a question.  A front-end that wants it as a `result`
  // event reads `found` from `run_line` instead and may ignore this.
  std::function<void(bool found)> result;
  // `InputStringFromDialog`: the answer of the pending input request, in UTF-8.
  // An empty string ends the input, the way end-of-file does on the console.
  std::function<std::string(const std::string &caption_utf8, bool split_space)> input;
};

// Installs the sinks of a front-end.  An empty `Sinks` restores the console.
void install(Sinks sinks);

// Echo `?цель.` of the running questions, as the `-q` switch asks for.
void set_print_questions(bool on);

// Starts the interpreter: the heap, the variable tables and `buildin`.
// Returns 0 when it is ready, or 1/2/3 after an initialization failure, which it
// has already reported through the `error` sink.
int start();

// The error code `buildin` returned (0 when fine); the console reports it after
// its run, the JSON session after its first request.
int initial_error();

// True while the interpreter is still allowed to run; a stopped query clears it.
bool running();

// Restarts the source-line counter, so the `line` of a diagnostic counts
// from the first line of the program being run rather than from the start
// of the session.
void reset_line_counter();

// Runs one source line, in UTF-8, the way the console reads it.  `was_query` is
// set when the line was a question and `found` when that question was answered
// Yes.  Returns the error code of the line (0 when it was fine); a failure has
// already gone out through the `error` sink.
int run_line(const std::string &utf8_line, bool &was_query, bool &found);

// Reports an error code the way the console does at the end of its run.
void report_error(int code);

}  // namespace host
