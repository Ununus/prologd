// Control: negation, disjunction, `ВЫП`, the cut and recursion.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "test_helpers.hpp"

using prologd_test::question;
using prologd_test::run;

TEST_CASE("negation is failure", "[control]") {
  REQUIRE(question("НЕ(БОЛЬШЕ(3,5))") == "ДА\n");
  REQUIRE(question("НЕ(БОЛЬШЕ(5,3))") == "НЕТ\n");
}

TEST_CASE("disjunction takes what is solved", "[control]") {
  REQUIRE(question("ИЛИ(БОЛЬШЕ(1,2),БОЛЬШЕ(5,3))") == "ДА\n");
  REQUIRE(question("ИЛИ(БОЛЬШЕ(1,2),БОЛЬШЕ(3,5))") == "НЕТ\n");
}

TEST_CASE("call runs a goal given as a term", "[control]") {
  REQUIRE(question("ВЫП(БОЛЬШЕ(5,3))") == "ДА\n");
}

TEST_CASE("recursion walks a list", "[control]") {
  const std::string program =
      "длина([],0).\n"
      "длина([_|х],н):-длина(х,м),СЛОЖЕНИЕ(м,1,н).\n"
      "?длина([абв,где,ёж],н).\n";
  REQUIRE(run(program).output == "н=3\n");
}

TEST_CASE("a predicate with several clauses is searched in order", "[control]") {
  const std::string program =
      "цвет(абв).\n"
      "цвет(где).\n"
      "?цвет(абв).\n";
  REQUIRE(run(program).output == "ДА\n");
}

TEST_CASE("fail ends the search", "[control]") {
  const std::string program =
      "плохо(х):-тест(х),ЛОЖЬ.\n"
      "тест(1).\n"
      "?плохо(1).\n";
  REQUIRE(run(program).output == "НЕТ\n");
}

TEST_CASE("the trace switches do not change the answer", "[control]") {
  // This engine has no stepping, but `ТРАССА`/`НЕТ_ТРАССЫ` are part of the
  // dialect: a program that uses them must still run.
  REQUIRE(run("?НЕТ_ТРАССЫ,БОЛЬШЕ(5,3).\n").output == "ДА\n");
}

TEST_CASE("wait is a delay and changes nothing", "[control]") {
  REQUIRE(question("ЖДИ(1)") == "ДА\n");
}

TEST_CASE("a random walk of the database keeps its count", "[control]") {
  const std::string program =
      "факт(1).\n"
      "факт(2).\n"
      "факт(3).\n"
      "все(н):-СКОЛЬКО(факт,н).\n"
      "?все(н).\n";
  REQUIRE(run(program).output == "н=3\n");
}
