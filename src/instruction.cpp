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

#include <cassert>
#include <stdexcept>
#include "instruction.h"

Instruction::Instruction( Opcode opcode, const Location &loc )
  : m_opcode( opcode )
  , m_num_operands( 0 )
  , m_loc( loc ) {

}

Instruction::Instruction( Opcode opcode, const Location &loc, std::initializer_list< Operand > initializer_list )
  : m_opcode( opcode )
  , m_num_operands( 0 )
  , m_loc( loc ) {
  for ( auto i = initializer_list.begin(); i != initializer_list.end(); ++i )
    append_operand( *i );
}

Instruction::Instruction( const Instruction &other )
  : m_opcode( other.m_opcode )
  , m_num_operands( other.m_num_operands )
  , m_loc( other.m_loc ) {
  copy_operands( other );
}

Instruction::~Instruction() {

}

Instruction &Instruction::operator=( const Instruction &rhs ) {
  if ( this != &rhs ) {
    m_opcode = rhs.m_opcode;
    copy_operands( rhs );
    m_num_operands = rhs.m_num_operands;
    m_loc = rhs.m_loc;
  }
  return *this;
}

void Instruction::append_operand( const Operand &operand ) {
  assert( m_num_operands < 3 );
  m_operands[ m_num_operands++ ] = operand;
}

Opcode Instruction::get_opcode() const {
  return m_opcode;
}

int Instruction::get_num_operands() const {
  return m_num_operands;
}

const Operand &Instruction::at( size_t i ) const {
  if ( i >= size_t( m_num_operands ) )
    throw std::out_of_range( "invalid operand index" );
  return m_operands[ i ];
}

Operand &Instruction::at( size_t i ) {
  if ( i >= size_t( m_num_operands ) )
    throw std::out_of_range( "invalid operand index" );
  return m_operands[ i ];
}

const Location &Instruction::get_loc() const {
  return m_loc;
}

void Instruction::copy_operands( const Instruction &ins ) {
  for ( int i = 0; i < 3; ++i )
    m_operands[i] = ins.m_operands[i];
}
