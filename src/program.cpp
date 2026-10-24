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

void Program::validate() const {
  for ( auto i = m_functions.begin(); i != m_functions.end(); ++i )
    for ( auto j = (*i)->begin(); j != (*i)->end(); ++j )
      validate_instruction( **i, *j );
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
        throw SimError( "", ins.get_loc() );
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
  case Opcode::HINS_mov_b:
  case Opcode::HINS_mov_w:
  case Opcode::HINS_mov_l:
  case Opcode::HINS_mov_q:
    validate_mov( fn, ins );
    break;

  default:
    throw SimError( "don't know how to validate " + std::to_string(int(ins.get_opcode())), ins.get_loc() );
  }
}

void Program::validate_mov( const Function &fn, Instruction &ins ) const {
  // There must be two operands
  if ( ins.get_num_operands() != 2 )
    throw SimError( "wrong number of operands", ins.get_loc() );
  
  // The destination operand must be a vreg or a memory reference
  const Operand &dest = ins.at( 0 );
  if ( dest.get_kind() != OperandKind::VREG && dest.get_kind() != OperandKind::VREG_MEMREF )
    throw SimError( "invalid destination operand", ins.get_loc() );

  // The source operand must not be a label
  const Operand &src = ins.at( 1 );
  if ( src.get_kind() == OperandKind::LABEL )
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
