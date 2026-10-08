// Input: the `$` lines of the program, and the caller's answers when the program
// has none left.  A test does not show the question the engine asks (`Введите
// строку`), so only the program's own printing and the answer are read here.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "test_helpers.hpp"

using prologd_test::run;

TEST_CASE("a dollar line is the program's own input", "[input]") {
  REQUIRE(run("$слово\n?ВВОДСТР(с),ВЫВОД(с).\n").output == "словос=слово\n");
}

TEST_CASE("a string is asked for and read", "[input]") {
  REQUIRE(run("?ВВОДСТР(с),ВЫВОД(с).\n", {"привет"}).output == "приветс=привет\n");
}

TEST_CASE("a whole number is asked for and read", "[input]") {
  const std::string output = run("?ВВОДЦЕЛ(н),ВЫВОД(н).\n", {"42"}).output;
  REQUIRE(output.find("н=42") != std::string::npos);
}

TEST_CASE("a real number is asked for and read", "[input]") {
  const std::string output = run("?ВВОДВЕЩ(н),ВЫВОД(н).\n", {"1.5"}).output;
  REQUIRE(output.find("н=1.5") != std::string::npos);
}

TEST_CASE("one character is asked for and read", "[input]") {
  const std::string output = run("?ВВОДСИМВ(с),ВЫВОД(с).\n", {"а"}).output;
  REQUIRE(output.find("с=а") != std::string::npos);
}

TEST_CASE("what was not asked for is never read", "[input]") {
  // Only the first `$` line is used, and it answers the question that asks.
  const std::string program =
      "$первое\n"
      "?ВВОДСТР(с),ВЫВОД(с).\n";
  REQUIRE(run(program).output == "первоес=первое\n");
}

TEST_CASE("input that runs out is an error of the program", "[input][errors]") {
  // As in the console: the harness says so, and the goal fails - `ВВОДЦЕЛ` of
  // nothing is a goal that is not solved, and the answer is therefore НЕТ.
  const auto result = run("?ВВОДЦЕЛ(н).\n");
  REQUIRE(result.output == "НЕТ\n");
  REQUIRE(result.error_code == 0);
  REQUIRE(result.errors.find("Недостаточно входных данных") != std::string::npos);
}
