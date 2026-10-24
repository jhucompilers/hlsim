#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include "stack.h"
class Program;
class Instruction;
class Operand;
class Function;

class Simulator {
private:
  const Program &m_program;
  Stack m_stack;
  bool m_done;
  uint64_t m_global_vregs[10];
  const Function *m_cur_fn;

  // no value semantics
  Simulator( const Simulator & ) = delete;
  Simulator &operator=( const Simulator & ) = delete;

public:
  Simulator( const Program &program );
  ~Simulator();

  uint64_t execute();

private:
  // Kinds of comparisons
  enum class Compare { LT, LTE, GT, GTE, EQ, NEQ };

  unsigned execute_ins( const Instruction &ins, unsigned pc );
  void execute_mov( const Instruction &ins );
  void execute_scmp( const Instruction &ins, unsigned operand_size, Compare cmp );
  uint64_t load_source( Frame *frame, const Operand &operand, unsigned operand_size, const Location &loc );
  void store_dest( Frame *frame, const Operand &operand, unsigned operand_size, uint64_t val, const Location &loc );
  uint64_t read_vreg( Frame *frame, int vreg_num ) const;
  void write_vreg( Frame *frame, int vreg_num, uint64_t val );
  uint64_t read_mem( uint64_t addr, unsigned operand_size, const Location &loc );
  void write_mem( uint64_t addr, unsigned operand_size, uint64_t val, const Location &loc );
  Frame *determine_frame( uint64_t addr, const char *op, const Location &loc );
  uint64_t trim( uint64_t val, unsigned operand_size ) const;
};

#endif // SIMULATOR_H
