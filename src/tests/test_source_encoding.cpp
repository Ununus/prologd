// The encoding of the sources themselves.  The engine is compiled without
// `/utf-8` and reads its own text in the ANSI code page, so a CP1251 source is
// the only shape in which a Russian message reaches the binary intact.  A file
// that was re-saved as UTF-8 therefore does not merely look wrong: every letter
// of it becomes `п?=п?=…` on the console, and if the re-save went through a
// decoder with replacement, the letters are gone for good and the commit cannot
// be repaired from itself.
//
// That happened once (extfunc.cpp, findall.cpp, forall.cpp, termcopy.h), which
// is what this test is for.  It walks src/ and asks three questions of every
// source file:
//
//   * is there a U+FFFD in it?  Nothing legitimate has one: it is the mark of a
//     damaged file, whatever the encoding was meant to be;
//   * a file meant to be UTF-8 must decode as UTF-8;
//   * a file meant to be CP1251 must not decode as UTF-8 — a CP1251 file with
//     Russian text in it never does, so "it does" means it was re-saved.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

//: The checkout this binary was built from (CMakeLists.txt passes it in).
const fs::path sourceDirectory = PROLOGD_SOURCE_DIR;

//: U+FFFD, the replacement character: what a decoder writes where it could not
//: read a byte, and what no source file of this project may contain.
const std::string replacementCharacter = "\xEF\xBF\xBD";

//: A byte-order mark, which a CP1251 file must not start with either.
const std::string byteOrderMark = "\xEF\xBB\xBF";

//: The files that are read as UTF-8: the front-end protocol and everything that
//: speaks to it, the preprocessor (its messages are UTF-8), the conversions
//: themselves, and the tests and examples, which are written in the words of the
//: dialect.  Everything else with a Russian letter in it is CP1251.
bool isUtf8(const std::string &relative) {
  static const std::set<std::string> names = {
      "preprocessor.cpp", "preprocessor.h", "path_utf8.cpp", "path_utf8.hpp",
      "protocol.cpp", "protocol.hpp", "host.hpp",
      "prlib/encoding.cpp", "prlib/encoding.h", "prlib/keywords.h",
  };
  if (names.count(relative) != 0) {
    return true;
  }
  return relative.rfind("tests/", 0) == 0 || relative.rfind("examples/", 0) == 0;
}

std::string read(const fs::path &path) {
  std::ifstream in(path, std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

bool hasNonAscii(const std::string &bytes) {
  for (const char byte : bytes) {
    if (static_cast<unsigned char>(byte) > 127) {
      return true;
    }
  }
  return false;
}

//: A strict UTF-8 decoder: no overlong forms, no surrogates, nothing above the
//: last code point.
bool isValidUtf8(const std::string &bytes) {
  size_t i = 0;
  while (i < bytes.size()) {
    const auto lead = static_cast<unsigned char>(bytes[i]);
    if (lead < 0x80) {
      ++i;
      continue;
    }
    size_t length = 0;
    unsigned int code = 0;
    if ((lead & 0xE0) == 0xC0) {
      length = 2;
      code = lead & 0x1Fu;
    } else if ((lead & 0xF0) == 0xE0) {
      length = 3;
      code = lead & 0x0Fu;
    } else if ((lead & 0xF8) == 0xF0) {
      length = 4;
      code = lead & 0x07u;
    } else {
      return false;  // a continuation byte or 0xF8..0xFF where a lead belongs
    }
    if (i + length > bytes.size()) {
      return false;
    }
    for (size_t k = 1; k < length; ++k) {
      const auto next = static_cast<unsigned char>(bytes[i + k]);
      if ((next & 0xC0) != 0x80) {
        return false;
      }
      code = (code << 6) | (next & 0x3Fu);
    }
    if ((length == 2 && code < 0x80) || (length == 3 && code < 0x800) ||
        (length == 4 && code < 0x10000) || code > 0x10FFFF ||
        (code >= 0xD800 && code <= 0xDFFF)) {
      return false;
    }
    i += length;
  }
  return true;
}

//: The sources of the interpreter: prlib, the console, the preprocessor and the
//: tests, plus the example programs.  The third-party folder is a build artifact
//: and the CMakeLists files are read by CMake, which always expects UTF-8.
bool isSourceFile(const fs::path &path) {
  const std::string extension = path.extension().string();
  return extension == ".cpp" || extension == ".h" || extension == ".hpp" || extension == ".pld";
}

std::vector<fs::path> sourceFiles() {
  std::vector<fs::path> files;
  const fs::path root = sourceDirectory / "src";
  std::error_code error;
  for (const auto &entry : fs::recursive_directory_iterator(root, error)) {
    if (!entry.is_regular_file(error) || !isSourceFile(entry.path())) {
      continue;
    }
    const fs::path relative = fs::relative(entry.path(), root, error);
    if (relative.empty() || relative.string().rfind("3rdparty", 0) == 0) {
      continue;
    }
    files.push_back(relative);
  }
  return files;
}

bool startsWith(const std::string &text, const std::string &prefix) {
  return text.rfind(prefix, 0) == 0;
}

}  // namespace

TEST_CASE("the sources are in the encoding the compiler reads", "[encoding]") {
  const std::vector<fs::path> files = sourceFiles();
  REQUIRE(files.size() > 20);  // a wrong directory would otherwise pass silently

  size_t cp1251Files = 0;
  size_t utf8Files = 0;
  for (const fs::path &relative : files) {
    INFO(relative.generic_string());
    const std::string bytes = read(sourceDirectory / "src" / relative);
    const bool utf8 = isUtf8(relative.generic_string());
    CHECK(bytes.find(replacementCharacter) == std::string::npos);
    if (utf8) {
      ++utf8Files;
      CHECK(isValidUtf8(bytes));
      continue;
    }
    CHECK_FALSE(startsWith(bytes, byteOrderMark));
    if (!hasNonAscii(bytes)) {
      continue;  // ASCII reads the same either way
    }
    ++cp1251Files;
    CHECK_FALSE(isValidUtf8(bytes));
  }

  // Both halves of the rule are exercised by the tree as it stands.
  CHECK(cp1251Files > 5);
  CHECK(utf8Files > 5);
}
