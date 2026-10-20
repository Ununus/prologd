// ИМПОРТ: откуда движок берёт файл, который называет `?ИМПОРТ("имя")`.
//
// Каталогов четыре, и спрашиваются они по очереди: каталог импортирующего файла,
// каталоги `-I` (и `-d`), каталог самого движка (рядом с ним поставка держит
// `examples` и `stdlib`) и текущий каталог.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "path_utf8.hpp"
#include "preprocessor.h"
#include "test_helpers.hpp"

using prologd_test::run;

namespace {

//: Каталог проб: файлы пишутся в него, а по концу теста он убирается.
class Sandbox {
 public:
  explicit Sandbox(const std::string &name) {
    root_ = std::filesystem::temp_directory_path() / ("prologd-import-" + name);
    std::filesystem::remove_all(root_);
    std::filesystem::create_directories(root_);
  }
  ~Sandbox() { std::filesystem::remove_all(root_); }

  Sandbox(const Sandbox &) = delete;
  Sandbox &operator=(const Sandbox &) = delete;

  //: Пишет файл и отвечает его путём.  Имя — UTF-8, как его называет программа:
  //: в имени может быть кириллица, и в системный путь его переводит path_utf8.
  std::filesystem::path file(const std::string &relative, const std::string &text) const {
    const std::filesystem::path path = root_ / prologd_path::from_utf8(relative);
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
    return path;
  }

  std::string under(const std::string &relative) const {
    return prologd_path::to_utf8(root_ / prologd_path::from_utf8(relative));
  }
  std::string root() const { return prologd_path::to_utf8(root_); }

 private:
  std::filesystem::path root_;
};

//: Каталоги поиска — состояние движка: тест ставит свои и убирает их за собой.
class SearchPaths {
 public:
  explicit SearchPaths(std::vector<std::string> directories = {}, std::string executable = {}) {
    preprocessor_set_import_directories(std::move(directories));
    preprocessor_set_executable_directory(std::move(executable));
  }
  ~SearchPaths() {
    preprocessor_set_import_directories({});
    preprocessor_set_executable_directory({});
  }
};

//: Текст программы, каким его читает движок: строки файла и всё, что он вобрал.
//: Имя файла — UTF-8: так его называет и командная строка, и протокол окна.
std::string readProgram(const std::filesystem::path &path) {
  std::stringstream &source = preprocessor_run(prologd_path::to_utf8(path));
  std::string text;
  std::string line;
  while (std::getline(source, line)) {
    text += line + "\n";
  }
  return text;
}

//: То же для текста программы, названного файлом, — так грузит программу окно.
std::string readSource(const std::string &text, const std::string &source_path) {
  std::list<std::string> lines;
  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)) {
    lines.push_back(line);
  }
  std::string joined;
  for (const std::string &out : preprocessor_run_on_source(lines, source_path)) {
    joined += out + "\n";
  }
  return joined;
}

bool same(const std::string &found, const std::string &expected) {
  return !found.empty() && prologd_path::from_utf8(found) == prologd_path::from_utf8(expected);
}

}  // namespace

TEST_CASE("an import is found beside the program that names it", "[preprocessor]") {
  SearchPaths paths;
  const Sandbox sandbox("beside");
  sandbox.file("lib/library.pld", "удвоить(Ч, Д):-УМНОЖЕНИЕ(Ч, 2, Д).\n");
  const std::filesystem::path program =
      sandbox.file("main.pld", "?ИМПОРТ(\"lib/library.pld\").\n?удвоить(21, Д).\n");
  REQUIRE(run(readProgram(program)).output == "Д=42\n");
}

TEST_CASE("an import is found in a directory of the command line", "[preprocessor]") {
  const Sandbox sandbox("include");
  sandbox.file("lib/library.pld", "привет(мир).\n");
  SearchPaths paths({sandbox.under("lib")});
  REQUIRE(run(readSource("?ИМПОРТ(\"library.pld\").\n?привет(К).\n", "")).output == "К=мир\n");
}

TEST_CASE("an import is found beside the interpreter", "[preprocessor]") {
  const Sandbox sandbox("install");
  sandbox.file("examples/lib/library.pld", "привет(мир).\n");
  SearchPaths paths({}, sandbox.root());
  const std::string text = "?ИМПОРТ(\"examples/lib/library.pld\").\n?привет(К).\n";
  REQUIRE(run(readSource(text, "")).output == "К=мир\n");
}

TEST_CASE("the program's own directory is tried before the others", "[preprocessor]") {
  const Sandbox sandbox("order");
  sandbox.file("lib/library.pld", "привет(свой).\n");
  sandbox.file("other/library.pld", "привет(чужой).\n");
  SearchPaths paths({sandbox.under("other")});
  const std::filesystem::path program =
      sandbox.file("main.pld", "?ИМПОРТ(\"lib/library.pld\").\n?привет(К).\n");
  REQUIRE(run(readProgram(program)).output == "К=свой\n");
}

TEST_CASE("an imported file searches from its own directory", "[preprocessor]") {
  SearchPaths paths;
  const Sandbox sandbox("nested");
  sandbox.file("lib/inner.pld", "внутри(1).\n");
  sandbox.file("lib/library.pld", "?ИМПОРТ(\"inner.pld\").\nснаружи(2).\n");
  const std::filesystem::path program =
      sandbox.file("main.pld", "?ИМПОРТ(\"lib/library.pld\").\n?внутри(х).\n?снаружи(у).\n");
  REQUIRE(run(readProgram(program)).output == "х=1\nу=2\n");
}

TEST_CASE("an absolute import name is taken as it is", "[preprocessor]") {
  SearchPaths paths;
  const Sandbox sandbox("absolute");
  const std::filesystem::path library = sandbox.file("far/library.pld", "привет(мир).\n");
  const std::string text = "?ИМПОРТ(\"" + library.string() + "\").\n?привет(К).\n";
  REQUIRE(run(readSource(text, "")).output == "К=мир\n");
}

TEST_CASE("an import may be written as a directive", "[preprocessor]") {
  // `:- ИМПОРТ("имя").` — запись Пролога-Д 2.x: переносимые программы пишут так.
  SearchPaths paths;
  const Sandbox sandbox("directive");
  sandbox.file("lib/library.pld", "удвоить(Ч, Д):-УМНОЖЕНИЕ(Ч, 2, Д).\n");
  const std::filesystem::path program =
      sandbox.file("main.pld", ":- ИМПОРТ(\"lib/library.pld\").\n?удвоить(21, Д).\n");
  REQUIRE(run(readProgram(program)).output == "Д=42\n");
}

TEST_CASE("a directive allows spaces after the colon and the dash", "[preprocessor]") {
  SearchPaths paths;
  const Sandbox sandbox("spaces");
  sandbox.file("library.pld", "привет(мир).\n");
  REQUIRE(run(readSource(":-   ИМПОРТ(\"library.pld\").\n?привет(К).\n",
                         sandbox.under("main.pld"))).output == "К=мир\n");
  REQUIRE(run(readSource(":-ИМПОРТ(\"library.pld\").\n?привет(К).\n",
                         sandbox.under("main.pld"))).output == "К=мир\n");
  // и вопрос с пробелом — то же самое
  REQUIRE(run(readSource("?   ИМПОРТ(\"library.pld\").\n?привет(К).\n",
                         sandbox.under("main.pld"))).output == "К=мир\n");
}

TEST_CASE("the names directive of another implementation is dropped", "[preprocessor]") {
  // `:- ИМЕНА(N).` задаёт, как читать имена, в Прологе-Д 2.x.  Этот движок
  // читает их только по-старому, и такой строки он не знает: в разбор она не
  // попадает, а остальной текст остаётся как был.
  SearchPaths paths;
  const Sandbox sandbox("names");
  const std::filesystem::path program = sandbox.file(
      "main.pld", ":- ИМЕНА(2).\nцель(1).\n:-ИМЕНА(0).\n?цель(х).\n");
  REQUIRE(readProgram(program) == "цель(1).\n?цель(х).\n");
  REQUIRE(run(readProgram(program)).output == "х=1\n");
}

TEST_CASE("a names directive may stand among imports", "[preprocessor]") {
  SearchPaths paths;
  const Sandbox sandbox("names-import");
  sandbox.file("lib/library.pld", "привет(мир).\n");
  const std::filesystem::path program = sandbox.file(
      "main.pld",
      ":- ИМЕНА(2).\n:- ИМПОРТ(\"lib/library.pld\").\n?привет(К).\n");
  REQUIRE(readProgram(program) == "привет(мир).\n?привет(К).\n");
  REQUIRE(run(readProgram(program)).output == "К=мир\n");
}

TEST_CASE("a missing import is named in the error", "[preprocessor]") {
  SearchPaths paths;
  const Sandbox sandbox("missing");
  const std::filesystem::path program = sandbox.file("main.pld", "?ИМПОРТ(\"lib/none.pld\").\n");
  bool thrown = false;
  try {
    (void)readProgram(program);
  } catch (const std::runtime_error &error) {
    thrown = true;
    REQUIRE(std::string(error.what()) == "Cannot open file: lib/none.pld");
  }
  REQUIRE(thrown);
}

TEST_CASE("an import cannot reach the parent directory", "[preprocessor]") {
  SearchPaths paths;
  const Sandbox sandbox("parent");
  const std::filesystem::path program = sandbox.file("main.pld", "?ИМПОРТ(\"../secret.pld\").\n");
  bool thrown = false;
  try {
    (void)readProgram(program);
  } catch (const std::runtime_error &error) {
    thrown = true;
    REQUIRE(std::string(error.what()) ==
            "An attempt was made to access the parent directory with \"../\".");
  }
  REQUIRE(thrown);
}

TEST_CASE("a Cyrillic name is found and read", "[preprocessor]") {
  // Имя файла приходит в UTF-8 — так его несёт текст программы, — а строка,
  // отданная `std::ifstream`, прочиталась бы в ANSI-кодировке: «библиотека»
  // превратилась бы в «?» и файла не нашлось бы.
  SearchPaths paths;
  const Sandbox sandbox("cyrillic-name");
  sandbox.file("библиотека/книги.pld", "удвоить(Ч, Д):-УМНОЖЕНИЕ(Ч, 2, Д).\n");
  const std::filesystem::path program = sandbox.file(
      "main.pld", "?ИМПОРТ(\"библиотека/книги.pld\").\n?удвоить(21, Д).\n");
  REQUIRE(run(readProgram(program)).output == "Д=42\n");

  // Тот же файл по имени, названному строкой вопроса, и по имени каталога `-I`.
  REQUIRE(run(readSource("?ИМПОРТ(\"библиотека/книги.pld\").\n?удвоить(21, Д).\n",
                         sandbox.under("main.pld"))).output == "Д=42\n");
  SearchPaths include({sandbox.under("библиотека")});
  REQUIRE(run(readSource("?ИМПОРТ(\"книги.pld\").\n?удвоить(21, Д).\n", "")).output == "Д=42\n");
}

TEST_CASE("a Cyrillic path names the program and the directory its imports come from",
          "[preprocessor]") {
  // Путь самой программы тоже бывает кириллическим: тогда от него ищется
  // `?ИМПОРТ`, а он открывается всё тем же путём, а не строкой.
  SearchPaths paths;
  const Sandbox sandbox("cyrillic-path");
  sandbox.file("примеры/книги.pld", "привет(мир).\n");
  const std::filesystem::path program =
      sandbox.file("примеры/программа.pld", "?ИМПОРТ(\"книги.pld\").\n?привет(К).\n");
  REQUIRE(run(readProgram(program)).output == "К=мир\n");

  // Окно грузит текст и называет его путём — кириллица и здесь.
  const std::string text = "?ИМПОРТ(\"книги.pld\").\n?привет(К).\n";
  REQUIRE(run(readSource(text, sandbox.under("примеры/программа.pld"))).output == "К=мир\n");

  // Найденное имя — UTF-8, и это тот же файл, каким его назвали.
  REQUIRE(same(preprocessor_find_import("книги.pld", sandbox.under("примеры")),
               sandbox.under("примеры/книги.pld")));
}

TEST_CASE("the search asks the file, then -I, then the interpreter", "[preprocessor]") {
  const Sandbox sandbox("find");
  sandbox.file("byfile/library.pld", "");
  sandbox.file("byinclude/library.pld", "");
  sandbox.file("examples/lib/library.pld", "");  // это и есть каталог движка
  SearchPaths paths({sandbox.under("byinclude")}, sandbox.root());

  REQUIRE(same(preprocessor_find_import("library.pld", sandbox.under("byfile")),
               sandbox.under("byfile/library.pld")));
  REQUIRE(same(preprocessor_find_import("library.pld", sandbox.under("nowhere")),
               sandbox.under("byinclude/library.pld")));
  REQUIRE(same(preprocessor_find_import("examples/lib/library.pld", ""),
               sandbox.under("examples/lib/library.pld")));
  // Имени нет нигде: поиск обошёл все каталоги и ничего не нашёл.
  REQUIRE(preprocessor_find_import("no/such.pld", sandbox.under("byfile")).empty());
  REQUIRE(preprocessor_find_import("no/such.pld", "").empty());
}
