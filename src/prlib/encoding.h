#pragma once

#include <string>
#include <string_view>

// The interpreter's own text is CP1251: the dialect's letters are single CP1251
// bytes, the sources are read as CP1251 and every string the engine prints comes
// out that way.  Everything outside it — a front-end on the other side of a pipe,
// a test file written in UTF-8 — is UTF-8.
//
// These two functions are that boundary in one place, so the console, the JSON
// session and the tests convert alike instead of each carrying its own copy.
namespace prologd {

// CP1251 bytes -> UTF-8.
//
// A byte CP1251 gives no letter passes through with its high bit cleared, which
// is what the console has always written for it.
std::string cp1251_to_utf8(std::string_view text);

// UTF-8 -> CP1251.
//
// Throws std::runtime_error when the text is not valid UTF-8 or names a character
// CP1251 has no byte for.
std::string utf8_to_cp1251(std::string_view text);

}  // namespace prologd
