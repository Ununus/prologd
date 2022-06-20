#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "../PrologD.h"
#include <QSyntaxHighlighter>
#include <QRegularExpression>

class Highlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  explicit Highlighter(QTextDocument *parent = 0);
  void setSearchString(const QString &string);

protected:
  void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> m_highlighting_rules;
  QTextCharFormat m_keyword_format;
  QTextCharFormat m_digits_format;
  QTextCharFormat m_upper_english_words;
  QTextCharFormat m_qustion_format;
  QTextCharFormat m_single_line_comment_format;
  QTextCharFormat m_search_string_format;
  QString m_search_string;
};

#endif  // HIGHLIGHTER_H
