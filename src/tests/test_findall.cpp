// НАЙТИВСЕ: все решения цели и список подстановок шаблона (findall).
//
// Цель решается как отдельный вопрос, поэтому её связывания не достаются
// вызывающему, а подстановки шаблона копируются: каждая — со своими переменными.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "prlib/err.h"
#include "test_helpers.hpp"

using prologd_test::question;
using prologd_test::run;

TEST_CASE("a template collects the substitutions of the goal", "[findall]") {
  const std::string program =
      "студент(вова, корчагин).\n"
      "студент(данил, ахмеров).\n"
      "студент(ваня, петров).\n"
      "\n"
      "?НАЙТИВСЕ(И, студент(И, Ф), А).\n"
      "?НАЙТИВСЕ([И, Ф], студент(И, Ф), А).\n";
  REQUIRE(run(program).output ==
          "И=_1\n"
          "Ф=_1\n"
          "А=[вова,данил,ваня]\n"
          "И=_1\n"
          "Ф=_1\n"
          "А=[[вова,корчагин],[данил,ахмеров],[ваня,петров]]\n");
}

TEST_CASE("an answer of the goal reaches the template", "[findall]") {
  const std::string program =
      "пара(1, аб).\n"
      "пара(2, бв).\n"
      "?НАЙТИВСЕ(у, пара(х, у), м).\n";
  REQUIRE(run(program).output ==
          "у=_1\n"
          "х=_1\n"
          "м=[аб,бв]\n");
}

TEST_CASE("the goal keeps its bindings to itself", "[findall]") {
  const std::string program =
      "пара(1, аб).\n"
      "пара(2, бв).\n"
      "?НАЙТИВСЕ(пара(х, у), пара(1, аб), м).\n";
  // В шаблоне нет ни одной переменной цели, поэтому обе переменные шаблона
  // свободны, и у каждого ответа они свои.
  REQUIRE(run(program).output ==
          "х=_1\n"
          "у=_1\n"
          "м=[пара(_1,_2)]\n");
}

TEST_CASE("every answer gets its own variables", "[findall]") {
  const std::string program =
      "факт(1).\n"
      "факт(2).\n"
      "?НАЙТИВСЕ(х, факт(у), м).\n"
      "?НАЙТИВСЕ(у, факт(у), м).\n";
  REQUIRE(run(program).output ==
          "х=_1\n"
          "у=_1\n"
          "м=[_1,_2]\n"
          "у=_1\n"
          "м=[1,2]\n");
}

TEST_CASE("a goal without answers gives an empty list", "[findall]") {
  REQUIRE(question("НАЙТИВСЕ(х,ЛОЖЬ,м)") == "х=_1\nм=[]\n");
}

TEST_CASE("a constant template is repeated in the list", "[findall]") {
  const std::string program =
      "число(2).\n"
      "?НАЙТИВСЕ(2, число(2), м).\n";
  REQUIRE(run(program).output == "м=[2]\n");
}

TEST_CASE("a nested findall is one answer of the goal", "[findall]") {
  const std::string program =
      "число(1).\n"
      "число(2).\n"
      "число(3).\n"
      "?НАЙТИВСЕ(А, НАЙТИВСЕ(Б, число(Б), А), Л).\n";
  REQUIRE(run(program).output ==
          "А=_1\n"
          "Б=_1\n"
          "Л=[[1,2,3]]\n");
}

TEST_CASE("two findalls of one query keep their own lists", "[findall]") {
  const std::string program =
      "число(1).\n"
      "число(2).\n"
      "?НАЙТИВСЕ(х, число(х), к), НАЙТИВСЕ(у, число(у), м), ВЫВОД(к), ПС, ВЫВОД(м), ПС.\n";
  REQUIRE(run(program).output ==
          "[1,2]\n"
          "[1,2]\n"
          "х=_1\n"
          "к=[1,2]\n"
          "у=_1\n"
          "м=[1,2]\n");
}

TEST_CASE("an error of the goal is not swallowed", "[findall]") {
  const std::string program =
      "число(1).\n"
      "?НАЙТИВСЕ(х, СЛОЖЕНИЕ(1, 2), м).\n";
  const prologd_test::Run answer = run(program);
  REQUIRE(answer.output == "НЕТ\n");
  REQUIRE(answer.errors.find("Не верное число аргументов") != std::string::npos);
}

TEST_CASE("a goal that is not a goal is rejected", "[findall]") {
  const prologd_test::Run answer = run("?НАЙТИВСЕ(х, ц, м).\n");
  REQUIRE(answer.error_code == static_cast<int>(ErrorCode::AnExceptionOccurredDuringExecution));
  REQUIRE(answer.output.empty());
}
