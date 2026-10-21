// СОРТИРОВКА/2: the list is put in ascending order by the comparison of БОЛЬШЕ,
// and the repeats are left as one value.  The expectations here are what the built
// interpreter answers, not what the standard order of other Prologs would say.

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "prlib/err.h"
#include "test_helpers.hpp"

using prologd_test::question;
using prologd_test::run;

TEST_CASE("a list of numbers is ordered", "[sort]") {
  REQUIRE(question("СОРТИРОВКА([3,1,2],с)") == "с=[1,2,3]\n");
  REQUIRE(question("СОРТИРОВКА([3,2,1],с)") == "с=[1,2,3]\n");
  REQUIRE(question("СОРТИРОВКА([1,2,3],с)") == "с=[1,2,3]\n");
  REQUIRE(question("СОРТИРОВКА([5],с)") == "с=[5]\n");
  REQUIRE(question("СОРТИРОВКА([],с)") == "с=[]\n");
}

TEST_CASE("the repeats stay as one value", "[sort]") {
  REQUIRE(question("СОРТИРОВКА([3,1,2,1],с)") == "с=[1,2,3]\n");
  REQUIRE(question("СОРТИРОВКА([1,1,1],с)") == "с=[1]\n");
  // 1 and 1.0 are one value for БОЛЬШЕ, and the one that stood first stays.
  REQUIRE(question("СОРТИРОВКА([1,1.0],с)") == "с=[1]\n");
  REQUIRE(question("СОРТИРОВКА([2.0,2],с)") == "с=[2.0]\n");
}

TEST_CASE("a fraction keeps its place among the integers", "[sort]") {
  REQUIRE(question("СОРТИРОВКА([2.5,1,0.5],с)") == "с=[0.5,1,2.5]\n");
}

TEST_CASE("text is ordered the way the engine compares it", "[sort]") {
  REQUIRE(question("СОРТИРОВКА([\"б\",\"а\",\"в\"],с)") == "с=[а,б,в]\n");
  REQUIRE(question("СОРТИРОВКА([боря,аня,вова],с)") == "с=[аня,боря,вова]\n");
  // A word that begins another is the smaller of the two.
  REQUIRE(question("СОРТИРОВКА([\"абв\",\"аб\",\"а\"],с)") == "с=[а,аб,абв]\n");
  // A constant and a string of the same letters are one value.
  REQUIRE(question("СОРТИРОВКА([боря,\"аня\",боря],с)") == "с=[аня,боря]\n");
}

TEST_CASE("nested lists are ordered element by element", "[sort]") {
  REQUIRE(question("СОРТИРОВКА([[2],[1]],с)") == "с=[[1],[2]]\n");
  REQUIRE(question("СОРТИРОВКА([[1,3],[1,2]],с)") == "с=[[1,2],[1,3]]\n");
  // The empty list is smaller than any other.
  REQUIRE(question("СОРТИРОВКА([[], [1]],с)") == "с=[[],[1]]\n");
}

TEST_CASE("the second argument may be given", "[sort]") {
  REQUIRE(question("СОРТИРОВКА([2,1],[1,2])") == "ДА\n");
  REQUIRE(question("СОРТИРОВКА([2,1],[2,1])") == "НЕТ\n");
  REQUIRE(question("СОРТИРОВКА([],[])") == "ДА\n");
  REQUIRE(question("СОРТИРОВКА([],[1])") == "НЕТ\n");
}

TEST_CASE("the list it sorts is left as it was", "[sort]") {
  const std::string program =
      "исходный([3,1]).\n"
      "?исходный(л),СОРТИРОВКА(л,с),РАВНО(л,[3,1]).\n";
  REQUIRE(run(program).output == "л=[3,1]\nс=[1,3]\n");
}

TEST_CASE("it sorts what findall collected", "[sort]") {
  const std::string program =
      "человек(вова).\n"
      "человек(аня).\n"
      "человек(боря).\n"
      "?НАЙТИВСЕ(ч,человек(ч),а),СОРТИРОВКА(а,с).\n";
  REQUIRE(run(program).output == "ч=_1\nа=[вова,аня,боря]\nс=[аня,боря,вова]\n");
}

TEST_CASE("a list that is not a proper list is refused", "[sort]") {
  // A listlike tail, something that is not a list at all, and a list that
  // contains itself - the last one is why the walk is guarded: a plain walk
  // would follow the cycle forever.
  for (const std::string &goal :
       {"СОРТИРОВКА([1|2],с)", "СОРТИРОВКА(5,с)", "РАВНО(ц,[1|ц]),СОРТИРОВКА(ц,с)"}) {
    INFO("goal: " << goal);
    REQUIRE(run("?" + goal + ".\n").error_code ==
            static_cast<int>(ErrorCode::AnExceptionOccurredDuringExecution));
  }
}

TEST_CASE("elements of different kinds are not comparable", "[sort]") {
  // The comparison is the one of БОЛЬШЕ, and it has no order between a number and
  // a word: that is an error here too, not some third ordering.
  REQUIRE(run("?СОРТИРОВКА([1,\"а\"],с).\n").error_code ==
          static_cast<int>(ErrorCode::AnExceptionOccurredDuringExecution));
  REQUIRE(run("?СОРТИРОВКА([[1],[\"а\"]],с).\n").error_code ==
          static_cast<int>(ErrorCode::AnExceptionOccurredDuringExecution));
}

TEST_CASE("the arity of the predicate is checked", "[sort]") {
  const auto answer = run("?СОРТИРОВКА([1]).\n");
  REQUIRE(answer.errors.find("Не верное число аргументов") != std::string::npos);
}
