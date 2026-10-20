#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

class Parser {
private:
  Lexer &m_lexer;
  // TODO: Program

  // No value semantics
  Parser( const Parser & ) = delete;
  Parser &operator=( const Parser & ) = delete;

public:
  Parser( Lexer &lexer );
  ~Parser();

  void parse();

private:
  void parse_Unit();
  void parse_TItem();
  void parse_Section();
  void parse_Function();
  void parse_OptGlobal();
  void parse_InsList();
  void parse_Ins();
  void parse_OperandList();
  void parse_Operand();

  void expect( TokenKind kind, Token &tok );
};

#endif // PARSER_H
