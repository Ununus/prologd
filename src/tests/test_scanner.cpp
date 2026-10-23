// The scanner: what a name is, and the quotes an atom is written in.
//
// The dialect marks a variable with an apostrophe (`переменная'`), so a name is
// either a constant or a variable; this file covers the two extensions that make
// the choice wider - `'атом'` (a constant in quotes, spaces allowed) and `_имя`
// (a variable without the apostrophe).

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "prlib/err.h"
#include "test_helpers.hpp"

using prologd_test::question;
using prologd_test::run;

TEST_CASE("an atom in quotes is a constant", "[scanner]") {
  REQUIRE(run("?ВЫВОД('Тоже атом').\n").output == "Тоже атом\nДА\n");
  REQUIRE(run("пункт('Тоже атом').\n?пункт(х).\n").output == "х=Тоже атом\n");
}

TEST_CASE("an atom in quotes is the same constant as the bare one", "[scanner]") {
  REQUIRE(run("пункт(атом).\n?пункт('атом').\n").output == "ДА\n");
  REQUIRE(run("пункт('атом').\n?пункт(атом).\n").output == "ДА\n");
  REQUIRE(run("пункт(атом).\n?пункт('атом!').\n").output == "НЕТ\n");
}

TEST_CASE("an atom in quotes keeps the signs that separate otherwise", "[scanner]") {
  REQUIRE(run("пункт('а, б').\n?пункт(х).\n").output == "х=а, б\n");
  REQUIRE(run("пункт('а.б').\n?пункт(х).\n").output == "х=а.б\n");
  REQUIRE(run("пункт('f(x)').\n?пункт(х).\n").output == "х=f(x)\n");
  // A name in quotes is a name of a predicate as well.
  REQUIRE(run("пункт(1).\n?'пункт'(1).\n").output == "ДА\n");
  // Double quotes are still a string, and an apostrophe inside one is its own.
  REQUIRE(run("?ВЫВОД(\"а'б\").\n").output == "а'б\nДА\n");
}

TEST_CASE("an atom in quotes must be closed", "[scanner]") {
  const prologd_test::Run result = run("?ВЫВОД('незакрытый).\n");
  REQUIRE(result.error_code == static_cast<int>(ErrorCode::InvalidAtomFormat));
  REQUIRE(result.errors.find("Неверный формат атома") != std::string::npos);
}

TEST_CASE("an apostrophe after a name still makes it a variable", "[scanner]") {
  REQUIRE(question("СЛОЖЕНИЕ(1,2,х')") == "х'=3\n");
  // A one-letter name is a variable without the apostrophe, as it always was.
  REQUIRE(question("СЛОЖЕНИЕ(1,2,х)") == "х=3\n");
}

TEST_CASE("a name that starts with an underscore is a variable", "[scanner]") {
  REQUIRE(question("СЛОЖЕНИЕ(1,2,_Сумма)") == "_Сумма=3\n");
  // The apostrophe is not needed there and does not change the name.
  REQUIRE(question("СЛОЖЕНИЕ(1,2,_Сумма')") == "_Сумма=3\n");
  REQUIRE(question("СЛОЖЕНИЕ(1,2,_Пер)") == "_Пер=3\n");
}

TEST_CASE("the same name in two places is the same variable", "[scanner]") {
  const std::string program =
      "пункт(_Арг):-БОЛЬШЕ(_Арг',1).\n"
      "?пункт(3).\n"
      "?пункт(0).\n";
  REQUIRE(run(program).output == "ДА\nНЕТ\n");
  REQUIRE(question("СЛОЖЕНИЕ(1,2,_А),СЛОЖЕНИЕ(_Б,0,_А)") == "_А=3\n_Б=3\n");
}

TEST_CASE("names differ in their case", "[scanner]") {
  REQUIRE(question("СЛОЖЕНИЕ(1,2,_Пер),СЛОЖЕНИЕ(_Пер,0,_пер)") == "_Пер=3\n_пер=3\n");
}

TEST_CASE("the underscore alone is still an anonymous variable", "[scanner]") {
  REQUIRE(run("пара(1,2).\n?пара(_,_).\n").output == "ДА\n");
}
