#include <memory>
#include <string>
#include <sstream>
#include <cassert>
#include <cstring>
#include "exceptions.h"
#include "stack.h"

namespace {

std::string to_hex( uint64_t x ) {
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
  msg += " addr=";
  msg += to_hex( offset + size );
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
  throw SimError( msg, loc );
}

// Frame class: simulates a stack frame

Frame::Frame( uint64_t base, const Function *caller, const Function *callee, unsigned ret_ins_offset )
  : m_base( base )
  , m_caller( caller )
  , m_callee( callee )
  , m_ret_ins_offset( ret_ins_offset ) {

}

Frame::~Frame() {

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
    throw SimError( "multiple enter instructions in one function?", loc );
  m_block = std::make_unique<Block>( m_base, nbytes );
}

void Frame::leave( uint64_t nbytes, const Location &loc ) {
  if ( !m_block.get() )
    throw SimError( "leave instruction without enter instruction", loc );
  if ( m_block->get_size() != nbytes )
    throw SimError( "leave instruction size mismatch with enter", loc );
  m_block.reset( nullptr );
}

Block *Frame::get_block( const Location &loc ) {
  Block *p = m_block.get();
  if ( !p )
    throw SimError( "memory operation in stack frame with no memory block", loc );
  return p;
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

Frame *Stack::get_top() {
  assert( !m_stack.empty() );
  return m_stack.back().get();
}
