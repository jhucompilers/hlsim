#ifndef PROGRAM_H
#define PROGRAM_H

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

class Function;
class Location;
class Instruction;

class Program {
private:
  std::vector<std::shared_ptr<Function>> m_functions;
  std::unordered_map<std::string, unsigned> m_fn_map;
  std::unordered_map<std::string, std::string> m_string_literals;
  unsigned m_str_pool_size;
  std::unique_ptr<char[]> m_str_pool;
  std::unordered_map<std::string, uint64_t> m_string_literal_addrs;

  // no value semantics
  Program( const Program & ) = delete;
  Program &operator=( const Program & ) = delete;

public:
  Program();
  ~Program();

  void add_function( std::shared_ptr<Function> fn, const Location &loc );
  std::shared_ptr<Function> lookup_fn( const std::string &fn_name, const Location &loc ) const;

  void define_string_literal( const std::string &name, const std::string &str_value, const Location &loc );
  unsigned get_str_pool_size() const;

  // Get a pointer to the string literal referred-to by given runtime address
  const char *get_str_lit_ptr( uint64_t addr, const Location &loc ) const;

  // Create the string pool and determine addresses of
  // string literals
  void create_string_pool();

  // Check all Instructions in all Functions to make sure they
  // have a valid form (to avoid needing to check that at simulation
  // time)
  void validate() const;

  // Add implementations of intrinsic (runtime) functions
  void add_intrinsics();

private:
  void validate_instruction( const Function &fn, Instruction &ins ) const;
  void validate_instruction_variable_format( const Function &fn, Instruction &ins ) const;
  void validate_mov( const Function &fn, Instruction &ins ) const;
  void validate_binop( const Function &fn, Instruction &ins ) const;
  void validate_instruction_extra( const Function &fn, Instruction &ins ) const;
};

#endif // PROGRAM_H
