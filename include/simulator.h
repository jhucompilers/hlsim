#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include "stack.h"
class Program;
class Instruction;
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

  void execute();

private:
  unsigned execute_ins( const Instruction &ins, unsigned pc );
};

#endif // SIMULATOR_H
