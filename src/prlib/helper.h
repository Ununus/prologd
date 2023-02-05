#ifndef HELPER_H
#define HELPER_H

#include <utility>
#include <string>
#include <sstream>

#define USE_OLD_COMPILER 1

namespace hlp {

#if USE_OLD_COMPILER
  static inline bool from_chars(const char *fst, const char *lst, IntegerType &val) {
    if (fst == lst || *fst == '\0') {
      return false;
    }
    val = 0;
    IntegerType neg = 1;
    if (*fst == '+') {
      ++fst;
    } else if (*fst == '-') {
      neg = -1;
      ++fst;
    }
    if (fst == lst || *fst == '\0') {
      return false;
    }
    for (const char *p = fst; *p != '\0' && p != lst; ++p) {
      if (*p < '0' || *p > '9') {
        return false;  // error
      }
      val *= 10;
      val += static_cast<IntegerType>(*p - '0');
    }
    val *= neg;
    return true;  // success
  }
  static inline bool from_chars(const char *fst, const char *lst, FloatType &val) {
    if (fst == lst || *fst == '\0') {
      return false;
    }
    val = 0.f;
    FloatType neg = 1.f;
    if (*fst == '+') {
      ++fst;
    } else if (*fst == '-') {
      neg = -1.f;
      ++fst;
    }
    if (fst == lst || *fst == '\0') {
      return false;
    }
    const char *p = fst;
    for (; *p != '.' && *p != 'e' && p != lst; ++p) {
      if (*p == '\0') {
        val *= neg;
        return true;  // success
      }
      if (*p < '0' || *p > '9') {
        return false;  // error
      }
      val *= 10.f;
      val += static_cast<FloatType>(*p - '0');
    }
    if (p == lst) {
      val *= neg;
      return true;
    }
    if (*p == '.') {
      ++p;
      for (FloatType dv = 0.1f; *p != 'e' && p != lst; dv /= 10.f, ++p) {
        if (*p == '\0') {
          val *= neg;
          return true;  // success
        }
        if (*p < '0' || *p > '9') {
          return false;  // error
        }
        val += static_cast<FloatType>(*p - '0') * dv;
      }
    }
    if (p == lst) {
      val *= neg;
      return true;
    }
    if (*p == 'e') {
      ++p;
      if (*p == '\0' || p == lst) {
        return false;
      }
      FloatType m = 10.f;
      if (*p == '+') {
        ++p;
      } else if (*p == '-') {
        m = 0.1f;
        ++p;
      }
      if (*p == '\0' || p == lst) {
        return false;
      }
      int mult = 0;
      for (; *p != '\0' && p != lst; ++p) {
        if (*p < '0' || *p > '9') {
          return false;  // error
        }
        mult *= 10;
        mult += static_cast<int>(*p - '0');
      }
      for (int i = 0; i < mult; ++i) {
        val *= m;
      }
    }
    val *= neg;
    return true;  // success
  }
  template<class Tp>
  const char *to_chars(char *fst, const char *lst, Tp val) {
    std::stringstream s;
    s << val;
    std::string st = s.str();
    for (size_t i = 0; i < st.size(); ++i) {
      *fst = st[i];
      ++fst;
      if (fst == lst) {
        break;
      }
    }
    return fst;
  }
#endif
}  // namespace hlp

#endif  // HELPER_H
