#ifndef STACK_H
#define STACK_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
class Location;
class Function;

// A block of memory allocated for storage of memory variables
// in a stack frame.
class Block {
public:
  // Number of bytes in the address range for the memory block
  // of one stack frame. This is, therefore, the maximum amount
  // of memory that can be allocated.
  static constexpr uint64_t ADDR_RANGE = (32UL*1024UL*1024UL);

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

  void storeb( uint64_t addr, uint8_t val, const Location &loc );
  void storew( uint64_t addr, uint16_t val, const Location &loc );
  void storel( uint64_t addr, uint32_t val, const Location &loc );
  void storeq( uint64_t addr, uint64_t val, const Location &loc );

  uint8_t loadb( uint64_t addr, const Location &loc );
  uint16_t loadw( uint64_t addr, const Location &loc );
  uint32_t loadl( uint64_t addr, const Location &loc );
  uint64_t loadq( uint64_t addr, const Location &loc );

private:
  uint64_t bounds_and_alignment_check( const char *op, uint64_t addr, uint64_t size, const Location &loc );
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

  const Function *get_caller() const;
  const Function *get_callee() const;
  unsigned get_ret_ins_offset() const;

  void enter( uint64_t nbytes, const Location &loc );
  void leave( uint64_t nbytes, const Location &loc );

  Block *get_block( const Location &loc );

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
  Frame *get_top();
};

#endif // STACK_H
