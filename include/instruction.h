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

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <initializer_list>
#include "opcode.h"
#include "operand.h"
#include "location.h"

class Instruction {
private:
  Opcode m_opcode;
  Operand m_operands[3];
  int m_num_operands;
  Location m_loc;

public:
  Instruction( Opcode opcode = Opcode::INVALID, const Location &loc = Location() );
  Instruction( Opcode opcode, const Location &loc, std::initializer_list< Operand > initializer_list );
  Instruction( const Instruction &other );
  ~Instruction();

  Instruction &operator=( const Instruction &rhs );

  void append_operand( const Operand &operand );

  Opcode get_opcode() const;
  int get_num_operands() const;
  const Operand &at( size_t i ) const;
  Operand &at( size_t i );
  const Location &get_loc() const;

private:
  void copy_operands( const Instruction &ins );
};

#endif // INSTRUCTION_H
