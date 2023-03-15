#include "scaner.h"
#include "functions.h"
#include "pdefs.h"
#include "pstructs.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

// ������� ��������������� ������� ������
// �������� ������� ������ � �������
ErrorCode scaner(char *Str, TScVar *ScVar, array *heap) {
  ErrorCode err = ErrorCode::NoErrors;
  bool &EndOfClause = ScVar->EndOfClause;
  bool &Query = ScVar->Query;
  auto &bptr = ScVar->bptr;
  auto &gptr = ScVar->gptr;
  auto &novar = ScVar->novar;
  bool &right = ScVar->right;
  auto &exprip = ScVar->exprip;
  EndOfClause = false;
  char *p = Str;
  while (*p && err == ErrorCode::NoErrors)  // ��������� 0 ��� ��� ������ � �����������
  {
    if (ScVar->vbuf.size() < bptr + 256) {
      ScVar->vbuf.resize(std::max(ScVar->vbuf.size() * 2, bptr + 256));
    }
    if (ScVar->vgoal.size() < gptr + 16) {
      ScVar->vgoal.resize(std::max(ScVar->vgoal.size() * 2, gptr + 16));
    }
    auto &buf = ScVar->vbuf;
    auto &goal = ScVar->vgoal;
    switch (*p) {
    case ',': buf[bptr++] = iscomma; break;  // ������ ������� � buf
    case '|': buf[bptr++] = isstick; break;
    case '(': {
      if (!exprip) {
        goal[gptr++] = bptr;
      }
      buf[bptr++] = isbbeg;
    } break;  // ����� �������
    case ')': {
      buf[bptr++] = isbend;
      if (!exprip) {
        err = tokbb(ScVar, heap);
      }
    } break;
    case ':':
    case '<': err = implic(p, ScVar, heap); break;
    case '%':
      while (*p != '\n' && *p) {
        p++;
      }
      break;
    case ';':
    case '.': {
      buf[bptr++] = isend;
      err = clause(ScVar, heap);
      right = false;
      bptr = gptr = exprip = novar = 0;
      if (Query) {
        if (*p) {
          p++;
        }
        return err;
      }  // ��� ������ ��� ������������� tfileedit
    } break;
    case '#': {
      // ����� ���������
      err = expbeg(ScVar, heap);
      break;
    }
    case '[': {
      // ������
      err = lbeg(p, ScVar, heap);
      break;
    }
    case ']':
      buf[bptr++] = islend;
      err = list(ScVar, heap);
      break;
    case '\"': {
      // ������ ��������
      err = strings(p, ScVar, heap);
      break;
    }
    case '?': {
      // ������
      if (bptr || right) {
        err = ErrorCode::IncorrectUseQuestion;  // 10;
      } else {
        Query = right = true;
      }
    } break;
    case '\n':
    case '\r':
    case ' ': break;  // �� ����� �������� - ������� �� ��������� ������
    case '_': {
      // ��������
      if (gptr <= 0) {
        err = ErrorCode::IncorrectUseUnderscore;  // 11;
      } else {
        if (bptr + 1 >= buf.size()) {
          buf.resize(std::max(buf.size() * 2, bptr + 2));
        }
        buf[bptr++] = ScVar->hpunkn;
      }
    } break;
    case '!':
      if (!right) {
        err = ErrorCode::IncorrectUseExclamation;  // 12;
      } else {
        if (bptr + 1 >= buf.size()) {
          buf.resize(std::max(buf.size() * 2, bptr + 2));
        }
        buf[bptr++] = hpcut;
      }
      break;
    case '+': {
      err = arsgn(ScVar, isplus);
    } break;
    case '-': {
      if (exprip && bptr &&
          (buf[bptr - 1] != islbeg && buf[bptr - 1] != isexpress && buf[bptr - 1] != isbbeg && buf[bptr - 1] != ismult && buf[bptr - 1] != isslash)) {
        err = arsgn(ScVar, isminus);
      } else {
        err = num(p, ScVar, heap);
      }
    } break;
    case '*': {
      err = arsgn(ScVar, ismult);
      break;
    }
    case '/': {
      err = arsgn(ScVar, isslash);
      break;
    }
      // ������� �������������� ������� ������ ����������� � expresse
    default: {
      size_t len = 0;
      if (issvar(p, len)) {
        err = variabletable(p, len, ScVar, heap);
        p += len - 1;
      } else {
        if (issconst(p, len)) {
          err = wrsconst(p, len, ScVar, heap);
          p += len - 1;
        } else {  // �����
          if ((*p >= '0' && *p <= '9') || *p == '-' || *p == '+') {
            err = num(p, ScVar, heap);
          } else {
            err = ErrorCode::InvalidCharacter;  // 1 // ������ �� �������� ����������
          }
        }
      }
    }
    }
    if (*p) {
      p++;  // ��� ������ �� ����� ��������� lptr ������ ���������
    }       // �� ��������� ������������ ������
  }
  if (err != ErrorCode::NoErrors) {
    std::string Mess = "������ ��� �������: " + std::string(Str);
    errout(Mess.c_str());
  }
  return err;
}
