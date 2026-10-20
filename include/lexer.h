#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <iostream>
#include <vector>

enum class TokenKind {
  INVALID,
  GLOBL,
  SECTION,
  VREG,
  IDENT,
  COMMA,
  COLON,
  LPAREN,
  RPAREN,
  IMMEDIATE,
};

class Token {
private:
  TokenKind m_kind;
  std::string m_lexeme;

public:
  Token( TokenKind kind = TokenKind::INVALID, const std::string &lexeme = "" );
  Token( const Token &other );
  ~Token();

  Token &operator=( const Token &rhs );

  TokenKind get_kind() const;
  const std::string &get_lexeme() const;
};

class Lexer {
private:
  std::istream &m_in;
  std::vector< char > m_buf;
  Token m_next;
  bool m_at_eof;

  // No value semantics
  Lexer( const Lexer & ) = delete;
  Lexer &operator=( const Lexer & ) = delete;

public:
  Lexer( std::istream &in );
  ~Lexer();

  bool peek( Token &tok );
  void next( Token &tok );

  // These are called by the generated lexer code

  // Get the next character of input.
  // Returns -1 if EOF has been reached.
  int get();

  // Unget the character most recently returned by get()
  void unget( int c );

private:
  bool fill();
  bool skip_ws_or_comment();
};

#endif // LEXER_H
