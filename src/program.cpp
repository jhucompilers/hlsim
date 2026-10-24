#include <memory>
#include <string>
#include <unordered_map>
#include "function.h"
#include "location.h"
#include "instruction.h"
#include "exceptions.h"
#include "program.h"

namespace {

const std::unordered_map<OperandKind, std::string> s_operand_format_names = {
  { OperandKind::VREG, "virtual register" },
  { OperandKind::VREG_MEMREF, "virtual register memory reference" },
  { OperandKind::IMM, "immediate" },
  { OperandKind::LABEL, "label" },
};

// Instruction formats (expected number and kind of operands)
// for instructions which have only one valid format.
const std::unordered_map<Opcode, std::vector<OperandKind>> s_instruction_formats = {
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
};

}

Program::Program() {

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
    throw SimError( "unknown function name", loc );
  return m_functions.at( i->second );
}

void Program::define_string_literal( const std::string &name, const std::string &str_value, const Location &loc ) {
  auto i = m_string_literals.find( name );
  if ( i != m_string_literals.end() )
    throw SimError( "duplicate string literal '" + name + "'", loc );

  m_string_literals[ name ] = str_value;
}

void Program::validate() const {
  for ( auto i = m_functions.begin(); i != m_functions.end(); ++i )
    for ( auto j = (*i)->begin(); j != (*i)->end(); ++j )
      validate_instruction( **i, *j );
}

void Program::add_intrinsics() {
  Location loc( "<intrinsics>", 1, 1 );

  std::shared_ptr<Function> read_i32 = std::make_shared<Function>( "read_i32" );
  add_function( read_i32, loc ); 
  read_i32->add_instruction( Instruction( Opcode::HINS_read_i32, loc++ ) );
  read_i32->add_instruction( Instruction( Opcode::HINS_ret, loc++ ) );
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
    validate_instruction_variable_format( fn, ins );
  }

  // Make sure any referenced labels refer to something valid,
  // and precompute the target instruction offsets for control flow
  // instructions
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
  }

  // Do any additional validation for specific instruction types
  validate_instruction_extra( fn, ins );
}

void Program::validate_instruction_variable_format( const Function &fn, Instruction &ins ) const {
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
    validate_mov( fn, ins );
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
    validate_binop( fn, ins );
    break;

  default:
    throw SimError( "don't know how to validate " + std::to_string(int(ins.get_opcode())), ins.get_loc() );
  }
}

void Program::validate_mov( const Function &fn, Instruction &ins ) const {
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

void Program::validate_binop( const Function &fn, Instruction &ins ) const {
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

void Program::validate_instruction_extra( const Function &fn, Instruction &ins ) const {
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
