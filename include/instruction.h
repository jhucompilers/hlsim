#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <initializer_list>
#include <vector>
#include "opcode.h"
#include "operand.h"

class Instruction {
private:
  Opcode m_opcode;
  std::vector< Operand > m_operands;

public:
  Instruction( Opcode opcode );
  Instruction( Opcode opcode, std::initializer_list< Operand > initializer_list );
  Instruction( const Instruction &other );
  ~Instruction();

  Instruction &operator=( const Instruction &rhs );

  Opcode get_opcode() const;
  int get_num_operands() const;
  const Operand &operator[]( size_t i ) const;
};

#endif // INSTRUCTION_H
