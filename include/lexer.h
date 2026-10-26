//  hlsim - simulate nearly_cc high-level intermediate code
//  Copyright (C) 2026, David H. Hovemeyer <david.hovemeyer@gmail.com>

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <iostream>
#include <vector>
#include "location.h"

enum class TokenKind {
  INVALID,
  GLOBL,
  SECTION,
  STRING,
  VREG,
  IDENT,
  OPCODE, // used for identifiers which are valid mnemonics
  COMMA,
  COLON,
  LPAREN,
  RPAREN,
  IMMEDIATE,
  IMMEDIATE_LABEL,
  STRING_LITERAL,
};

class Token {
private:
  TokenKind m_kind;
  std::string m_lexeme;
  Location m_loc;

public:
  Token( TokenKind kind = TokenKind::INVALID, const Location &loc = Location(), const std::string &lexeme = "" );
  Token( const Token &other );
  ~Token();

  Token &operator=( const Token &rhs );

  TokenKind get_kind() const;
  const std::string &get_lexeme() const;
  const Location &get_loc() const;

  std::string get_string_literal_value() const;
};

class Lexer {
private:
  std::istream &m_in;
  std::vector< char > m_buf;
  Token m_next;
  bool m_at_eof;
  Location m_loc;

  // No value semantics
  Lexer( const Lexer & ) = delete;
  Lexer &operator=( const Lexer & ) = delete;

public:
  Lexer( std::istream &in, const std::string &filename );
  ~Lexer();

  bool peek( Token &tok );
  void next( Token &tok );

  // These are called by the generated lexer code

  // Get the next character of input.
  // Returns -1 if EOF has been reached.
  int get();

  // Unget the character most recently returned by get()
  void unget( int c );

  // Get the current location (i.e., the location of the
  // next character returned by get())
  const Location &get_loc() const;

private:
  int doget();
  bool fill();
  bool skip_ws_or_comment();
};

#endif // LEXER_H
