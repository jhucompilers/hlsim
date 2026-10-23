#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "stack.h"
class Program;
class Instruction;

class Simulator {
private:
  const Program &m_program;
  Stack m_stack;
  bool m_done;

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
