// A simple Lexer meant to demonstrate a few theoretical concepts. It can
// support several parser concepts and is very fast (though speed is not its
// design goal).
//
// J. Arrieta
// (C) 2018 Nabla Zero Labs

#include <cassert>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include "utils/json11.hpp"
#include <list>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <type_traits>


class Token {
 public:
  enum class Kind {
    Number,
    Identifier,
    LeftParen,
    RightParen,
    LeftSquare,
    RightSquare,
    LeftCurly,
    RightCurly,
    LessThan,
    GreaterThan,
    Equal,
    Plus,
    Minus,
    Asterisk,
    Slash,
    Hash,
    Dot,
    Comma,
    Colon,
    Semicolon,
    SingleQuote,
    DoubleQuote,
    Comment,
    Pipe,
    End,
    Unexpected,
    Space,
    NewLine,
  };

  Token(Kind kind) noexcept : m_kind{kind} {}

  Token(Kind kind, const char* beg, std::size_t len) noexcept
      : m_kind{kind}, m_lexeme(beg, len) {}

  Token(Kind kind, const char* beg, const char* end) noexcept
      : m_kind{kind}, m_lexeme(beg, std::distance(beg, end)) {}

  Kind kind() const noexcept { return m_kind; }

  void kind(Kind kind) noexcept { m_kind = kind; }

  bool is(Kind kind) const noexcept { return m_kind == kind; }

  bool is_not(Kind kind) const noexcept { return m_kind != kind; }

  bool is_one_of(Kind k1, Kind k2) const noexcept { return is(k1) || is(k2); }

  template <typename... Ts>
  bool is_one_of(Kind k1, Kind k2, Ts... ks) const noexcept {
    return is(k1) || is_one_of(k2, ks...);
  }

  std::string lexeme() const noexcept { return m_lexeme; }

  void lexeme(std::string lexeme) noexcept {
    m_lexeme = std::move(lexeme);
  }

 private:
  Kind             m_kind{};
  std::string m_lexeme{};
};

class Lexer {
 public:
  Lexer(const char* beg) noexcept : m_beg{beg} {}

  Token next() noexcept;

 private:
  Token identifier() noexcept;
  Token number() noexcept;
  Token slash_or_comment() noexcept;
  Token atom(Token::Kind) noexcept;

  char peek() const noexcept { return *m_beg; }
  char get() noexcept { return *m_beg++; }

  const char* m_beg = nullptr;
};

bool is_space(char c) noexcept {
  switch (c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      return true;
    default:
      return false;
  }
}

bool is_digit(char c) noexcept {
  switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return true;
    default:
      return false;
  }
}

bool is_identifier_char(char c) noexcept {
  switch (c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '_':
      return true;
    default:
      return false;
  }
}

Token Lexer::atom(Token::Kind kind) noexcept { return Token(kind, m_beg++, 1); }

Token Lexer::next() noexcept {
  // while (is_space(peek())) get();

  switch (peek()) {
    case '\0':
      return Token(Token::Kind::End, m_beg, 1);
    default:
      return atom(Token::Kind::Unexpected);
    case ' ':
      return atom(Token::Kind::Space);
    case '\n':
      return atom(Token::Kind::NewLine);
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
      return identifier();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return number();
    case '(':
      return atom(Token::Kind::LeftParen);
    case ')':
      return atom(Token::Kind::RightParen);
    case '[':
      return atom(Token::Kind::LeftSquare);
    case ']':
      return atom(Token::Kind::RightSquare);
    case '{':
      return atom(Token::Kind::LeftCurly);
    case '}':
      return atom(Token::Kind::RightCurly);
    case '<':
      return atom(Token::Kind::LessThan);
    case '>':
      return atom(Token::Kind::GreaterThan);
    case '=':
      return atom(Token::Kind::Equal);
    case '+':
      return atom(Token::Kind::Plus);
    case '-':
      return atom(Token::Kind::Minus);
    case '*':
      return atom(Token::Kind::Asterisk);
    case '/':
      return slash_or_comment();
    case '#':
      return atom(Token::Kind::Hash);
    case '.':
      return atom(Token::Kind::Dot);
    case ',':
      return atom(Token::Kind::Comma);
    case ':':
      return atom(Token::Kind::Colon);
    case ';':
      return atom(Token::Kind::Semicolon);
    case '\'':
      return atom(Token::Kind::SingleQuote);
    case '"':
      return atom(Token::Kind::DoubleQuote);
    case '|':
      return atom(Token::Kind::Pipe);
  }
}

Token Lexer::identifier() noexcept {
  const char* start = m_beg;
  get();
  while (is_identifier_char(peek())) get();
  return Token(Token::Kind::Identifier, start, m_beg);
}

Token Lexer::number() noexcept {
  const char* start = m_beg;
  get();
  while (is_digit(peek())) get();
  return Token(Token::Kind::Number, start, m_beg);
}

Token Lexer::slash_or_comment() noexcept {
  const char* start = m_beg;
  get();
  if (peek() == '/') {
    get();
    start = m_beg;
    while (peek() != '\0') {
      if (get() == '\n') {
        return Token(Token::Kind::Comment, start,
                     std::distance(start, m_beg) - 1);
      }
    }
    return Token(Token::Kind::Unexpected, m_beg, 1);
  } else {
    return Token(Token::Kind::Slash, start, 1);
  }
}

#include <iomanip>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Token::Kind& kind) {
  static const char* const names[]{
      "Number",      "Identifier",  "LeftParen",  "RightParen", "LeftSquare",
      "RightSquare", "LeftCurly",   "RightCurly", "LessThan",   "GreaterThan",
      "Equal",       "Plus",        "Minus",      "Asterisk",   "Slash",
      "Hash",        "Dot",         "Comma",      "Colon",      "Semicolon",
      "SingleQuote", "DoubleQuote", "Comment",    "Pipe",       "End",
      "Unexpected",  "Space",       "NewLine"
  };
  return os << names[static_cast<int>(kind)];
}

#include <string>
#include <fstream>
#include <streambuf>
#include <regex>

int main() {
  // std::ifstream t("./syntax.json");
  // std::string str((std::istreambuf_iterator<char>(t)),
  //                 std::istreambuf_iterator<char>());
  
  //   std::string err_comment;
  //   auto json_comment = json11::Json::parse(str.c_str(), err_comment);
  //   auto listC = json_comment["configurations"].array_items();
  //   for(auto iItem : listC)
  //   {
  //     std::cout << " key : " << iItem["key"].string_value() << std::endl;
  //     std::cout << " fg  : " << iItem["fg"].int_value() << std::endl;
  //   }

  //   std::map<std::string, int> nmap;
  //   nmap["x"]  = 10;
  //   std::cout << nmap["x"];

  // std::smatch typeMatch;
  // std::regex typeRegx(R"(class\s([A-Za-z0-9]+))");
  // std::string line = "class nameClass ";
  // if(std::regex_search(line, typeMatch, typeRegx)) {
  //     if (typeMatch.size() > 1) 
  //         std::cout << typeMatch[1].str() << '\n';
  // }

    auto code =
        "void // TextAr"
        "{\n"
        "if(m_cursor.row > 0)\n"
        "{\n"
            "m_cursor.row--;\n"
        "}\n"
        "else if(m_cursor.row == 0)\n"
        "{\n"
            "// if(m_scrollView.pos.row > 0)\n"
            "{\n"
               "m_scrollView.pos.row--;\n"
            "}\n"
        "}\n";

  Lexer lex(code);
  for (auto token = lex.next();
       not token.is_one_of(Token::Kind::End, Token::Kind::Unexpected);
       token = lex.next()) {
    std::cout << std::setw(12) << token.kind() << " |" << token.lexeme()
              << "|\n";
  }
}
