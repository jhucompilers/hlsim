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

#include <memory>
#include <string>
#include <cstring>
#include <cassert>
#include <unordered_map>
#include "function.h"
#include "location.h"
#include "instruction.h"
#include "exceptions.h"
#include "stack.h"
#include "stacktrace.h"
#include "program.h"

namespace {

// Instruction formats (expected number and kind of operands)
// for instructions which have only one valid format.
const std::unordered_map<Opcode, std::vector<OperandKind>> s_instruction_formats = {
  { Opcode::HINS_nop, {} },
  { Opcode::HINS_enter, { OperandKind::IMM } },
  { Opcode::HINS_leave, { OperandKind::IMM } },
  { Opcode::HINS_ret, {} },
  { Opcode::HINS_call, { OperandKind::LABEL } },
  { Opcode::HINS_localaddr, { OperandKind::VREG, OperandKind::IMM } },
  { Opcode::HINS_jmp, { OperandKind::LABEL } },
  { Opcode::HINS_cjmp_t, { OperandKind::VREG, OperandKind::LABEL } },
  { Opcode::HINS_cjmp_f, { OperandKind::VREG, OperandKind::LABEL } },
  { Opcode::HINS_call, { OperandKind::LABEL } },
  { Opcode::HINS_read_i32, {} },
  { Opcode::HINS_print_str, {} },
  { Opcode::HINS_print_nl, {} },
  { Opcode::HINS_print_i32, {} },
  { Opcode::HINS_read_i64, {} },
  { Opcode::HINS_print_i64, {} },
  { Opcode::HINS_print_space, {} },
};

}

Program::Program()
  : m_str_pool_size( 0 ) {

}

Program::~Program() {

}

void Program::add_function( std::shared_ptr<Function> fn, const Location &loc ) {
  if ( m_fn_map.find( fn->get_name() ) != m_fn_map.end() )
    throw SimError( "duplicate function name", loc );
  m_fn_map[ fn->get_name() ] = unsigned( m_functions.size() );
  m_functions.push_back( fn );
}

std::shared_ptr<Function> Program::lookup_fn( const std::string &fn_name, const Location &loc ) const {
  auto i = m_fn_map.find( fn_name );
  if ( i == m_fn_map.end() )
    throw SimError( "unknown function name '" + fn_name + "'", loc );
  return m_functions.at( i->second );
}

void Program::define_string_literal( const std::string &name, const std::string &str_value, const Location &loc ) {
  auto i = m_string_literals.find( name );
  if ( i != m_string_literals.end() )
    throw SimError( "duplicate string literal '" + name + "'", loc );

  m_string_literals[ name ] = str_value;
}

unsigned Program::get_str_pool_size() const {
  return m_str_pool_size;
}

const char *Program::get_str_lit_ptr( uint64_t addr, const Location &loc, ThrowSimErrorWithStackTrace *t ) const {
  // Make sure the address is in the valid address range
  // for string literals in the string pool
  uint64_t str_addr_base = Program::STR_POOL_BASE;
  if ( addr < str_addr_base || addr >= ( str_addr_base + m_str_pool_size ) )
    t->raise_simerr( "Invalid string literal address " + Stack::to_hex( addr ), loc );

  uint64_t offset = addr - str_addr_base;
  assert( offset < m_str_pool_size );

  return m_str_pool.get() + offset;
}

void Program::create_string_pool() {
  // Determine the total size of the string pool
  m_str_pool_size = 0;
  for ( auto i = m_string_literals.begin(); i != m_string_literals.end(); ++i )
    m_str_pool_size += ( i->second.size() + 1 );

  // Allocate memory for the string values
  m_str_pool = std::make_unique<char[]>( m_str_pool_size );

  // Addresses of string literals start exactly halfway between 0
  // and the base address of the first stack frame
  uint64_t str_addr_base = Program::STR_POOL_BASE;

  // Populate string pool memory, and build a map of string literal names
  // to their addresses
  unsigned offset = 0;
  for ( auto i = m_string_literals.begin(); i != m_string_literals.end(); ++i ) {
    const std::string &str_lit_name = i->first, &str_value = i->second;
    char *p = m_str_pool.get() + offset;
    unsigned str_value_size = str_value.size() + 1; // allow for NUL terminator
    assert( offset + str_value_size <= m_str_pool_size );
    std::strcpy( p, str_value.c_str() );
    m_string_literal_addrs[ str_lit_name ] = str_addr_base + offset;
    offset += str_value_size;
  }
}

void Program::validate() const {
  for ( auto i = m_functions.begin(); i != m_functions.end(); ++i )
    for ( auto j = (*i)->begin(); j != (*i)->end(); ++j )
      validate_instruction( **i, *j );
}

#define ADD_INTRINSIC(name) \
  do { \
    std::shared_ptr<Function> name = std::make_shared<Function>( #name ); \
    add_function( name, loc );  \
    name->add_instruction( Instruction( Opcode::HINS_##name, loc++ ) ); \
    name->add_instruction( Instruction( Opcode::HINS_ret, loc++ ) ); \
  } while (0)

void Program::add_intrinsics() {
  Location loc( "<intrinsics>", 1, 1 );

  ADD_INTRINSIC( read_i32 );
  ADD_INTRINSIC( print_i32 );
  ADD_INTRINSIC( read_i64 );
  ADD_INTRINSIC( print_i64 );
  ADD_INTRINSIC( print_str );
  ADD_INTRINSIC( print_nl );
  ADD_INTRINSIC( print_space );
}

void Program::validate_instruction( const Function &fn, Instruction &ins ) const {
  auto i = s_instruction_formats.find( ins.get_opcode() );

  if ( i != s_instruction_formats.end() ) {
    // Check operands of instruction against expected template
    const auto &expected = i->second;

    if ( ins.get_num_operands() != int( expected.size() ) )
      throw SimError( "wrong number of operands (expected " + std::to_string(expected.size()) + ")", ins.get_loc() );

    for ( size_t j = 0; j < expected.size(); ++j ) {
      const Operand &operand = ins.at( j );
      if ( operand.get_kind() != expected[j] )
        throw SimError( "invalid operand " + std::to_string(j+1), ins.get_loc() );
    }
  } else {
    // Instruction has multiple valid forms
    validate_instruction_variable_format( ins );
  }

  // Make sure any referenced labels refer to something valid,
  // precompute the target instruction offsets for control flow
  // instructions, and precompute addresses for referenced
  // string literals
  for ( int i = 0; i < ins.get_num_operands(); ++i ) {
    Operand &operand = ins.at( unsigned(i) );

    if ( operand.get_kind() == OperandKind::LABEL ) {
      if ( ins.get_opcode() == Opcode::HINS_call ) {
        // This will throw an exception if there is no function with
        // the referenced name
        lookup_fn( operand.get_label(), ins.get_loc() );
      } else {
        // Assume label is meant to refer to an instruction in
        // the function, i.e., it's for control flow. Note that
        // Function::lookup_label() will throw an exception
        // if there is no such target label.
        unsigned target_offset = fn.lookup_label( operand.get_label(), ins.get_loc() );
        operand.set_target_offset( target_offset );
      }
    }

    if ( operand.get_kind() == OperandKind::IMM_LABEL ) {
      // The only thing these can refer to is a string literal
      auto i = m_string_literal_addrs.find( operand.get_label() );
      if ( i == m_string_literal_addrs.end() )
        throw SimError( "unknown string literal '" + operand.get_label() + "'", ins.get_loc() );
      uint64_t str_lit_addr = i->second;
      operand.set_str_lit_addr( str_lit_addr );
    }
  }

  // Do any additional validation for specific instruction types
  validate_instruction_extra( ins );
}

void Program::validate_instruction_variable_format( Instruction &ins ) const {
  switch ( ins.get_opcode() ) {
  // Moves of various kinds
  case Opcode::HINS_mov_b:
  case Opcode::HINS_mov_w:
  case Opcode::HINS_mov_l:
  case Opcode::HINS_mov_q:
  case Opcode::HINS_sconv_bw:
  case Opcode::HINS_sconv_bl:
  case Opcode::HINS_sconv_bq:
  case Opcode::HINS_sconv_wl:
  case Opcode::HINS_sconv_wq:
  case Opcode::HINS_sconv_lq:
  case Opcode::HINS_uconv_bw:
  case Opcode::HINS_uconv_bl:
  case Opcode::HINS_uconv_bq:
  case Opcode::HINS_uconv_wl:
  case Opcode::HINS_uconv_wq:
  case Opcode::HINS_uconv_lq:
    validate_mov( ins );
    break;

  // All binary operator instructions (there are a lot of these)
  case Opcode::HINS_add_b:
  case Opcode::HINS_add_w:
  case Opcode::HINS_add_l:
  case Opcode::HINS_add_q:
  case Opcode::HINS_sub_b:
  case Opcode::HINS_sub_w:
  case Opcode::HINS_sub_l:
  case Opcode::HINS_sub_q:
  case Opcode::HINS_mul_b:
  case Opcode::HINS_mul_w:
  case Opcode::HINS_mul_l:
  case Opcode::HINS_mul_q:
  case Opcode::HINS_div_b:
  case Opcode::HINS_div_w:
  case Opcode::HINS_div_l:
  case Opcode::HINS_div_q:
  case Opcode::HINS_mod_b:
  case Opcode::HINS_mod_w:
  case Opcode::HINS_mod_l:
  case Opcode::HINS_mod_q:
  case Opcode::HINS_lshift_b:
  case Opcode::HINS_lshift_w:
  case Opcode::HINS_lshift_l:
  case Opcode::HINS_lshift_q:
  case Opcode::HINS_rshift_b:
  case Opcode::HINS_rshift_w:
  case Opcode::HINS_rshift_l:
  case Opcode::HINS_rshift_q:
  case Opcode::HINS_and_b:
  case Opcode::HINS_and_w:
  case Opcode::HINS_and_l:
  case Opcode::HINS_and_q:
  case Opcode::HINS_or_b:
  case Opcode::HINS_or_w:
  case Opcode::HINS_or_l:
  case Opcode::HINS_or_q:
  case Opcode::HINS_xor_b:
  case Opcode::HINS_xor_w:
  case Opcode::HINS_xor_l:
  case Opcode::HINS_xor_q:
  case Opcode::HINS_cmplt_b:
  case Opcode::HINS_cmplt_w:
  case Opcode::HINS_cmplt_l:
  case Opcode::HINS_cmplt_q:
  case Opcode::HINS_cmplte_b:
  case Opcode::HINS_cmplte_w:
  case Opcode::HINS_cmplte_l:
  case Opcode::HINS_cmplte_q:
  case Opcode::HINS_cmpgt_b:
  case Opcode::HINS_cmpgt_w:
  case Opcode::HINS_cmpgt_l:
  case Opcode::HINS_cmpgt_q:
  case Opcode::HINS_cmpgte_b:
  case Opcode::HINS_cmpgte_w:
  case Opcode::HINS_cmpgte_l:
  case Opcode::HINS_cmpgte_q:
  case Opcode::HINS_ucmplt_b:
  case Opcode::HINS_ucmplt_w:
  case Opcode::HINS_ucmplt_l:
  case Opcode::HINS_ucmplt_q:
  case Opcode::HINS_ucmplte_b:
  case Opcode::HINS_ucmplte_w:
  case Opcode::HINS_ucmplte_l:
  case Opcode::HINS_ucmplte_q:
  case Opcode::HINS_ucmpgt_b:
  case Opcode::HINS_ucmpgt_w:
  case Opcode::HINS_ucmpgt_l:
  case Opcode::HINS_ucmpgt_q:
  case Opcode::HINS_ucmpgte_b:
  case Opcode::HINS_ucmpgte_w:
  case Opcode::HINS_ucmpgte_l:
  case Opcode::HINS_ucmpgte_q:
  case Opcode::HINS_cmpeq_b:
  case Opcode::HINS_cmpeq_w:
  case Opcode::HINS_cmpeq_l:
  case Opcode::HINS_cmpeq_q:
  case Opcode::HINS_cmpneq_b:
  case Opcode::HINS_cmpneq_w:
  case Opcode::HINS_cmpneq_l:
  case Opcode::HINS_cmpneq_q:
    validate_binop( ins );
    break;

  // Validate unary operations
  case Opcode::HINS_neg_b:
  case Opcode::HINS_neg_w:
  case Opcode::HINS_neg_l:
  case Opcode::HINS_neg_q:
  case Opcode::HINS_not_b:
  case Opcode::HINS_not_w:
  case Opcode::HINS_not_l:
  case Opcode::HINS_not_q:
  case Opcode::HINS_compl_b:
  case Opcode::HINS_compl_w:
  case Opcode::HINS_compl_l:
  case Opcode::HINS_compl_q:
  case Opcode::HINS_inc_b:
  case Opcode::HINS_inc_w:
  case Opcode::HINS_inc_l:
  case Opcode::HINS_inc_q:
  case Opcode::HINS_dec_b:
  case Opcode::HINS_dec_w:
  case Opcode::HINS_dec_l:
  case Opcode::HINS_dec_q:
    validate_unop( ins );
    break;

  default:
    throw SimError( "don't know how to validate " + std::to_string(int(ins.get_opcode())), ins.get_loc() );
  }
}

void Program::validate_mov( Instruction &ins ) const {
  // There must be two operands
  if ( ins.get_num_operands() != 2 )
    throw SimError( "wrong number of operands", ins.get_loc() );
  
  // The destination operand must be assignable
  const Operand &dest = ins.at( 0 );
  if ( !dest.is_assignable() )
    throw SimError( "invalid destination operand", ins.get_loc() );

  // The source operand must be readable
  const Operand &src = ins.at( 1 );
  if ( !src.is_readable() )
    throw SimError( "invalid source operand", ins.get_loc() );
}

void Program::validate_binop( Instruction &ins ) const {
  // There must be exactly 3 operands
  if ( ins.get_num_operands() != 3 )
    throw SimError( "instruction should have exactly 3 operands", ins.get_loc() );

  // The destination operand must be assignable
  if ( !ins.at(0).is_assignable() )
    throw SimError( "invalid destination operand", ins.get_loc() );

  // The source operands must be readable
  if ( !ins.at(1).is_readable() || !ins.at(2).is_readable() )
    throw SimError( "invalid source operand", ins.get_loc() );
}

void Program::validate_unop( Instruction &ins ) const {
  // There must be exactly 2 operands
  if ( ins.get_num_operands() != 2 )
    throw SimError( "instruction should have exactly 2 operands", ins.get_loc() );

  // The destination must be assignable
  if ( !ins.at(0).is_assignable() )
    throw SimError( "invalid destination operand", ins.get_loc() );

  // The source operand must be readable
  if ( !ins.at(1).is_readable() )
    throw SimError( "invalid source operand", ins.get_loc() );
}

void Program::validate_instruction_extra( Instruction &ins ) const {
  switch ( ins.get_opcode() ) {
  case Opcode::HINS_localaddr:
    // offset must be non-negative
    if ( ins.at(1).get_imm() < 0L )
      throw SimError( "localaddr offset must be non-negative", ins.get_loc() );
    break;
  default:
    break;
  }
}
