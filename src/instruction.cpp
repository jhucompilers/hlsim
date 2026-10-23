#include <cassert>
#include "instruction.h"

Instruction::Instruction( Opcode opcode, const Location &loc )
  : m_opcode( opcode )
  , m_loc( loc ) {

}

Instruction::Instruction( Opcode opcode, const Location &loc, std::initializer_list< Operand > initializer_list )
  : m_opcode( opcode )
  , m_operands( initializer_list )
  , m_loc( loc ) {

}

Instruction::Instruction( const Instruction &other )
  : m_opcode( other.m_opcode )
  , m_operands( other.m_operands )
  , m_loc( other.m_loc ) {

}

Instruction::~Instruction() {

}

Instruction &Instruction::operator=( const Instruction &rhs ) {
  if ( this != &rhs ) {
    m_opcode = rhs.m_opcode;
    m_operands = rhs.m_operands;
    m_loc = rhs.m_loc;
  }
  return *this;
}

void Instruction::append_operand( const Operand &operand ) {
  m_operands.push_back( operand );
}

Opcode Instruction::get_opcode() const {
  return m_opcode;
}

int Instruction::get_num_operands() const {
  return int( m_operands.size() );
}

const Operand &Instruction::operator[]( size_t i ) const {
  assert( i < m_operands.size() );
  return m_operands[ i ];
}

const Location &Instruction::get_loc() const {
  return m_loc;
}
