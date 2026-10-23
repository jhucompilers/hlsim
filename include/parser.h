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
  // TODO: Program

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

  void expect( TokenKind kind, Token &tok );
};

#endif // PARSER_H
