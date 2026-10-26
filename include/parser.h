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

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "program.h"
#include "instruction.h"

class Parser {
private:
  Lexer &m_lexer;
  Program &m_program;
  std::shared_ptr<Function> m_cur_func;
  Instruction m_cur_ins;
  std::string m_cur_section;

  // No value semantics
  Parser( const Parser & ) = delete;
  Parser &operator=( const Parser & ) = delete;

public:
  Parser( Lexer &lexer, Program &program );
  ~Parser();

  void parse();

private:
  void parse_Unit();
  void parse_TItem();
  void parse_Section();
  void parse_Function();
  void parse_InsList();
  void parse_Ins();
  void parse_OptOperandList();
  void parse_OperandList();
  void parse_Operand();
  void parse_StringLiteral();

  void expect( TokenKind kind, Token &tok );
};

#endif // PARSER_H
