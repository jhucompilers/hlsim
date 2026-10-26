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
#include "exceptions.h"
#include "function.h"

Function::Function( const std::string &name )
  : m_name( name )
  , m_highest_vreg_num( 9 ) {

}

Function::~Function() {

}

const std::string &Function::get_name() const {
  return m_name;
}

void Function::set_label( const std::string &label ) {
  m_labels[ label ] = unsigned( m_instructions.size() );
}

void Function::add_instruction( const Instruction &ins ) {
  m_instructions.push_back( ins );

  // Keep track of the highest vreg number referenced by
  // any instruction
  for ( int i = 0; i < ins.get_num_operands(); ++i ) {
    const Operand &operand = ins.at( unsigned(i) );
    if ( operand.has_vreg() )
      if ( operand.get_vreg() > m_highest_vreg_num )
        m_highest_vreg_num = operand.get_vreg();
  }

  // Update end location
  const Location &loc = ins.get_loc();
  if ( loc.get_line() > m_end_loc.get_line() )
    m_end_loc = ins.get_loc();
}

unsigned Function::lookup_label( const std::string &label, const Location &loc ) const {
  auto i = m_labels.find( label );
  if ( i == m_labels.end() )
    throw SimError( "unknown control flow label", loc );
  return i->second;
}

Function::iterator Function::begin() {
  return m_instructions.begin();
}

Function::iterator Function::end() {
  return m_instructions.end();
}

unsigned Function::get_num_instructions() const {
  return unsigned( m_instructions.size() );
}

const Instruction &Function::at( unsigned index ) const {
  assert( index < m_instructions.size() );
  return m_instructions.at( index );
}

int Function::get_highest_vreg_num() const {
  return m_highest_vreg_num;
}

const Location &Function::get_end_loc() const {
  return m_end_loc;
}
