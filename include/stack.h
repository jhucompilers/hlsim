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

#ifndef STACK_H
#define STACK_H

#include "stacktrace.h"
#include <cstdint>
#include <memory>
#include <vector>
class Location;
class Function;
class ThrowSimErrorWithStackTrace;

// A block of memory allocated for storage of memory variables
// in a stack frame.
class Block {
public:
  // Number of bits in the stack frame offset part of an
  // address.
  static constexpr unsigned ADDR_RANGE_SHIFT = 25;

  // Number of bytes in the address range for the memory block
  // of one stack frame. This is, therefore, the maximum amount
  // of memory that can be allocated.
  static constexpr uint64_t ADDR_RANGE = ( 1UL << ADDR_RANGE_SHIFT );

private:
  uint64_t m_base;
  char *m_mem;
  uint64_t m_size;

  // no value semantics
  Block( const Block & ) = delete;
  Block &operator=( const Block & ) = delete;

public:
  Block( uint64_t base, uint64_t size );
  ~Block();

  uint64_t get_base() const;
  uint64_t get_size() const;

  void storeb( uint64_t addr, uint8_t val, const Location &loc, ThrowSimErrorWithStackTrace *t );
  void storew( uint64_t addr, uint16_t val, const Location &loc, ThrowSimErrorWithStackTrace *t );
  void storel( uint64_t addr, uint32_t val, const Location &loc, ThrowSimErrorWithStackTrace *t );
  void storeq( uint64_t addr, uint64_t val, const Location &loc, ThrowSimErrorWithStackTrace *t );

  uint8_t loadb( uint64_t addr, const Location &loc, ThrowSimErrorWithStackTrace *t );
  uint16_t loadw( uint64_t addr, const Location &loc, ThrowSimErrorWithStackTrace *t );
  uint32_t loadl( uint64_t addr, const Location &loc, ThrowSimErrorWithStackTrace *t );
  uint64_t loadq( uint64_t addr, const Location &loc, ThrowSimErrorWithStackTrace *t );

private:
  uint64_t bounds_and_alignment_check( const char *op, uint64_t addr, uint64_t size, const Location &loc, ThrowSimErrorWithStackTrace *t );
};

// A stack frame.
class Frame {
private:
  uint64_t m_base;
  const Function *m_caller, *m_callee; // pointers to caller and callee
  unsigned m_ret_ins_offset; // offset of the instruction to return to in the caller
  std::unique_ptr<Block> m_block; // memory block
  std::unique_ptr<uint64_t[]> m_local_vregs;

  // no value semantics
  Frame( const Frame & ) = delete;
  Frame &operator=( const Frame & ) = delete;

public:
  Frame( uint64_t base, const Function *caller, const Function *callee, unsigned ret_ins_offset );
  ~Frame();

  uint64_t get_base() const;

  const Function *get_caller() const;
  const Function *get_callee() const;
  unsigned get_ret_ins_offset() const;

  void enter( uint64_t nbytes, const Location &loc, ThrowSimErrorWithStackTrace *t );
  void leave( uint64_t nbytes, const Location &loc, ThrowSimErrorWithStackTrace *t );

  Block *get_block( const Location &loc, ThrowSimErrorWithStackTrace *t );

  void store_local_vreg( int vreg_num, uint64_t val );
  uint64_t load_local_vreg( int vreg_num );
};

class Stack {
private:
  std::vector< std::unique_ptr<Frame> > m_stack;

  // no value semantics
  Stack( const Stack & ) = delete;
  Stack &operator=( const Stack & ) = delete;

public:
  Stack();
  ~Stack();

  // Push a new stack frame to begin a function call
  void call( uint64_t base, const Function *caller, const Function *callee, unsigned ret_ins_offset );

  // Pop the current stack frame to end a function call:
  // stores the pointer to the function being returned to
  // and the instruction offset in that function in the
  // variables referenced by the parameters
  void ret( const Function *&caller, unsigned &ret_ins_offset );

  // Get a pointer to the top Frame
  Frame *get_top() const;

  // Return how many Frames are currently on the stack
  unsigned get_num_frames() const;

  // Return a pointer to a frame (0 is the frame for main,
  // 1 is the function called from main, etc.)
  Frame *get_frame( unsigned index ) const;

  // Convert a uint64_t value into a hexadecimal string
  static std::string to_hex( uint64_t val );
};

#endif // STACK_H
