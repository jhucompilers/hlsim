#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <unordered_map>
#include "instruction.h"

class Function {
public:
  typedef std::vector< Instruction > InsList;

private:
  std::string m_name;
  InsList m_instructions;
  // map of label names to instruction indices
  std::unordered_map< std::string, unsigned > m_labels;

  // No value semantics
  Function( const Function & ) = delete;
  Function &operator=( const Function & ) = delete;

public:
  Function( const std::string &name );
  ~Function();

  void set_label( const std::string &label );
  void add_instruction( const Instruction &ins );

  // Support for iterating over the instructions
  typedef InsList::const_iterator const_iterator;
  const_iterator begin() const;
  const_iterator end() const;
};

#endif // FUNCTION_H
