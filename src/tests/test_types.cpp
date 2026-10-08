// The type checks.  The Russian words look at the type of what they are given,
// while `int`/`float` of the old English spelling accept any number.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "test_helpers.hpp"

using prologd_test::question;

TEST_CASE("a whole number is a whole number", "[types]") {
  REQUIRE(question("ЦЕЛ(3)") == "ДА\n");
  REQUIRE(question("ЦЕЛ(3.0)") == "НЕТ\n");
}

TEST_CASE("a real number is a real number", "[types]") {
  REQUIRE(question("ВЕЩ(3.0)") == "ДА\n");
  REQUIRE(question("ВЕЩ(3)") == "НЕТ\n");
}

TEST_CASE("int and float take a number of either kind", "[types]") {
  REQUIRE(question("int(3)") == "ДА\n");
  REQUIRE(question("int(3.0)") == "ДА\n");
  REQUIRE(question("float(3)") == "ДА\n");
  REQUIRE(question("float(3.0)") == "ДА\n");
}

TEST_CASE("a symbol is a name in quotes", "[types]") {
  REQUIRE(question("СИМВ(\"а\")") == "ДА\n");
  REQUIRE(question("СИМВ(а)") == "НЕТ\n");
}

TEST_CASE("a list is a list", "[types]") {
  REQUIRE(question("СПИСОК([])") == "ДА\n");
  REQUIRE(question("СПИСОК([1])") == "ДА\n");
  REQUIRE(question("СПИСОК(1)") == "НЕТ\n");
}

TEST_CASE("a term becomes its list", "[types]") {
  // A term of the database is a list of its functor and its arguments.
  REQUIRE(question("ТЕРМ(тест(1),х)") == "х=[тест,1]\n");
  REQUIRE(question("ТЕРМ(тест,х)") == "х=[тест]\n");
}
