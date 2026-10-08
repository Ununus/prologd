// Strings and the type checks the dialect writes in words.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "test_helpers.hpp"

using prologd_test::question;
using prologd_test::run;

TEST_CASE("the length of a string", "[strings]") {
  REQUIRE(question("ДЛИНА(\"абв\",х)") == "х=3\n");
  REQUIRE(question("ДЛИНА(\"\",х)") == "х=0\n");
}

TEST_CASE("two strings become one", "[strings]") {
  REQUIRE(question("СЦЕП(\"абв\",\"где\",х)") == "х=абвгде\n");
  REQUIRE(question("СЦЕП(\"\",\"где\",х)") == "х=где\n");
}

TEST_CASE("a string becomes a number", "[strings]") {
  REQUIRE(question("СТРЦЕЛ(\"12\",х)") == "х=12\n");
  REQUIRE(question("СТРВЕЩ(\"1.5\",х)") == "х=1.5\n");
}

TEST_CASE("a part of a string is a string of its own", "[strings]") {
  REQUIRE(question("КОПИЯ(\"абв\",1,2,х)") == "х=аб\n");
}

TEST_CASE("a whole number becomes a real one", "[strings]") {
  REQUIRE(question("ЦЕЛВЕЩ(3,х)") == "х=3.0\n");
}

TEST_CASE("a letter and a digit are found by their place", "[strings]") {
  // The place counts from one, as `БУКВА("абв",1)` is `а`.
  REQUIRE(question("БУКВА(\"абв\",1)") == "ДА\n");
  REQUIRE(question("БУКВА(\"1бв\",1)") == "НЕТ\n");
  REQUIRE(question("ЦИФРА(\"1бв\",1)") == "ДА\n");
  REQUIRE(question("ЦИФРА(\"абв\",1)") == "НЕТ\n");
}

TEST_CASE("a string becomes the list of its codes", "[strings]") {
  const std::string answer = question("СТРСПИС(\"абв\",х)");
  REQUIRE(answer.rfind("х=[", 0) == 0);
}

TEST_CASE("printing without a newline", "[strings]") {
  // What the program prints itself is followed by the answer on its own line.
  REQUIRE(run("?ВЫВОД(42).\n").output == "42\nДА\n");
  REQUIRE(run("?ВЫВОД(1,2).\n").output == "12\nДА\n");
}

TEST_CASE("a character is printed by its code", "[strings]") {
  REQUIRE(run("?ВВКОД(65).\n").output == "A\nДА\n");
}
