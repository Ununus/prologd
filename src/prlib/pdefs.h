#pragma once
#include <type_traits>
#include <cstddef>
#include <limits>

template<typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
  return static_cast<typename std::underlying_type<E>::type>(e);
}

// �������������� �������� ���������
constexpr size_t isvar = 0;
constexpr size_t issymbol = 1;
constexpr size_t isinteger = 2;
constexpr size_t isfloat = 3;
constexpr size_t isexpression = 4;
constexpr size_t isunknown = 5;
constexpr size_t isemptylist = 6;
constexpr size_t islist = 7;
constexpr size_t isfunction = 8;
constexpr size_t isstring = 9;
constexpr size_t isclause = 10;
constexpr size_t isclauseq = 11;
constexpr size_t iscut = 12;

constexpr size_t maxarity = 5;  // ���� max ������� ����������� ���������

// ��������� ��� ������ � ������ buf
constexpr size_t isnil = std::numeric_limits<size_t>::max();  // �� ������ ������
constexpr size_t isbase = isnil - 17;                         //
constexpr size_t iscomma = isnil - 16;                        //     ,
constexpr size_t isstick = isnil - 15;                        //     |
constexpr size_t isbbeg = isnil - 14;                         //     (
constexpr size_t isbend = isnil - 13;                         //     )
constexpr size_t isimpl = isnil - 12;                         // :-  <-
constexpr size_t isend = isnil - 11;                          // . ;
constexpr size_t isexpress = isnil - 10;                      // #
constexpr size_t islbeg = isnil - 9;                          // [
constexpr size_t islend = isnil - 8;                          // ]
constexpr size_t isunitminus = isnil - 7;                     // -
constexpr size_t isminus = isnil - 6;                         // -
constexpr size_t isslash = isnil - 5;                         // /
constexpr size_t ismult = isnil - 4;                          // *
constexpr size_t isplus = isnil - 3;                          // +
constexpr size_t ismod = isnil - 2;                           // mod
constexpr size_t isdiv = isnil - 1;                           // div

constexpr size_t maxgrx = 1024;  // max ���������� ��� ��������� ������� X
constexpr size_t maxgry = 768;   //                                     Y
