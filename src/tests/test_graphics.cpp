// Graphics.  The console of this build draws nothing, so every drawing predicate
// has to be seen through the hooks of test_host.cpp, which write down each call.
// The picture a program draws is therefore a list of calls, and nothing else.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "test_helpers.hpp"

using prologd_test::graphics_calls;
using prologd_test::run;

TEST_CASE("a pixel is put on the screen", "[graphics]") {
  REQUIRE(run("?ТОЧКА(1,2,3).\n").output == "ДА\n");
  REQUIRE(graphics_calls() == std::vector<std::string>{"setPixel(1,2,3)"});
}

TEST_CASE("a line is drawn", "[graphics]") {
  REQUIRE(run("?ЛИНИЯ(0,0,10,10,4).\n").output == "ДА\n");
  REQUIRE(graphics_calls() == std::vector<std::string>{"lineTo(0,0,10,10,4)"});
}

TEST_CASE("a circle is drawn", "[graphics]") {
  // ОКРУЖНОСТЬ takes the centre and the radius; the diameter reaches the hook.
  REQUIRE(run("?ОКРУЖНОСТЬ(1,2,3,4).\n").output == "ДА\n");
  REQUIRE(graphics_calls() == std::vector<std::string>{"ellipse(1,2,3,3,4)"});
}

TEST_CASE("an area is filled", "[graphics]") {
  REQUIRE(run("?ЗАКРАСКА(1,1,7).\n").output == "ДА\n");
  REQUIRE(graphics_calls() == std::vector<std::string>{"floodFill(1,1,7)"});
}

TEST_CASE("a pixel is read back as its colour", "[graphics]") {
  // An empty screen of a test is black, and reading neither draws nor fills.
  REQUIRE(run("?ТОЧКА(1,2,х).\n").output == "х=0\n");
  REQUIRE(graphics_calls().empty());
}

TEST_CASE("a program that draws nothing leaves the screen alone", "[graphics]") {
  REQUIRE(run("?БОЛЬШЕ(5,3).\n").output == "ДА\n");
  REQUIRE(graphics_calls().empty());
}

TEST_CASE("a picture is drawn in the order its program asks for", "[graphics]") {
  const std::string program =
      "рамка(x,y,w):-"
      "ТОЧКА(x,y,15),"
      "ЛИНИЯ(x,y,#x+w#,#y+w#,0),"
      "ЗАКРАСКА(#x+1#,#y+1#,7).\n"
      "?рамка(1,1,10).\n";
  REQUIRE(run(program).output == "ДА\n");
  REQUIRE(graphics_calls() == std::vector<std::string>{
                                  "setPixel(1,1,15)",
                                  "lineTo(1,1,11,11,0)",
                                  "floodFill(2,2,7)",
                              });
}
