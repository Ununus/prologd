// A path from outside - a command line argument, the name an import line gives,
// the `path` of a protocol request - is UTF-8.  A path as Windows wants it is
// UTF-16, and a narrow string handed to `std::ifstream` or to
// `std::filesystem::path` is read in the *ANSI* code page instead: a Cyrillic
// name then turns into `?` and the file is never found.  These two functions
// are the only place in the interpreter that knows about that difference.
#pragma once

#include <filesystem>
#include <string>

namespace prologd_path {

//: The path a UTF-8 name means.  A name that is not valid UTF-8 is read in the
//: ANSI code page instead - that is what a command line of an older build (and
//: the API of this one before the names were UTF-8) carries.
std::filesystem::path from_utf8(const std::string &name);

//: The same path as UTF-8: what a message shows, what the protocol sends and
//: what `preprocessor_find_import` answers.
std::string to_utf8(const std::filesystem::path &path);

}  // namespace prologd_path
