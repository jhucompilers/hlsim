#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <memory>
#include <map>
#include <string>

class Function;
class Location;
class Instruction;

class Program {
private:
  std::vector<std::shared_ptr<Function>> m_functions;
  std::map<std::string, unsigned> m_fn_map;

  // no value semantics
  Program( const Program & ) = delete;
  Program &operator=( const Program & ) = delete;

public:
  Program();
  ~Program();

  void add_function( std::shared_ptr<Function> fn, const Location &loc );
  std::shared_ptr<Function> lookup_fn( const std::string &fn_name, const Location &loc ) const;

  // Check all Instructions in all Functions to make sure they
  // have a valid form (to avoid needing to check that at simulation
  // time)
  void validate() const;

private:
  void validate_instruction( const Function &fn, Instruction &ins ) const;
  void validate_instruction_variable_format( const Function &fn, Instruction &ins ) const;
  void validate_mov( const Function &fn, Instruction &ins ) const;
  void validate_instruction_extra( const Function &fn, Instruction &ins ) const;
};

#endif // PROGRAM_H
