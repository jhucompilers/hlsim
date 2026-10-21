#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include "instruction.h"

class Function {
public:
  typedef std::vector< Instruction > InsList;

private:
  std::string m_name;
  InsList m_instructions;

  // No value semantics
  Function( const Function & ) = delete;
  Function &operator=( const Function & ) = delete;

public:
  Function( const std::string &name );
  ~Function();

  void add_instruction( const Instruction &ins );

  // Support for iterating over the instructions
  typedef InsList::const_iterator const_iterator;
  const_iterator begin() const;
  const_iterator end() const;
};

#endif // FUNCTION_H
