// Занятые имена движка: список, который интерфейс читает командой
// `prologd --list-keywords` и полем `keywords` события `ready`.
//
// Таблица ниже — то, что движок принимает на самом деле (арности сверены с
// поведением: на чужую арность консоль отвечает «Не верное число аргументов во
// встроенном предикате»).  Подсказки и подсветка редактора строятся по этому
// списку, поэтому он должен меняться вместе с движком.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "prlib/encoding.h"
#include "prlib/keywords.h"

namespace {

//: Ожидаемое имя: его арности, «принимает любое число аргументов» и «можно
//: назвать целью».
struct Expected {
  const char *name;
  std::vector<size_t> arities;
  bool variadic = false;
  bool callable = true;
};

const Expected kExpected[] = {
    {"!", {0}},
    {"ABS", {2}},
    {"COS", {2}},
    {"EXP", {2}},
    {"SIN", {2}},
    {"SQRT", {2}},
    {"TAN", {2}},
    {"div", {}, false, false},
    {"float", {1}},
    {"int", {1}},
    {"mod", {}, false, false},
    {"БОЛЬШЕ", {2}},
    {"БУКВА", {2}},
    {"ВВКОД", {1}},
    {"ВВОДВЕЩ", {1, 2}},
    {"ВВОДСИМВ", {1, 2}},
    {"ВВОДСТР", {1, 2}},
    {"ВВОДЦЕЛ", {1, 2}},
    {"ВЕЩ", {1}},
    {"ВЕРСИЯ", {0}},
    {"ВЫВОД", {}, true},
    {"ВЫП", {1}},
    {"ДЛИНА", {2}},
    {"ДОБ", {3}},
    {"ЖДИ", {1}},
    {"ЗАКРАСКА", {3}},
    {"ЗАПИСЬ_В", {1}},
    {"ИЛИ", {2}},
    {"КОПИЯ", {4}},
    {"ЛИНИЯ", {5}},
    {"ЛОЖЬ", {0}},
    {"МЕНЬШЕ", {2}},
    {"НАЙТИВСЕ", {3}},
    {"НЕ", {1}},
    {"НЕТ_ТРАССЫ", {0}},
    {"ОКРУЖНОСТЬ", {4}},
    {"ПЕР", {1}},
    {"ПРЕДЛ", {4}},
    {"ПС", {0}},
    {"РАВНО", {2}},
    {"СИМВ", {1}},
    {"СКОЛЬКО", {2}},
    {"СЛОЖЕНИЕ", {3}},
    {"СЛУЧ", {1, 3}},
    {"СПИСОК", {1}},
    {"СТРВЕЩ", {2}},
    {"СТРСПИС", {2}},
    {"СТРЦЕЛ", {2}},
    {"СЦЕП", {3}},
    {"ТЕРМ", {2}},
    {"ТИХО", {0}},
    {"ТОЧКА", {3}},
    {"ТРАССА", {0}},
    {"УДАЛЕНИЕ", {2}},
    {"УМНОЖЕНИЕ", {3, 4}},
    {"ЦЕЛ", {1}},
    {"ЦЕЛВЕЩ", {2}},
    {"ЦИФРА", {2}},
    {"ЧТЕНИЕ_ИЗ", {1}},
};

//: Имя движка в UTF-8 — так его видит интерфейс.
std::string utf8(const prologd::ReservedName &name) {
  return prologd::cp1251_to_utf8(name.name);
}

const prologd::ReservedName *find(const std::vector<prologd::ReservedName> &names,
                                  const std::string &name) {
  for (const prologd::ReservedName &candidate : names) {
    if (utf8(candidate) == name) {
      return &candidate;
    }
  }
  return nullptr;
}

//: Строки имени, как их печатает `--list-keywords`, в одну строку.
std::string shown(const prologd::ReservedName &name) {
  std::string text;
  for (const std::string &label : name.labels()) {
    if (!text.empty()) {
      text += ' ';
    }
    text += prologd::cp1251_to_utf8(label);
  }
  return text;
}

}  // namespace

TEST_CASE("the engine lists every name it accepts, with its arities", "[keywords]") {
  const std::vector<prologd::ReservedName> names = prologd::reserved_names();
  REQUIRE(names.size() == sizeof(kExpected) / sizeof(Expected));
  for (const Expected &expected : kExpected) {
    INFO("имя: " << expected.name);
    const prologd::ReservedName *name = find(names, expected.name);
    REQUIRE(name != nullptr);
    REQUIRE(name->arities == expected.arities);
    REQUIRE(name->variadic == expected.variadic);
    REQUIRE(name->callable == expected.callable);
  }
}

TEST_CASE("the names are sorted and none is listed twice", "[keywords]") {
  const std::vector<prologd::ReservedName> names = prologd::reserved_names();
  REQUIRE(!names.empty());
  for (size_t i = 1; i < names.size(); ++i) {
    REQUIRE(names[i - 1].name < names[i].name);
  }
}

TEST_CASE("a name is shown as name/arity", "[keywords]") {
  const std::vector<prologd::ReservedName> names = prologd::reserved_names();
  // Two arities are two lines, an arity the engine does not check is `/+`, and a
  // name that is neither is a name on its own.
  REQUIRE(shown(*find(names, "УМНОЖЕНИЕ")) == "УМНОЖЕНИЕ/3 УМНОЖЕНИЕ/4");
  REQUIRE(shown(*find(names, "СЛУЧ")) == "СЛУЧ/1 СЛУЧ/3");
  REQUIRE(shown(*find(names, "ЛИНИЯ")) == "ЛИНИЯ/5");
  REQUIRE(shown(*find(names, "ЛОЖЬ")) == "ЛОЖЬ/0");
  REQUIRE(shown(*find(names, "ВЫВОД")) == "ВЫВОД/+");
  REQUIRE(shown(*find(names, "mod")) == "mod");
}
