//  hlsim - simulate nearly_cc high-level intermediate code
//  Copyright (C) 2026, David H. Hovemeyer <david.hovemeyer@gmail.com>

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include "stack.h"
#include "stacktrace.h"
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
  void execute_sconv( const Instruction &ins, unsigned src_operand_size, unsigned dest_operand_size );
  void execute_uconv( const Instruction &ins, unsigned src_operand_size, unsigned dest_operand_size );
  void execute_scmp( const Instruction &ins, unsigned operand_size, Compare cmp );
  void execute_ucmp( const Instruction &ins, unsigned operand_size, Compare cmp );
  uint64_t load_source( Frame *frame, const Operand &operand, unsigned operand_size, const Location &loc );
  void store_dest( Frame *frame, const Operand &operand, unsigned operand_size, uint64_t val, const Location &loc );
  uint64_t read_vreg( Frame *frame, int vreg_num ) const;
  void write_vreg( Frame *frame, int vreg_num, uint64_t val );
  uint64_t read_mem( uint64_t addr, unsigned operand_size, const Location &loc );
  void write_mem( uint64_t addr, unsigned operand_size, uint64_t val, const Location &loc );
  Frame *determine_frame( uint64_t addr, const char *op, const Location &loc );
  uint64_t trim( uint64_t val, unsigned operand_size ) const;
  void create_stacktrace( StackTrace &stacktrace, const Instruction &ins ) const;
};

#endif // SIMULATOR_H
