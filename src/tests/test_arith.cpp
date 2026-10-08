// Arithmetic: the four operations, the two-argument comparisons, the built-in
// functions and the integers that are wider than a machine word.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "test_helpers.hpp"

using prologd_test::question;

TEST_CASE("addition and multiplication answer their third argument", "[arith]") {
  REQUIRE(question("СЛОЖЕНИЕ(2,3,х)") == "х=5\n");
  REQUIRE(question("УМНОЖЕНИЕ(2,3,х)") == "х=6\n");
  // The four-argument form multiplies by the first and adds the third.
  REQUIRE(question("УМНОЖЕНИЕ(2,3,4,х)") == "х=10\n");
}

TEST_CASE("the operations keep the fraction", "[arith]") {
  REQUIRE(question("СЛОЖЕНИЕ(1.5,2.5,х)") == "х=4.0\n");
  REQUIRE(question("УМНОЖЕНИЕ(1.5,2.0,х)") == "х=3.0\n");
}

TEST_CASE("comparisons answer YES or NO", "[arith]") {
  REQUIRE(question("РАВНО(2,2)") == "ДА\n");
  REQUIRE(question("РАВНО(2,3)") == "НЕТ\n");
  REQUIRE(question("БОЛЬШЕ(5,3)") == "ДА\n");
  REQUIRE(question("МЕНЬШЕ(3,5)") == "ДА\n");
  REQUIRE(question("БОЛЬШЕ(2.5,2.0)") == "ДА\n");
}

TEST_CASE("ABS and SQRT", "[arith]") {
  REQUIRE(question("ABS(#0-5#,х)") == "х=5\n");
  REQUIRE(question("ABS(5,х)") == "х=5\n");
  REQUIRE(question("SQRT(16,х)") == "х=4\n");
  // A whole root: the fractional part is dropped, as the first version did.
  REQUIRE(question("SQRT(15,х)") == "х=3\n");
}

TEST_CASE("the trigonometric functions want a real argument", "[arith]") {
  REQUIRE(question("SIN(0.0,х)") == "х=0.0\n");
  REQUIRE(question("COS(0.0,х)") == "х=1.0\n");
  REQUIRE(question("TAN(0.0,х)") == "х=0.0\n");
  REQUIRE(question("EXP(0.0,х)") == "х=1.0\n");
}

TEST_CASE("a random number is an integer", "[arith]") {
  const std::string answer = question("СЛУЧ(х)");
  REQUIRE(answer.rfind("х=", 0) == 0);
  REQUIRE(answer.back() == '\n');
}

TEST_CASE("the integers are wider than a machine word", "[arith][big-int]") {
  // 30! — the factorial that does not fit in any built-in type.
  const std::string factorial =
      "факториал(0,1):-!;\n"
      "факториал(н,ф):-факториал(#н-1#,х),УМНОЖЕНИЕ(х,н,ф);\n"
      "?факториал(30,ф);\n";
  REQUIRE(prologd_test::run(factorial).output ==
          "ф=265252859812191058636308480000000\n");

  // A product of two twenty-digit numbers: forty digits, exact.
  REQUIRE(question("УМНОЖЕНИЕ(12345678901234567890,98765432109876543210,х)") ==
          "х=1219326311370217952237463801111263526900\n");

  // A square root of a twenty-five-digit number.
  REQUIRE(question("SQRT(1000000000000000000000000,х)") == "х=1000000000000\n");
}

TEST_CASE("a calculation never loses precision to a float", "[arith][big-int]") {
  // 2^100 is exact; a double would round it.
  const std::string program =
      "степень(0,1):-!;\n"
      "степень(н,р):-степень(#н-1#,м),УМНОЖЕНИЕ(м,2,р);\n"
      "?степень(100,р);\n";
  REQUIRE(prologd_test::run(program).output == "р=1267650600228229401496703205376\n");
}
