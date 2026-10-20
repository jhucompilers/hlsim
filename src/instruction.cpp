#include "instruction.h"
#include <stdexcept>

Instruction::Instruction( Opcode opcode )
  : m_opcode( opcode ) {

}

Instruction::Instruction( Opcode opcode, std::initializer_list< Operand > initializer_list )
  : m_opcode( opcode )
  , m_operands( initializer_list ) {

}

Instruction::Instruction( const Instruction &other )
  : m_opcode( other.m_opcode )
  , m_operands( other.m_operands ) {

}

Instruction::~Instruction() {

}

Instruction &Instruction::operator=( const Instruction &rhs ) {
  if ( this != &rhs ) {
    m_opcode = rhs.m_opcode;
    m_operands = rhs.m_operands;
  }
  return *this;
}

Opcode Instruction::get_opcode() const {
  return m_opcode;
}

int Instruction::get_num_operands() const {
  return int( m_operands.size() );
}

const Operand &Instruction::operator[]( size_t i ) const {
  if ( i >= m_operands.size() )
    throw std::runtime_error( "Operand index is out of bounds" );
  return m_operands[ i ];
}
