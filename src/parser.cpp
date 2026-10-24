#include <memory>
#include <set>
#include <cassert>
#include "parser.h"
#include "lexer.h"
#include "exceptions.h"
#include "function.h"
#include "instruction.h"

// Grammar
//   Unit -> TItem
//   Unit -> TItem Unit
//   TItem -> Section
//   TItem -> Function
//   TItem -> StringLiteral
//   Section -> '.section' ident
//   Function -> '.globl' ident ident ':' InsList
//   InsList -> Ins
//   InsList -> Ins InsList
//   Ins -> opcode OptOperandList
//   Ins -> ident ': opcode OptOperandList
//   OptOperandList -> OperandList
//   OptOperandList -> ε
//   OperandList -> Operand
//   OperandList -> Operand ',' OperandList
//   Operand -> vreg
//   Operand -> '(' vreg ')'
//   Operand -> imm
//   Operand -> imm_label
//   Operand -> ident
//   StringLiteral -> ident ':' '.string' string_literal

Parser::Parser( Lexer &lexer, Program &program )
  : m_lexer( lexer )
  , m_program( program ) {

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
  // TItem -> Section
  // TItem -> Function
  // TItem -> StringLiteral

  Token tok;
  if ( !m_lexer.peek( tok ) )
    throw SimError( "Unexpected EOF looking for top-level item", m_lexer.get_loc() );
  if ( tok.get_kind() == TokenKind::SECTION )
    parse_Section();
  else if ( tok.get_kind() == TokenKind::GLOBL )
    parse_Function();
  else
    parse_StringLiteral();
}

void Parser::parse_Section() {
  Token tok;
  // Section -> ^ .section ident
  expect( TokenKind::SECTION, tok );
  expect( TokenKind::IDENT, tok );
  m_cur_section = tok.get_lexeme();
  return;
}

void Parser::parse_Function() {
  if ( m_cur_section != ".text" )
    throw SimError( "functions must be defined in the .text section", m_lexer.get_loc() );

  // Function -> '.globl' ident ident ':' InsList
  Token globl;
  expect( TokenKind::GLOBL, globl );
  Token ident;
  expect( TokenKind::IDENT, ident );
  expect( TokenKind::IDENT, ident );
  // start new function
  m_cur_func = std::make_shared<Function>( ident.get_lexeme());
  Token colon;
  expect( TokenKind::COLON, colon );
  parse_InsList();

  // Add new function to Program
  m_program.add_function( m_cur_func, ident.get_loc() );
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
  // Ins -> opcode OptOperandList
  // Ins -> ident ': opcode OptOperandList

  std::string label;

  Token tok;
  m_lexer.next( tok );
  if ( tok.get_kind() == TokenKind::IDENT ) {
    // there is a label
    label = tok.get_lexeme();
    m_cur_func->set_label( label );
    expect( TokenKind::COLON, tok );
    m_lexer.next( tok ); // reads the opcode
  }

  if ( tok.get_kind() != TokenKind::OPCODE )
    throw SimError( "expected opcode", tok.get_loc() );

  Opcode opcode = opcode_from_s( tok.get_lexeme() );
  assert( opcode != Opcode::INVALID );

  // start new instruction
  m_cur_ins = Instruction( opcode, tok.get_loc() );
  parse_OptOperandList();
  m_cur_func->add_instruction( m_cur_ins );
}

void Parser::parse_OptOperandList() {
  // OptOperandList -> OperandList
  // OptOperandList -> ε

  // FIRST(OperandList) = { vreg, '(', imm, ident }
  static const std::set< TokenKind > first_OperandList = {
    TokenKind::VREG,
    TokenKind::LPAREN,
    TokenKind::IMMEDIATE,
    TokenKind::IDENT
  };

  Token tok;
  if ( m_lexer.peek( tok ) &&
       first_OperandList.find( tok.get_kind() ) != first_OperandList.end() ) {
    parse_OperandList();
  }
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

  if ( m_cur_ins.get_num_operands() >= 3 )
    throw SimError( "hlsim allows at most 3 operands per instruction", m_lexer.get_loc() );

  Token tok;
  if ( !m_lexer.peek( tok ) )
    throw SimError( "unexpected EOF parsing operand", m_lexer.get_loc() );

  if ( tok.get_kind() == TokenKind::VREG ) {
    // is a virtual register
    m_lexer.next( tok );
    int vreg_num = std::stoi( tok.get_lexeme().substr( 2 ) );
    m_cur_ins.append_operand( Operand( OperandKind::VREG, vreg_num ) );
  } else if ( tok.get_kind() == TokenKind::LPAREN ) {
    // is a memory reference via a vreg storing a pointer
    expect( TokenKind::LPAREN, tok );
    expect( TokenKind::VREG, tok );
    int vreg_num = std::stoi( tok.get_lexeme().substr( 2 ) );
    expect( TokenKind::RPAREN, tok );
    m_cur_ins.append_operand( Operand( OperandKind::VREG_MEMREF, vreg_num ) );
  } else if ( tok.get_kind() == TokenKind::IMMEDIATE ) {
    // is an immediate operand
    expect( TokenKind::IMMEDIATE, tok );
    long imm_val = std::stol( tok.get_lexeme().substr( 1 ) );
    m_cur_ins.append_operand( Operand( OperandKind::IMM, imm_val ) );
  } else if ( tok.get_kind() == TokenKind::IMMEDIATE_LABEL) {
    // immediate label (i.e., specifying the memory address of a string constant)
    expect( TokenKind::IMMEDIATE_LABEL, tok );
    m_cur_ins.append_operand( Operand( OperandKind::IMM_LABEL, tok.get_lexeme() ) );
  } else {
    // is a label
    expect( TokenKind::IDENT, tok );
    m_cur_ins.append_operand( Operand( OperandKind::LABEL, tok.get_lexeme() ) );
  }
}

void Parser::parse_StringLiteral() {
  if ( m_cur_section != ".rodata" )
    throw SimError( "string constants must be in the .rodata section", m_lexer.get_loc() );

  // StringLiteral -> ident ':' '.string' string_literal
  Token ident;
  expect( TokenKind::IDENT, ident );
  Token tok;
  expect( TokenKind::COLON, tok );
  expect( TokenKind::STRING, tok );
  expect( TokenKind::STRING_LITERAL, tok );

  std::string str_value = tok.get_string_literal_value();
  m_program.define_string_literal( ident.get_lexeme(), str_value, ident.get_loc() );
}

void Parser::expect( TokenKind kind, Token &tok ) {
  m_lexer.next( tok );
  if ( tok.get_kind() != kind )
    throw SimError( "Unexpected token", tok.get_loc() );
}
