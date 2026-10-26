// The program: facts, questions, the answers and the diagnostics.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "prlib/err.h"
#include "prlib_config.h"
#include "test_helpers.hpp"

using prologd_test::question;
using prologd_test::run;

TEST_CASE("a question is answered YES or NO", "[program]") {
  REQUIRE(question("БОЛЬШЕ(5,3)") == "ДА\n");
  REQUIRE(question("БОЛЬШЕ(3,5)") == "НЕТ\n");
  // The same answer as the interpreter hands a front-end: a question is solved
  // or it is not.
  REQUIRE(run("?БОЛЬШЕ(5,3).\n").found);
  REQUIRE_FALSE(run("?БОЛЬШЕ(3,5).\n").found);
}

TEST_CASE("a fact is found by its name and arity", "[program]") {
  REQUIRE(run("тест(1).\n?тест(1).\n").output == "ДА\n");
  REQUIRE(run("тест(1).\n?тест(2).\n").output == "НЕТ\n");
  // A name of another arity is another predicate: two arguments find nothing.
  REQUIRE(run("тест(1).\n?тест(1,2).\n").output == "НЕТ\n");
}

TEST_CASE("a free variable is bound and printed", "[program]") {
  REQUIRE(run("тест(1).\n?тест(х).\n").output == "х=1\n");
}

TEST_CASE("every solution of a question is printed", "[program]") {
  const std::string program = "тест(1).\nтест(2).\n?тест(х).\n";
  REQUIRE(run(program).output == "х=1\nх=2\n");
}

TEST_CASE("a cut keeps the first solution", "[program]") {
  const std::string program =
      "тест(1).\n"
      "тест(2).\n"
      "первый(х):-тест(х),!.\n"
      "?первый(х).\n";
  REQUIRE(run(program).output == "х=1\n");
}

TEST_CASE("the questions option only defines the program", "[program]") {
  const std::string program = "тест(1).\n?тест(1).\n";
  REQUIRE(run(program, {}, false).output.empty());
  // ... and a later question still finds what the load defined.
  REQUIRE(run(program, {}, false).ok());
}

TEST_CASE("the input lines of the program start with a dollar", "[program]") {
  const std::string program = "$слово\n?ВВОДСТР(с),ВЫВОД(с).\n";
  REQUIRE(run(program).output.find("слово") != std::string::npos);
}

TEST_CASE("quiet suppresses the answer of its question", "[program]") {
  // `ТИХО` is the switch of one question: the program's own output stays.
  REQUIRE(run("?ТИХО,БОЛЬШЕ(5,3).\n").output.empty());
  REQUIRE(run("?ТИХО,ВЫВОД(9).\n").output == "9");
}

TEST_CASE("version prints the release stamp", "[program]") {
  // One fixed string - the release stamp of the dialect, not the date of the
  // build; the question it answers is a question, so the line is followed by `ДА`.
  REQUIRE(question("ВЕРСИЯ") == std::string(PROLOGD_RELEASE_DATE) + "\nДА\n");
  REQUIRE(std::string(PROLOGD_VERSION_STRING) == PROLOGD_RELEASE_DATE);
}

TEST_CASE("a syntax error names the line it was found on", "[program][errors]") {
  const auto result = run("тест(1).\n?НЕВЕРНЫЙ ~ .\n");
  REQUIRE(result.error_code == static_cast<int>(ErrorCode::InvalidCharacter));
  REQUIRE(result.line == 2);
  REQUIRE(result.errors.find("Ошибка при разборе") != std::string::npos);
}

TEST_CASE("an unknown predicate simply fails", "[program]") {
  const auto result = run("?нетакого(1).\n");
  REQUIRE(result.ok());
  REQUIRE(result.output == "НЕТ\n");
}

TEST_CASE("the welcome program of the window answers both students", "[program]") {
  const std::string program =
      "% пример программы\n"
      "студент(вова, корчагин).\n"
      "студент(данил, ахмеров).\n"
      "\n"
      "?студент(и, ф).\n";
  REQUIRE(run(program).output == "и=вова\nф=корчагин\nи=данил\nф=ахмеров\n");
}
