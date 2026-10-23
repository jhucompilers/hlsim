#include <cassert>
#include "program.h"
#include "location.h"
#include "function.h"
#include "instruction.h"
#include "exceptions.h"
#include "simulator.h"

namespace {

// Operand sizes in bytes, corresponding to the b/w/l/q forms.
// Instruction opcodes are always ordered in exactly this way,
// i.e., b variant is first, w variant is second, etc.
const unsigned OPERAND_SIZES[4] = { 1, 2, 4, 8 };

// Determine the operand size of an opcode by comparing it to the
// "base" opcode, which is the "b" variant.
unsigned get_operand_size( Opcode opcode, Opcode base_opcode ) {
  int offset = int( opcode ) - int( base_opcode );
  assert( offset >= 0 );
  assert( offset < 4 );
  return OPERAND_SIZES[ offset ];
}

};

Simulator::Simulator( const Program &program )
  : m_program( program )
  , m_done( false ) {
  for ( int i = 0; i < 10; ++i )
    m_global_vregs[i] = uint64_t(0) - 1U;
}

Simulator::~Simulator() {

}

uint64_t Simulator::execute() {
  Location startup_loc( "<start>", 1, 1 );
  std::shared_ptr<Function> main = m_program.lookup_fn( "main", startup_loc );
  m_cur_fn = main.get();
  m_stack.call( Block::ADDR_RANGE, nullptr, m_cur_fn, 0 );

  unsigned pc = 0;

  while ( !m_done ) {
    assert( pc < unsigned( m_cur_fn->get_num_instructions() ) );
    const Instruction &ins = m_cur_fn->at( pc );
    pc = execute_ins( ins, pc );
  }

  // Whatever is in vr0 when main returns is the result
  // of the program
  return m_global_vregs[0];
}

unsigned Simulator::execute_ins( const Instruction &ins, unsigned pc ) {
  unsigned next = pc + 1;

  switch ( ins.get_opcode() ) {
  case Opcode::HINS_enter:
    m_stack.get_top()->enter( ins.at(0).get_imm(), ins.get_loc() );
    break;

  case Opcode::HINS_leave:
    m_stack.get_top()->leave( ins.at(0).get_imm(), ins.get_loc() );
    break;
  
  case Opcode::HINS_ret:
    m_stack.ret( m_cur_fn, next );
    if ( m_cur_fn == nullptr )
      m_done = true;
    break;

  case Opcode::HINS_mov_b:
  case Opcode::HINS_mov_w:
  case Opcode::HINS_mov_l:
  case Opcode::HINS_mov_q:
    execute_mov( ins );
    break;
  
  default:
    throw SimError( "instruction not simulated yet", ins.get_loc() );
  }

  return next;
}

void Simulator::execute_mov( const Instruction &ins ) {
  unsigned operand_size = get_operand_size( ins.get_opcode(), Opcode::HINS_mov_b );
  Frame *frame = m_stack.get_top();
  uint64_t val = load_source( frame, ins.at( 1 ), operand_size, ins.get_loc() );
  store_dest( frame, ins.at( 0 ), operand_size, val, ins.get_loc() );
}

uint64_t Simulator::load_source( Frame *frame, const Operand &operand, unsigned operand_size, const Location &loc ) {
  uint64_t val;
  switch ( operand.get_kind() ) {
  case OperandKind::VREG:
    val = trim( read_vreg( frame, operand.get_vreg() ), operand_size );
    break;
  case OperandKind::VREG_MEMREF:
    val = trim( read_mem( read_vreg( frame, operand.get_vreg() ), operand_size, loc ), operand_size );
    break;
  case OperandKind::IMM:
    val = uint64_t( operand.get_imm() );
    break;
  default:
    assert( false );
    return uint64_t(0) - 1U;
  }

  return val;
}

void Simulator::store_dest( Frame *frame, const Operand &operand, unsigned operand_size, uint64_t val, const Location &loc ) {
  switch ( operand.get_kind() ) {
  case OperandKind::VREG:
    write_vreg( frame, operand.get_vreg(), trim( val, operand_size ) );
    break;

  case OperandKind::VREG_MEMREF:
    throw SimError( "writes to memory not supported yet", loc );

  default:
    assert( false );
  }
}

uint64_t Simulator::read_vreg( Frame *frame, int vreg_num ) const {
  assert( vreg_num >= 0 );
  if ( vreg_num < 10 )
    return m_global_vregs[ vreg_num ];
  else
    return frame->load_local_vreg( vreg_num );
}

void Simulator::write_vreg( Frame *frame, int vreg_num, uint64_t val ) {
  assert( vreg_num >= 0 );
  if ( vreg_num < 10 )
    m_global_vregs[ vreg_num ] = val;
  else
    frame->store_local_vreg( vreg_num, val );
}

uint64_t Simulator::read_mem( uint64_t addr, unsigned operand_size, const Location &loc ) {
  // Determine which stack frame should contain the memory location
  // named by the address.
  uint64_t frame_num = ( addr >> Block::ADDR_RANGE_SHIFT );
  if ( frame_num == 0U )
    throw SimError( "read of invalid address " + Stack::to_hex( addr ) + " (null pointer deref?)", loc );
   // frame number "1" corresponds to the stack frame for the
   // main function (frame number 0 is not used)
  --frame_num;

  if ( frame_num >= m_stack.get_num_frames() )
    throw SimError( "read of invalid address " + Stack::to_hex( addr ), loc );

  Frame *frame = m_stack.get_frame( frame_num );

  uint64_t val;
  switch ( operand_size ) {
  case 1:
    val = frame->get_block( loc )->loadb( addr, loc );
    break;
  case 2:
    val = frame->get_block( loc )->loadw( addr, loc );
    break;
  case 4:
    val = frame->get_block( loc )->loadl( addr, loc );
    break;
  case 8:
    val = frame->get_block( loc )->loadq( addr, loc );
    break;
  default:
    assert( false );
    val = uint64_t(0) - 1U;
  }

  return val;
}

uint64_t Simulator::trim( uint64_t val, unsigned operand_size ) const {
  switch ( operand_size ) {
  case 1:
    return val & 0xFF;
  case 2:
    return val & 0xFFFF;
  case 4:
    return val & 0xFFFFFFFF;
  case 8:
    return val;
  default:
    assert( false );
    return uint64_t(0) - 1U;
  }
}
