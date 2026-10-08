#include "protocol.hpp"

#include <fstream>
#include <istream>
#include <iterator>
#include <list>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "host.hpp"
#include "preprocessor.h"
#include "prlib/encoding.h"
#include "prlib/keywords.h"

namespace prologd_cli {
namespace {

using Json = nlohmann::json;

//: The wire version this session answers with.  It is a *subset* of prologd2's
//: protocol 13: the requests and events below exist, the stepping, tracing,
//: formatting and graphics ones do not (this engine has none of them), and a
//: front-end is expected to read what it knows and ignore the rest.
constexpr int kProtocolVersion = 13;

/// The reserved names of the engine, as the `ready` event carries them: the same
/// list `prologd --list-keywords` prints, with the kind of every name (`mod` and
/// `div` are operators of an expression, not goals).  The engine keeps its names
/// in CP1251 and the protocol speaks UTF-8.
Json keywords_event() {
  Json entries = Json::array();
  for (const prologd::ReservedName &name : prologd::reserved_names()) {
    Json entry{{"name", prologd::cp1251_to_utf8(name.name)},
               {"kind", name.callable ? "predicate" : "operator"}};
    if (name.variadic) {
      entry["variadic"] = true;
    } else if (!name.arities.empty()) {
      Json arities = Json::array();
      for (size_t arity : name.arities) {
        arities.push_back(arity);
      }
      entry["arities"] = arities;
    }
    entries.push_back(entry);
  }
  return entries;
}

/// Splits text into source lines the way the console's `getline` does.  A
/// trailing newline is a terminator, not an empty line.
std::list<std::string> split_lines(const std::string &text) {
  std::list<std::string> lines;
  std::string line;
  for (const char ch : text) {
    if (ch == '\n') {
      if (!line.empty() && line.back() == '\r') {
        line.pop_back();
      }
      lines.push_back(line);
      line.clear();
    } else {
      line.push_back(ch);
    }
  }
  if (!line.empty()) {
    lines.push_back(line);
  }
  return lines;
}

/// A `?цель.` line, which is what `questions:false` loads *without* running.
bool is_query_line(const std::string &line) {
  for (const char ch : line) {
    if (ch == ' ' || ch == '\t') {
      continue;
    }
    return ch == '?';
  }
  return false;
}

/// One JSON-line session: installs the interpreter's sinks, reads requests and
/// answers with events.
class Session {
 public:
  Session(std::istream &in, std::ostream &out) : in_(in), out_(out) {}

  int run() {
    host::install(make_sinks());
    emit(Json{{"event", "ready"},
              {"protocol", kProtocolVersion},
              {"version", "prologd"},
              {"keywords", keywords_event()}});

    const int init = host::start();
    if (init != 0) {
      return init;
    }
    const int build_error = host::initial_error();
    if (build_error != 0) {
      host::report_error(build_error);
    }

    std::string line;
    while (std::getline(in_, line)) {
      if (line.empty()) {
        continue;
      }
      Json request;
      try {
        request = Json::parse(line);
      } catch (const Json::exception &parse_error) {
        fail("protocol", std::string("некорректный JSON: ") + parse_error.what());
        continue;
      }
      if (!request.is_object()) {
        fail("protocol", "запрос должен быть объектом JSON");
        continue;
      }
      const std::string command = request.value("cmd", std::string());
      if (command == "quit") {
        break;
      }
      if (command == "load" || command == "query") {
        try {
          if (command == "load") {
            load(request);
          } else {
            query(request);
          }
        } catch (const Json::exception &bad_field) {
          fail("protocol", command + ": " + bad_field.what());
        }
        continue;
      }
      if (command == "input") {
        fail("protocol", "нет ожидающего запроса ввода");
        continue;
      }
      // `step`, `format`, `graphics`, `interrupt` and anything else the engine
      // of this implementation has no answer for are ignored on purpose.
    }

    emit(Json{{"event", "bye"}});
    return exit_code_;
  }

 private:
  host::Sinks make_sinks() {
    host::Sinks sinks;
    sinks.out_line = [this](const std::string &text) {
      at_line_start_ = true;
      emit(Json{{"event", "output"}, {"text", text}});
    };
    sinks.out_raw = [this](const std::string &text) {
      at_line_start_ = false;
      emit(Json{{"event", "output"}, {"text", text}});
    };
    sinks.error = [this](const std::string &text, int line, const std::string &kind) {
      Json event{{"event", "error"}, {"kind", kind}, {"severity", "error"}, {"message", text}};
      if (line > 0) {
        event["line"] = line;
      }
      emit(event);
    };
    // `prdout` is the Yes/No of a question whose goal has no variables: the
    // interpreter prints «ДА» or «НЕТ» for it (a goal with variables prints its
    // bindings instead), and the window shows that line as the answer - it reads
    // the `result` event only for the flag.  So the text a console user sees is
    // sent here, on its own line, and a question that finds nothing no longer
    // concludes in silence.
    sinks.result = [this](bool value) {
      std::string text;
      if (!at_line_start_) {
        // The program left a half-line of its own (`ВЫВОД` without `ПС`), exactly
        // as the console's `prdout` answers after a `usrout`.
        text += '\n';
      }
      text += value ? "ДА" : "НЕТ";
      text += '\n';
      at_line_start_ = true;
      emit(Json{{"event", "output"}, {"text", text}});
    };
    sinks.input = [this](const std::string &caption, bool split_space) {
      return read_input_answer(caption, split_space);
    };
    return sinks;
  }

  /// Runs a program: a file (`path`) or text (`text`), then its own questions.
  /// `questions:false` defines the program without running the `?`-statements,
  /// which is what the editor asks for when one query is clicked.
  void load(const Json &request) {
    const bool has_path = request.contains("path");
    const bool has_text = request.contains("text");
    if (has_path == has_text) {
      fail("protocol", "load: нужен ровно один из параметров path или text");
      return;
    }
    host::reset_line_counter();
    host::set_print_questions(request.value("printQuestions", false));
    const bool questions = request.value("questions", true);

    std::list<std::string> lines;
    if (has_path) {
      const std::string path = request.at("path").get<std::string>();
      std::ifstream file(path, std::ios::binary);
      if (!file) {
        fail("parse", "Cannot open file: " + path);
        exit_code_ = 2;
        return;
      }
      std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      lines = split_lines(text);
    } else {
      lines = split_lines(request.at("text").get<std::string>());
    }

    try {
      lines = preprocessor_run_on_source(lines);
    } catch (const std::runtime_error &import_error) {
      fail("parse", import_error.what());
      exit_code_ = 2;
      return;
    }

    for (const std::string &source_line : lines) {
      if (!questions && is_query_line(source_line)) {
        continue;
      }
      bool was_query = false;
      bool found = false;
      const int err = host::run_line(source_line, was_query, found);
      if (was_query) {
        emit(Json{{"event", "result"}, {"found", found}});
      }
      if (err != 0) {
        host::report_error(err);
        exit_code_ = 2;
        return;  // like the console: a failing line ends the run
      }
    }
    emit(Json{{"event", "loaded"}});
  }

  /// Runs one statement, usually `?Goal.`.
  void query(const Json &request) {
    if (!request.contains("text")) {
      fail("protocol", "query: нужен параметр text");
      return;
    }
    std::string text = request.at("text").get<std::string>();
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
      text.pop_back();
    }
    host::reset_line_counter();
    host::set_print_questions(false);

    bool was_query = false;
    bool found = false;
    const int err = host::run_line(text, was_query, found);
    if (was_query) {
      emit(Json{{"event", "result"}, {"found", found}});
    }
    if (err != 0) {
      host::report_error(err);
      exit_code_ = 3;
    }
  }

  /// Answers a pending `input_request` by reading requests until an `input`
  /// arrives; the interpreter is inside a query, so nothing else reads `in_`.
  std::string read_input_answer(const std::string &caption, bool split_space) {
    (void)caption;  // the front-end sees the kind; the caption is the console's
    emit(Json{{"event", "input_request"}, {"kind", split_space ? "token" : "line"}});
    std::string line;
    while (std::getline(in_, line)) {
      if (line.empty()) {
        continue;
      }
      Json request;
      try {
        request = Json::parse(line);
      } catch (const Json::exception &parse_error) {
        fail("protocol", std::string("некорректный JSON: ") + parse_error.what());
        continue;
      }
      if (!request.is_object()) {
        fail("protocol", "запрос должен быть объектом JSON");
        continue;
      }
      const std::string command = request.value("cmd", std::string());
      if (command == "input") {
        return request.value("text", std::string());
      }
      if (command == "interrupt" || command == "quit") {
        return std::string();  // end of input: the pending read fails
      }
      fail("protocol", "ожидался ответ input, получено: " + command);
    }
    return std::string();
  }

  void fail(const std::string &kind, const std::string &message) {
    emit(Json{{"event", "error"}, {"kind", kind}, {"severity", "error"}, {"message", message}});
  }

  void emit(const Json &event) {
    out_ << event.dump() << '\n';
    out_.flush();
  }

  std::istream &in_;
  std::ostream &out_;
  int exit_code_ = 0;
  //: Whether the last text sent to the front-end ended its line — the console
  //: keeps the same flag, and the answer of a question follows it.
  bool at_line_start_ = true;
};

}  // namespace

int run_json_session(std::istream &in, std::ostream &out) {
  Session session(in, out);
  return session.run();
}

}  // namespace prologd_cli
