#include "Highlighter.h"

#include <QDebug>

const char *KeyNames[] = { "mod",        "ЛОЖЬ",     "ТРАССА", "НЕТ_ТРАССЫ", "!",        "ВЫП",     "ВВОДСИМВ", "ВВОДЦЕЛ", "ВВКОД",  "ЧТЕНИЕ_ИЗ",
                           "ЗАПИСЬ_В",   "ПЕР",      "ЦЕЛ",    "ВЕЩ",        "СИМВ",     "ВЫВОД",   "БОЛЬШЕ",   "СТРЦЕЛ",  "СТРВЕЩ", "СТРСПИС",
                           "ЦЕЛВЕЩ",     "БУКВА",    "ЦИФРА",  "ТЕРМ",       "УДАЛЕНИЕ", "СКОЛЬКО", "ТОЧКА",    "СЦЕП",    "ДОБ",    "УМНОЖЕНИЕ",
                           "ОКРУЖНОСТЬ", "ЗАКРАСКА", "КОПИЯ",  "ПРЕДЛ",      "ЛИНИЯ",    "СЛУЧ",    "СЛОЖЕНИЕ", "ЖДИ",     "div",    "int",
                           "float",      "НЕ",       "ИЛИ",    "РАВНО",      "МЕНЬШЕ",   "ПС",      "ДЛИНА",    "ВВОДВЕЩ" };
const int NamesCnt = sizeof(KeyNames) / sizeof(char *);  // Также есть в MainWindow.cpp

Highlighter::Highlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent) {
  HighlightingRule rule;

  QColor colorEnglishWords(128, 0, 0);  // #800000
  m_upper_english_words.setForeground(colorEnglishWords);
  rule.pattern = QRegularExpression("[a-zA-Z]+");
  rule.format = m_upper_english_words;
  m_highlighting_rules.append(rule);

  m_keyword_format.setForeground(Qt::darkBlue);
  // keywordFormat.setFontWeight(QFont::Bold);
  for (int i = 0; i < NamesCnt; ++i) {
    QString pattern = QString(KeyNames[i]);
    rule.pattern = QRegularExpression(pattern);
    rule.format = m_keyword_format;
    m_highlighting_rules.append(rule);
  }
  QColor colorDigitsFormat(80, 80, 0);
  m_digits_format.setForeground(colorDigitsFormat);
  rule.pattern = QRegularExpression("[0-9]+");  // \\b[0-9]+\\b
  rule.format = m_digits_format;
  m_highlighting_rules.append(rule);

  QColor colorQustionFormat(10, 34, 214);  // ##0a22D6
  m_qustion_format.setForeground(colorQustionFormat);
  rule.pattern = QRegularExpression("^\\s*\\?[^\n]*");
  rule.format = m_qustion_format;
  m_highlighting_rules.append(rule);

  m_single_line_comment_format.setForeground(Qt::darkGreen);
  rule.pattern = QRegularExpression("%[^\n]*");
  rule.format = m_single_line_comment_format;
  m_highlighting_rules.append(rule);

  m_search_string_format.setBackground(Qt::yellow);
}
void Highlighter::highlightBlock(const QString &text) {
  foreach (const HighlightingRule &rule, m_highlighting_rules) {
    const QRegularExpression &expression = rule.pattern;
    auto it = expression.globalMatch(text);
    while (it.hasNext()) {
      auto match = it.next();
      int index = match.capturedStart();
      if (index >= 0) {
        int length = match.capturedLength();
        setFormat(index, length, rule.format);
      }
    }
  }
  if (!m_search_string.isEmpty()) {
    QRegularExpression expression(m_search_string);
    // QRegularExpression::PatternOptions opt;
    // expression.setPatternOptions()
    auto it = expression.globalMatch(text);
    while (it.hasNext()) {
      auto match = it.next();
      int index = match.capturedStart();
      if (index >= 0) {
        for (int i = index; i < index + match.capturedLength(); i++) {
          QTextCharFormat tmp = format(i);
          tmp.merge(m_search_string_format);
          setFormat(i, 1, tmp);
        }
      }
    }
  }
}
void Highlighter::setSearchString(const QString &string) {
  m_search_string = string;
  rehighlight();
}
