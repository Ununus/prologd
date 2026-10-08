#include "encoding.h"

#include <stdexcept>

namespace prologd {
namespace {

//: CP1251 bytes of the two letters that stand outside the А…я block.
constexpr unsigned char kYo = 0xB8;   // ё -> U+0451
constexpr unsigned char kYoUpper = 0xA8;  // Ё -> U+0401
//: CP1251's А starts here; А…я is one contiguous block that maps onto U+0410…
constexpr unsigned char kCapitalA = 0xC0;
constexpr unsigned kLetterA = 1040u;  // U+0410

}  // namespace

std::string cp1251_to_utf8(std::string_view text) {
  std::string result;
  result.reserve(text.size());
  for (const char symbol : text) {
    const unsigned char byte = static_cast<unsigned char>(symbol);
    if (byte == kYo) {
      result.push_back(static_cast<char>(0xD1));
      result.push_back(static_cast<char>(0x91));
    } else if (byte == kYoUpper) {
      result.push_back(static_cast<char>(0xD0));
      result.push_back(static_cast<char>(0x81));
    } else if (byte >= kCapitalA) {
      const unsigned code = kLetterA + (byte - kCapitalA);
      result.push_back(static_cast<char>(0xC0u + (code >> 6u)));
      result.push_back(static_cast<char>(0x80u + (code & 63u)));
    } else {
      result.push_back(static_cast<char>(byte & 0x7Fu));
    }
  }
  return result;
}

std::string utf8_to_cp1251(std::string_view text) {
  std::string result;
  result.reserve(text.size());
  for (std::size_t index = 0; index < text.size();) {
    const unsigned char first = static_cast<unsigned char>(text[index]);
    const unsigned char second =
        index + 1 < text.size() ? static_cast<unsigned char>(text[index + 1]) : 0u;
    if (index + 1 < text.size() && first == 0xD0 && second == 0x81) {
      result.push_back(static_cast<char>(kYoUpper));
      index += 2;
    } else if (index + 1 < text.size() && first == 0xD1 && second == 0x91) {
      result.push_back(static_cast<char>(kYo));
      index += 2;
    } else if ((first & 0x80u) == 0) {
      result.push_back(static_cast<char>(first));
      ++index;
    } else if ((first & 0xE0u) == 0xC0u && index + 1 < text.size()) {
      const unsigned code = ((first ^ 0xC0u) << 6u) + (second & 63u) - kLetterA - 64u;
      result.push_back(static_cast<char>(code));
      index += 2;
    } else {
      throw std::runtime_error("Decoding error");
    }
  }
  return result;
}

}  // namespace prologd
