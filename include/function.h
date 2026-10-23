#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <unordered_map>
#include <string>
#include "instruction.h"
class Location;

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

  const std::string &get_name() const;

  void set_label( const std::string &label );
  void add_instruction( const Instruction &ins );

  unsigned lookup_label( const std::string &label, const Location &loc ) const;

  // Support for iterating over the instructions
  typedef InsList::iterator iterator;
  iterator begin();
  iterator end();

  unsigned get_num_instructions() const;
  const Instruction &at( unsigned index ) const;
};

#endif // FUNCTION_H
