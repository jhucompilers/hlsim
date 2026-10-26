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

#include <memory>
#include <string>
#include <sstream>
#include <cassert>
#include <cstring>
#include "function.h"
#include "stack.h"
#include "simulator.h"

namespace {

std::string to_hex_s( uint64_t x ) {
  std::stringstream ss;
  ss << std::hex << x;
  return ss.str();
}

}

// Block class: used for simulating the memory block of
// one stack frame

Block::Block( uint64_t base, uint64_t size )
  : m_base( base )
  , m_mem( new char[ size ] )
  , m_size( size ) {
  std::memset( m_mem, 0xFF, m_size );
}

Block::~Block() {
  delete[] m_mem;
}

uint64_t Block::get_base() const {
  return m_base;
}

uint64_t Block::get_size() const {
  return m_size;
}

void Block::storeb( uint64_t addr, uint8_t val, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "store", addr, 1, loc );
  uint8_t *p = reinterpret_cast<uint8_t*>( m_mem + offset );
  *p = val;
}

void Block::storew( uint64_t addr, uint16_t val, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "store", addr, 2, loc );
  uint16_t *p = reinterpret_cast<uint16_t*>( m_mem + offset );
  *p = val;
}

void Block::storel( uint64_t addr, uint32_t val, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "store", addr, 4, loc );
  uint32_t *p = reinterpret_cast<uint32_t*>( m_mem + offset );
  *p = val;
}

void Block::storeq( uint64_t addr, uint64_t val, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "store", addr, 8, loc );
  uint64_t *p = reinterpret_cast<uint64_t*>( m_mem + offset );
  *p = val;
}

uint8_t Block::loadb( uint64_t addr, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "load", addr, 1, loc );
  uint8_t *p = reinterpret_cast<uint8_t*>( m_mem + offset );
  return *p;
}

uint16_t Block::loadw( uint64_t addr, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "load", addr, 2, loc );
  uint16_t *p = reinterpret_cast<uint16_t*>( m_mem + offset );
  return *p;
}

uint32_t Block::loadl( uint64_t addr, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "load", addr, 4, loc );
  uint32_t *p = reinterpret_cast<uint32_t*>( m_mem + offset );
  return *p;
}

uint64_t Block::loadq( uint64_t addr, const Location &loc ) {
  uint64_t offset = bounds_and_alignment_check( "load", addr, 8, loc );
  uint64_t *p = reinterpret_cast<uint64_t*>( m_mem + offset );
  return *p;
}

uint64_t Block::bounds_and_alignment_check( const char *op, uint64_t addr, uint64_t size, const Location &loc ) {
  // The simulator should only attempt a memory access if the address
  // is within the correct address range for the stack frame.
  assert( addr >= m_base );
  assert( addr < m_base + ADDR_RANGE );

  // Compute the offset within the memory block
  uint64_t offset = addr - m_base;

  // Check whether the access is in bounds and correctly aligned
  bool in_bounds = ( offset + size <= m_size );
  bool aligned = ( offset & ( size-1 ) ) == 0U;

  if ( in_bounds && aligned )
    return offset;

  std::string msg;
  msg += "Invalid ";
  msg += op;
  msg += " offset=";
  msg += to_hex_s( offset );
  msg += ", size=";
  msg += std::to_string( size );
  msg += " (";
  if ( !in_bounds )
    msg += "out of bounds";
  if ( !aligned ) {
    if ( !in_bounds )
      msg += ", ";
    msg += "misaligned";
  }
  msg += ")";
  Simulator::raise_simerr( msg, loc );
}

// Frame class: simulates a stack frame

Frame::Frame( uint64_t base, const Function *caller, const Function *callee, unsigned ret_ins_offset )
  : m_base( base )
  , m_caller( caller )
  , m_callee( callee )
  , m_ret_ins_offset( ret_ins_offset ) {
  // Reserve space for local virtual register values
  assert( callee->get_highest_vreg_num() >= 9 );
  int num_local_vregs = callee->get_highest_vreg_num() - 9;
  m_local_vregs = std::make_unique<uint64_t[]>( num_local_vregs );
}

Frame::~Frame() {

}

uint64_t Frame::get_base() const {
  return m_base;
}

const Function *Frame::get_caller() const {
  return m_caller;
}

const Function *Frame::get_callee() const {
  return m_callee;
}

unsigned Frame::get_ret_ins_offset() const {
  return m_ret_ins_offset;
}

void Frame::enter( uint64_t nbytes, const Location &loc ) {
  if ( m_block.get() != nullptr )
    Simulator::raise_simerr( "multiple enter instructions in one function?", loc );
  m_block = std::make_unique<Block>( m_base, nbytes );
}

void Frame::leave( uint64_t nbytes, const Location &loc ) {
  if ( !m_block.get() )
    Simulator::raise_simerr( "leave instruction without enter instruction", loc );
  if ( m_block->get_size() != nbytes )
    Simulator::raise_simerr( "leave instruction size mismatch with enter", loc );
  m_block.reset( nullptr );
}

Block *Frame::get_block( const Location &loc ) {
  Block *p = m_block.get();
  if ( !p )
    Simulator::raise_simerr( "memory operation in stack frame with no memory block", loc );
  return p;
}

void Frame::store_local_vreg( int vreg_num, uint64_t val ) {
  assert( vreg_num >= 10 );
  assert( vreg_num <= m_callee->get_highest_vreg_num() );
  m_local_vregs[ vreg_num - 10 ] = val;
}

uint64_t Frame::load_local_vreg( int vreg_num ) {
  assert( vreg_num >= 10 );
  assert( vreg_num <= m_callee->get_highest_vreg_num() );
  return m_local_vregs[ vreg_num - 10 ];
}

// Stack class: represents the current call stack

Stack::Stack() {

}

Stack::~Stack() {

}

void Stack::call( uint64_t base, const Function *caller, const Function *callee, unsigned ret_ins_offset ) {
  std::unique_ptr<Frame> frame = std::make_unique<Frame>( base, caller, callee, ret_ins_offset );
  m_stack.emplace_back( std::move( frame ) );
}

void Stack::ret( const Function *&caller, unsigned &ret_ins_offset ) {
  assert( !m_stack.empty() );
  auto &top = m_stack.back();
  caller = top->get_caller();
  ret_ins_offset = top->get_ret_ins_offset();
  m_stack.pop_back();
}

Frame *Stack::get_top() const {
  assert( !m_stack.empty() );
  return m_stack.back().get();
}

unsigned Stack::get_num_frames() const {
  return unsigned( m_stack.size() );
}

Frame *Stack::get_frame( unsigned index ) const {
  assert( index < m_stack.size() );
  return m_stack.at( index ).get();
}

std::string Stack::to_hex( uint64_t val ) {
  return to_hex_s( val );
}
