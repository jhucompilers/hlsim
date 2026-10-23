#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <initializer_list>
#include <vector>
#include "opcode.h"
#include "operand.h"
#include "location.h"

class Instruction {
private:
  Opcode m_opcode;
  std::vector< Operand > m_operands;
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
  const Operand &operator[]( size_t i ) const;
  const Location &get_loc() const;
};

#endif // INSTRUCTION_H
