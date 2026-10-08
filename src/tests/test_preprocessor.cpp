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

  //: Пишет файл и отвечает его путём.
  std::filesystem::path file(const std::string &relative, const std::string &text) const {
    const std::filesystem::path path = root_ / relative;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
    return path;
  }

  std::string under(const std::string &relative) const { return (root_ / relative).string(); }
  std::string root() const { return root_.string(); }

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
std::string readProgram(const std::filesystem::path &path) {
  std::stringstream &source = preprocessor_run(path.string());
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
  return !found.empty() && std::filesystem::path(found) == std::filesystem::path(expected);
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
