// ДЛЯВСЕХ(Условие, Следствие) — аналог forall: у каждого решения Условия есть
// решение Следствия.
//
// Это та же проверка, что НЕ((Условие,НЕ(Следствие))), но записать её руками в
// этом диалекте нельзя: конъюнкция не может быть аргументом предиката.  Условие
// решается отдельно от вопроса (как цель НАЙТИВСЕ), поэтому его связывания
// наружу не выходят, а Следствие проверяется в каждом полученном ответе.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "prlib/err.h"
#include "test_helpers.hpp"

using prologd_test::question;
using prologd_test::run;

TEST_CASE("every solution of the condition must satisfy the consequence", "[forall]") {
  const std::string program =
      "факт(1).\n"
      "факт(2).\n"
      "мал(1).\n"
      "мал(2).\n"
      "?ДЛЯВСЕХ(факт(Х), мал(Х)).\n";
  // Проверка истинна, но ничего не связывает: переменная вопроса остаётся свободной.
  REQUIRE(run(program).output == "Х=_1\n");
}

TEST_CASE("one solution without a consequence makes it false", "[forall]") {
  const std::string program =
      "факт(1).\n"
      "факт(2).\n"
      "мал(1).\n"
      "?ДЛЯВСЕХ(факт(Х), мал(Х)).\n";
  REQUIRE(run(program).output == "НЕТ\n");
}

TEST_CASE("a condition without solutions is true", "[forall]") {
  const std::string program =
      "факт(1).\n"
      "?ДЛЯВСЕХ(факт(9), мал(9)).\n";
  REQUIRE(run(program).output == "ДА\n");
}

TEST_CASE("an undefined condition is one without solutions", "[forall]") {
  // У неопределённого предиката нет предложений, то есть нет и решений: для
  // ДЛЯВСЕХ это то же, что «решений нет».
  REQUIRE(run("?ДЛЯВСЕХ(нет(1), факт(1)).\n").output == "ДА\n");
}

TEST_CASE("an undefined consequence is a counterexample", "[forall]") {
  const std::string program =
      "факт(1).\n"
      "?ДЛЯВСЕХ(факт(1), нет(Х)).\n";
  REQUIRE(run(program).output == "НЕТ\n");
}

TEST_CASE("ground goals and goals without arguments", "[forall]") {
  const std::string program =
      "факт(1).\n"
      "мал(1).\n"
      "?ДЛЯВСЕХ(факт(1), мал(1)).\n"
      "?ДЛЯВСЕХ(факт(1), мал(2)).\n";
  REQUIRE(run(program).output ==
          "ДА\n"
          "НЕТ\n");
  REQUIRE(run("есть.\nбыло.\n?ДЛЯВСЕХ(есть, было).\n").output == "ДА\n");
}

TEST_CASE("the condition may use the anonymous variable", "[forall]") {
  const std::string program =
      "факт(1).\n"
      "факт(2).\n"
      "?ДЛЯВСЕХ(факт(_), РАВНО(1,1)).\n";
  REQUIRE(run(program).output == "ДА\n");
}

TEST_CASE("the predicate is a goal like any other in a clause body", "[forall]") {
  const std::string all_pass =
      "студент(вова).\n"
      "студент(данил).\n"
      "сдал(вова).\n"
      "сдал(данил).\n"
      "все_сдали:-ДЛЯВСЕХ(студент(И), сдал(И)).\n"
      "?все_сдали.\n";
  REQUIRE(run(all_pass).output == "ДА\n");
  const std::string one_fails =
      "студент(вова).\n"
      "студент(данил).\n"
      "сдал(вова).\n"
      "все_сдали:-ДЛЯВСЕХ(студент(И), сдал(И)).\n"
      "?все_сдали.\n";
  REQUIRE(run(one_fails).output == "НЕТ\n");
}

TEST_CASE("a double negation is a goal like any other", "[forall]") {
  const std::string with_not =
      "число(1).\n"
      "число(2).\n"
      "чёт(2).\n"
      "?ДЛЯВСЕХ(число(Х), НЕ(НЕ(чёт(Х)))).\n";
  REQUIRE(run(with_not).output == "НЕТ\n");
  const std::string with_or =
      "факт(1).\n"
      "мал(2).\n"
      "?ДЛЯВСЕХ(факт(1), ИЛИ(мал(1), мал(2))).\n";
  REQUIRE(run(with_or).output == "ДА\n");
}

TEST_CASE("a builtin predicate is a goal like any other", "[forall]") {
  // Сравнения с постоянными аргументами: условие решается отдельно от вопроса, а
  // не через ВЫП, поэтому они здесь работают так же, как в любом другом теле.
  const std::string in_the_condition =
      "факт(2).\n"
      "?ДЛЯВСЕХ(БОЛЬШЕ(2,1), факт(2)).\n"
      "?ДЛЯВСЕХ(БОЛЬШЕ(2,2), факт(2)).\n";
  REQUIRE(run(in_the_condition).output ==
          "ДА\n"
          "ДА\n");
  const std::string in_the_consequence =
      "факт(2).\n"
      "?ДЛЯВСЕХ(факт(2), БОЛЬШЕ(2,1)).\n"
      "?ДЛЯВСЕХ(факт(2), БОЛЬШЕ(2,2)).\n";
  REQUIRE(run(in_the_consequence).output ==
          "ДА\n"
          "НЕТ\n");
}

TEST_CASE("a comparison reads the variable the condition bound", "[forall]") {
  const std::string program =
      "элемент(1).\n"
      "элемент(2).\n"
      "?ДЛЯВСЕХ(элемент(Х), БОЛЬШЕ(Х,0)).\n"
      "?ДЛЯВСЕХ(элемент(Х), БОЛЬШЕ(Х,1)).\n";
  REQUIRE(run(program).output ==
          "Х=_1\n"
          "НЕТ\n");
}

TEST_CASE("a nested call is a goal of its own", "[forall]") {
  const std::string program =
      "пара(1,а).\n"
      "пара(2,б).\n"
      "хорошо(а).\n"
      "хорошо(б).\n"
      "?ДЛЯВСЕХ(пара(Х,У), ДЛЯВСЕХ(пара(Х,У), хорошо(У))).\n";
  REQUIRE(run(program).output ==
          "Х=_1\n"
          "У=_1\n");
}

TEST_CASE("the consequence is asked once per solution of the condition", "[forall]") {
  const std::string program =
      "число(1).\n"
      "число(2).\n"
      "число(3).\n"
      "?ДЛЯВСЕХ(число(Х), ВЫВОД(Х)).\n";
  // Три решения условия — три вывода, а связывания условия наружу не выходят.
  REQUIRE(run(program).output == "123Х=_1\n");
}

TEST_CASE("the search stops at the first counterexample", "[forall]") {
  const std::string program =
      "число(1).\n"
      "число(2).\n"
      "число(3).\n"
      "мал(1).\n"
      "мал(3).\n"
      "печ(Х):-число(Х),ВЫВОД(Х).\n"
      "?ДЛЯВСЕХ(печ(Х), мал(Х)), ПС.\n";
  // Печать самого условия останавливается на контрпримере: 3 не выводится.
  REQUIRE(run(program).output ==
          "12\n"
          "НЕТ\n");
}

TEST_CASE("the predicate is a goal findall can collect", "[forall]") {
  const std::string program =
      "факт(1).\n"
      "факт(2).\n"
      "мал(1).\n"
      "мал(2).\n"
      "?НАЙТИВСЕ(Х, ДЛЯВСЕХ(факт(Х), мал(Х)), С).\n";
  REQUIRE(run(program).output ==
          "Х=_1\n"
          "С=[_1]\n");
}

TEST_CASE("an argument that is not a goal is rejected", "[forall]") {
  // The message travels to the console (prologdc prints what it caught), while a
  // test sees only the code: `run` keeps the exception's text to itself.
  const prologd_test::Run answer = run("?ДЛЯВСЕХ(1, 2).\n");
  REQUIRE(answer.error_code == static_cast<int>(ErrorCode::AnExceptionOccurredDuringExecution));
  REQUIRE(answer.output.empty());
}

TEST_CASE("the arity of the predicate is checked", "[forall]") {
  const prologd_test::Run answer = run("факт(1).\n?ДЛЯВСЕХ(факт(1)).\n");
  REQUIRE(answer.output == "НЕТ\n");
  REQUIRE(answer.errors.find("Не верное число аргументов") != std::string::npos);
}
