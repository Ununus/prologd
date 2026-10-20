#include "path_utf8.hpp"

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace prologd_path {
namespace {

#ifdef _WIN32

//: The bytes of one code page as wide characters, or empty when they are not of
//: that code page at all (`MB_ERR_INVALID_CHARS` is what makes the difference
//: between "these bytes are not UTF-8" and "these bytes are some other text"
//: answerable).
std::wstring widen(const std::string &text, UINT code_page) {
  if (text.empty()) {
    return std::wstring();
  }
  const DWORD flags = code_page == CP_UTF8 ? MB_ERR_INVALID_CHARS : 0;
  const int size =
      MultiByteToWideChar(code_page, flags, text.data(), static_cast<int>(text.size()), nullptr, 0);
  if (size <= 0) {
    return std::wstring();
  }
  std::wstring wide(static_cast<size_t>(size), L'\0');
  const int written = MultiByteToWideChar(code_page, flags, text.data(), static_cast<int>(text.size()),
                                          wide.data(), size);
  if (written != size) {
    return std::wstring();
  }
  return wide;
}

#endif

}  // namespace

std::filesystem::path from_utf8(const std::string &name) {
#ifdef _WIN32
  std::wstring wide = widen(name, CP_UTF8);
  if (wide.empty()) {
    wide = widen(name, CP_ACP);
  }
  return std::filesystem::path(wide);
#else
  // On every other system the narrow encoding of the system *is* UTF-8.
  return std::filesystem::path(name);
#endif
}

std::string to_utf8(const std::filesystem::path &path) {
#ifdef _WIN32
  const std::wstring &wide = path.native();
  if (wide.empty()) {
    return std::string();
  }
  const int size =
      WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
  if (size <= 0) {
    return std::string();
  }
  std::string bytes(static_cast<size_t>(size), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), bytes.data(), size, nullptr, nullptr);
  return bytes;
#else
  return path.string();
#endif
}

}  // namespace prologd_path
