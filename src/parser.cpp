#include "parser.h"
#include "lexer.h"

// Grammar
//   Unit -> TItem
//   Unit -> TItem Unit
//   TItem -> Section
//   TItem -> Function
//   Section -> '.section' ident
//   Function -> '.globl' ident ident ':' InsList
//   InsList -> Ins
//   InsList -> Ins InsList
//   Ins -> ident OperandList
//   Ins -> ident ': ident OperandList
//   OperandList -> Operand
//   OperandList -> Operand ',' OperandList
//   Operand -> vreg
//   Operand -> '(' vreg ')'
//   Operand -> imm
//   Operand -> ident

Parser::Parser( Lexer &lexer )
  : m_lexer( lexer ) {

}

Parser::~Parser() {

}

void Parser::parse() {
  parse_Unit();
}

void Parser::parse_Unit() {
  Token tok;
  do {
    parse_TItem();
  } while ( m_lexer.peek( tok ) );
}

void Parser::parse_TItem() {
  Token tok;
  if ( !m_lexer.peek( tok ) )
    throw std::runtime_error( "Unexpected EOF looking for top-level item" );
  if ( tok.get_kind() == TokenKind::SECTION ) {
    parse_Section();
    return;
  }
  parse_Function();
}

void Parser::parse_Section() {
  Token tok;
  // Section -> ^ .section ident
  m_lexer.next( tok );
  expect( TokenKind::IDENT, tok );
  return;
}

void Parser::parse_Function() {
  // Function -> '.globl' ident ident ':' InsList
  Token globl;
  expect( TokenKind::GLOBL, globl );
  Token ident;
  expect( TokenKind::IDENT, ident );
  expect( TokenKind::IDENT, ident );
  Token colon;
  expect( TokenKind::COLON, colon );
  parse_InsList();
}

void Parser::parse_OptGlobal() {
  Token tok;
  if ( !m_lexer.peek( tok ) || tok.get_kind() != TokenKind::GLOBL )
    return;
  expect( TokenKind::GLOBL, tok );
  expect( TokenKind::IDENT, tok );
}

void Parser::parse_InsList() {
  bool more = true;
  while ( more ) {
    parse_Ins();
    Token tok;
    if ( !m_lexer.peek( tok ) )
      more = false;
    else if ( tok.get_kind() == TokenKind::GLOBL ||
              tok.get_kind() == TokenKind::SECTION )
      more = false;
  }
}

void Parser::parse_Ins() {
  // Ins -> ident OperandList
  // Ins -> ident ': ident OperandList

  Token first, second;
  std::string label;

  Token mnemonic;

  expect( TokenKind::IDENT, first );
  if ( !m_lexer.peek( second ) )
    throw std::runtime_error( "Unexpected EOF in instruction" );
  if ( second.get_kind() == TokenKind::COLON ) {
    // We have a label
    label = first.get_lexeme();

    // Consume the ':'
    m_lexer.next( second );

    // Get the actual mnemonic
    expect( TokenKind::IDENT, mnemonic );
  } else {
    // This instruction is not labeled
    mnemonic = first;
  }

  parse_OperandList();
}

void Parser::parse_OperandList() {
  // OperandList -> Operand
  // OperandList -> Operand ',' OperandList

  parse_Operand();
  Token tok;
  if ( m_lexer.peek( tok ) && tok.get_kind() == TokenKind::COMMA ) {
    expect( TokenKind::COMMA, tok );
    parse_OperandList();
  }
}

void Parser::parse_Operand() {
  // Operand -> vreg
  // Operand -> '(' vreg ')'
  // Operand -> imm
  // Operand -> ident

  Token tok;
  if ( !m_lexer.peek( tok ) )
    throw std::runtime_error( "unexpected EOF parsing operand" );

  if ( tok.get_kind() == TokenKind::VREG ) {
    // is a virtual register
    m_lexer.next( tok );
  } else if ( tok.get_kind() == TokenKind::LPAREN ) {
    // is a memory reference via a vreg storing a pointer
    expect( TokenKind::LPAREN, tok );
    expect( TokenKind::VREG, tok );
    expect( TokenKind::RPAREN, tok );
  } else if ( tok.get_kind() == TokenKind::IMMEDIATE ) {
    // is an immediate operand
    expect( TokenKind::IMMEDIATE, tok );
  } else {
    // is a label
    expect( TokenKind::IDENT, tok );
  }
}

void Parser::expect( TokenKind kind, Token &tok ) {
  m_lexer.next( tok );
  if ( tok.get_kind() != kind )
    throw std::runtime_error( "Unexpected token" );
}
