#include <iostream>
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

// Sign-extend a signed value of given size (with bits stored in a uint64_t)
// to int64_t
int64_t sign_extend( uint64_t val, unsigned operand_size ) {
  switch ( operand_size ) {
  case 1:
    return int8_t( val & 0xFF );
  case 2:
    return int16_t( val & 0xFFFF );
  case 4:
    return int32_t( val & 0xFFFFFFFF );
  case 8:
    return int64_t( val );
  default:
    assert( false );
    return -1;
  }
}

};

Simulator::Simulator( const Program &program )
  : m_program( program )
  , m_done( false ) {
  // Initialize global vregs
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
    if ( pc >= m_cur_fn->get_num_instructions() )
      throw SimError( "pc is past end of instructions in function " + m_cur_fn->get_name(), m_cur_fn->get_end_loc() );
    const Instruction &ins = m_cur_fn->at( pc );
    pc = execute_ins( ins, pc );
  }

  // Whatever is in vr0 when main returns is the result
  // of the program
  return m_global_vregs[0];
}

#define HANDLE_BINOP_CHECKRIGHT(op, base_opcode, checkright) \
    do { \
      unsigned operand_size = get_operand_size( ins.get_opcode(), base_opcode ); \
      Frame *cur_frame = m_stack.get_top(); \
      uint64_t left, right, result; \
      left = load_source( cur_frame, ins.at(1), operand_size, ins.get_loc() ); \
      right = load_source( cur_frame, ins.at(2), operand_size, ins.get_loc() ); \
      checkright; \
      result = left op right; \
      store_dest( cur_frame, ins.at(0), operand_size, result, ins.get_loc() ); \
    } while ( 0 )

#define HANDLE_BINOP(op, base_opcode) HANDLE_BINOP_CHECKRIGHT(op, base_opcode, (void) right)

#define HANDLE_UNOP(xform, base_opcode) \
    do { \
      unsigned operand_size = get_operand_size( ins.get_opcode(), base_opcode ); \
      Frame *cur_frame = m_stack.get_top(); \
      uint64_t val = load_source( cur_frame, ins.at(1), operand_size, ins.get_loc() ); \
      xform; \
      store_dest( cur_frame, ins.at(0), operand_size, val, ins.get_loc() ); \
    } while ( 0 )

unsigned Simulator::execute_ins( const Instruction &ins, unsigned pc ) {
  unsigned next = pc + 1;

  switch ( ins.get_opcode() ) {
  case Opcode::HINS_nop:
    // do nothing
    break;

  case Opcode::HINS_enter:
    m_stack.get_top()->enter( ins.at(0).get_imm(), ins.get_loc() );
    break;

  case Opcode::HINS_leave:
    m_stack.get_top()->leave( ins.at(0).get_imm(), ins.get_loc() );
    break;

  case Opcode::HINS_call:
    {
      // Look up called function
      const Function *callee = m_program.lookup_fn( ins.at(0).get_label(), ins.get_loc() ).get();

      // Compute base address for the called function's memory block
      uint64_t base = ( m_stack.get_num_frames() + 1 ) * Block::ADDR_RANGE;

      // Start function call
      m_stack.call( base, m_cur_fn, callee, next );
      m_cur_fn = callee;
      next = 0;
    }
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

  case Opcode::HINS_sconv_bw:
    execute_sconv( ins, 1, 2 );
    break;

  case Opcode::HINS_sconv_bl:
    execute_sconv( ins, 1, 4 );
    break;

  case Opcode::HINS_sconv_bq:
    execute_sconv( ins, 1, 8 );
    break;

  case Opcode::HINS_sconv_wl:
    execute_sconv( ins, 2, 4 );
    break;

  case Opcode::HINS_sconv_wq:
    execute_sconv( ins, 2, 8 );
    break;

  case Opcode::HINS_sconv_lq:
    execute_sconv( ins, 4, 8 );
    break;

  case Opcode::HINS_uconv_bw:
    execute_uconv( ins, 1, 2 );
    break;

  case Opcode::HINS_uconv_bl:
    execute_uconv( ins, 1, 4 );
    break;

  case Opcode::HINS_uconv_bq:
    execute_uconv( ins, 1, 8 );
    break;

  case Opcode::HINS_uconv_wl:
    execute_uconv( ins, 2, 4 );
    break;

  case Opcode::HINS_uconv_wq:
    execute_uconv( ins, 2, 8 );
    break;

  case Opcode::HINS_uconv_lq:
    execute_uconv( ins, 4, 8 );
    break;
  
  case Opcode::HINS_localaddr:
    {
      Frame *cur_frame = m_stack.get_top();
      uint64_t offset = uint64_t( ins.at(1).get_imm() ); // localaddr offsets are always positive
      uint64_t addr = cur_frame->get_base() + offset;
      write_vreg( cur_frame, ins.at(0).get_vreg(), addr );
    }
    break;
  
  case Opcode::HINS_add_b:
  case Opcode::HINS_add_w:
  case Opcode::HINS_add_l:
  case Opcode::HINS_add_q:
    HANDLE_BINOP( +, Opcode::HINS_add_b );
    break;

  case Opcode::HINS_sub_b:
  case Opcode::HINS_sub_w:
  case Opcode::HINS_sub_l:
  case Opcode::HINS_sub_q:
    HANDLE_BINOP( -, Opcode::HINS_sub_b );
    break;

  case Opcode::HINS_mul_b:
  case Opcode::HINS_mul_w:
  case Opcode::HINS_mul_l:
  case Opcode::HINS_mul_q:
    HANDLE_BINOP( *, Opcode::HINS_mul_b );
    break;

  case Opcode::HINS_div_b:
  case Opcode::HINS_div_w:
  case Opcode::HINS_div_l:
  case Opcode::HINS_div_q:
    HANDLE_BINOP_CHECKRIGHT( /, Opcode::HINS_div_b, if ( right == 0 ) throw SimError( "division by 0", ins.get_loc() ) );
    break;

  case Opcode::HINS_mod_b:
  case Opcode::HINS_mod_w:
  case Opcode::HINS_mod_l:
  case Opcode::HINS_mod_q:
    HANDLE_BINOP_CHECKRIGHT( %, Opcode::HINS_mod_b, if ( right == 0 ) throw SimError( "mod by 0", ins.get_loc() ) );
    break;

  case Opcode::HINS_jmp:
    next = ins.at(0).get_target_offset();
    break;

  case Opcode::HINS_cmplt_b:
  case Opcode::HINS_cmplt_w:
  case Opcode::HINS_cmplt_l:
  case Opcode::HINS_cmplt_q:
    execute_scmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_cmplt_b ), Compare::LT );
    break;

  case Opcode::HINS_cmplte_b:
  case Opcode::HINS_cmplte_w:
  case Opcode::HINS_cmplte_l:
  case Opcode::HINS_cmplte_q:
    execute_scmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_cmplte_b ), Compare::LTE );
    break;
    
  case Opcode::HINS_cmpgt_b:
  case Opcode::HINS_cmpgt_w:
  case Opcode::HINS_cmpgt_l:
  case Opcode::HINS_cmpgt_q:
    execute_scmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_cmpgt_b ), Compare::GT );
    break;

  case Opcode::HINS_cmpgte_b:
  case Opcode::HINS_cmpgte_w:
  case Opcode::HINS_cmpgte_l:
  case Opcode::HINS_cmpgte_q:
    execute_scmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_cmpgte_b ), Compare::GTE );
    break;

  case Opcode::HINS_ucmplt_b:
  case Opcode::HINS_ucmplt_w:
  case Opcode::HINS_ucmplt_l:
  case Opcode::HINS_ucmplt_q:
    execute_ucmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_ucmplt_b ), Compare::LT );
    break;

  case Opcode::HINS_ucmplte_b:
  case Opcode::HINS_ucmplte_w:
  case Opcode::HINS_ucmplte_l:
  case Opcode::HINS_ucmplte_q:
    execute_ucmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_ucmplte_b ), Compare::LTE );
    break;

  case Opcode::HINS_ucmpgt_b:
  case Opcode::HINS_ucmpgt_w:
  case Opcode::HINS_ucmpgt_l:
  case Opcode::HINS_ucmpgt_q:
    execute_ucmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_ucmpgt_b ), Compare::GT );
    break;

  case Opcode::HINS_ucmpgte_b:
  case Opcode::HINS_ucmpgte_w:
  case Opcode::HINS_ucmpgte_l:
  case Opcode::HINS_ucmpgte_q:
    execute_ucmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_ucmpgte_b ), Compare::GTE );
    break;

  case Opcode::HINS_cmpeq_b:
  case Opcode::HINS_cmpeq_w:
  case Opcode::HINS_cmpeq_l:
  case Opcode::HINS_cmpeq_q:
    execute_ucmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_cmpeq_b ), Compare::EQ );
    break;

  case Opcode::HINS_cmpneq_b:
  case Opcode::HINS_cmpneq_w:
  case Opcode::HINS_cmpneq_l:
  case Opcode::HINS_cmpneq_q:
    execute_ucmp( ins, get_operand_size( ins.get_opcode(), Opcode::HINS_cmpneq_b ), Compare::NEQ );
    break;

  case Opcode::HINS_neg_b:
  case Opcode::HINS_neg_w:
  case Opcode::HINS_neg_l:
  case Opcode::HINS_neg_q:
    HANDLE_UNOP(val = ~val + 1, Opcode::HINS_neg_b);
    break;

  case Opcode::HINS_not_b:
  case Opcode::HINS_not_w:
  case Opcode::HINS_not_l:
  case Opcode::HINS_not_q:
    HANDLE_UNOP(val = (val == 0), Opcode::HINS_not_b);
    break;

  /*
  case Opcode::HINS_compl_b:
  case Opcode::HINS_compl_w:
  case Opcode::HINS_compl_l:
  case Opcode::HINS_compl_q:

  case Opcode::HINS_inc_b:
  case Opcode::HINS_inc_w:
  case Opcode::HINS_inc_l:
  case Opcode::HINS_inc_q:
  case Opcode::HINS_dec_b:
  case Opcode::HINS_dec_w:
  case Opcode::HINS_dec_l:
  case Opcode::HINS_dec_q:
  */

  case Opcode::HINS_cjmp_t:
  case Opcode::HINS_cjmp_f:
    {
      Frame *cur_frame = m_stack.get_top();
      uint64_t boolval = load_source( cur_frame, ins.at(0), 1, ins.get_loc() );
      bool take_branch = ( ins.get_opcode() == Opcode::HINS_cjmp_t ) ? bool(boolval) : !bool(boolval);
      if ( take_branch )
        next = ins.at(1).get_target_offset();
    }
    break;

  case Opcode::HINS_read_i32:
    {
      // Read one 32-bit signed integer value, and store it in vr0
      int32_t val;
      std::cin >> val;
      m_global_vregs[0] = uint32_t( val );
    }
    break;

  case Opcode::HINS_print_str:
    {
      // Print the string pointed to by the pointer value in vr1.

      // Note: we assume that this intrinsic will only every be used
      // to print string literals. If we wanted to allow printing
      // of strings created dynamically in program memory, we would
      // need to to handle addresses resolving to stack memory.

      uint64_t str_ptr = m_global_vregs[1];
      const char *s = m_program.get_str_lit_ptr( str_ptr, ins.get_loc() );
      std::cout << s;
    }
    break;

  case Opcode::HINS_print_nl:
    std::cout << "\n";
    break;

  case Opcode::HINS_print_i32:
    {
      // Print signed 32 bit integer in vr1.
      int32_t val = int32_t( m_global_vregs[1] );
      std::cout << val;
    }
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

void Simulator::execute_sconv( const Instruction &ins, unsigned src_operand_size, unsigned dest_operand_size) {
  Frame *cur_frame = m_stack.get_top();
  uint64_t val = load_source( cur_frame, ins.at(1), src_operand_size, ins.get_loc() );
  int64_t sval = sign_extend( val, src_operand_size );
  store_dest( cur_frame, ins.at(0), dest_operand_size, uint64_t( sval ), ins.get_loc() );
}

void Simulator::execute_uconv( const Instruction &ins, unsigned src_operand_size, unsigned dest_operand_size ) {
  Frame *cur_frame = m_stack.get_top();
  uint64_t val = load_source( cur_frame, ins.at(1), src_operand_size, ins.get_loc() );
  store_dest( cur_frame, ins.at(0), dest_operand_size, val, ins.get_loc() );
}

void Simulator::execute_scmp( const Instruction &ins, unsigned operand_size, Compare cmp ) {
  Frame *cur_frame = m_stack.get_top();

  // Get left and right operand values
  uint64_t uleft, uright;
  uleft = load_source( cur_frame, ins.at(1), operand_size, ins.get_loc() );
  uright = load_source( cur_frame, ins.at(2), operand_size, ins.get_loc() );

  // Convert to signed values, sign-extending to 64 bits as appropriate
  int64_t left = sign_extend( uleft, operand_size ), right = sign_extend( uright, operand_size );

  // Do the comparison
  uint64_t result;
  switch ( cmp ) {
  case Compare::LT:
    result = ( left < right );
    break;
  case Compare::LTE:
    result = ( left <= right );
    break;
  case Compare::GT:
    result = ( left > right );
    break;
  case Compare::GTE:
    result = ( left >= right );
    break;
  case Compare::EQ:
    result = ( left == right );
    break;
  case Compare::NEQ:
    result = ( left != right );
    break;
  }

  // Store result
  store_dest( cur_frame, ins.at(0), 8, result, ins.get_loc() );
}

void Simulator::execute_ucmp( const Instruction &ins, unsigned operand_size, Compare cmp ) {
  Frame *cur_frame = m_stack.get_top();

  // Get left and right operand values
  uint64_t uleft, uright;
  uleft = load_source( cur_frame, ins.at(1), operand_size, ins.get_loc() );
  uright = load_source( cur_frame, ins.at(2), operand_size, ins.get_loc() );

  // Compare values
  uint64_t result;
  switch ( cmp ) {
  case Compare::LT:
    result = ( uleft < uright );
    break;
  case Compare::LTE:
    result = ( uleft <= uright );
    break;
  case Compare::GT:
    result = ( uleft > uright );
    break;
  case Compare::GTE:
    result = ( uleft >= uright );
    break;
  case Compare::EQ:
    result = ( uleft == uright );
    break;
  case Compare::NEQ:
    result = ( uleft != uright );
    break;
  }

  // Store result
  store_dest( cur_frame, ins.at(0), 8, result, ins.get_loc() );
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
  case OperandKind::IMM_LABEL:
    // This must refer to a string constant
    assert( operand.get_str_lit_addr() >= Block::ADDR_RANGE/2 );
    val = operand.get_str_lit_addr();
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
    write_mem( read_vreg( frame, operand.get_vreg() ), operand_size, val, loc );
    break;
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
  Frame *frame = determine_frame( addr, "read", loc );
  Block *block = frame->get_block( loc );
  uint64_t val;
  switch ( operand_size ) {
  case 1:
    val = block->loadb( addr, loc );
    break;
  case 2:
    val = block->loadw( addr, loc );
    break;
  case 4:
    val = block->loadl( addr, loc );
    break;
  case 8:
    val = block->loadq( addr, loc );
    break;
  default:
    assert( false );
    val = uint64_t(0) - 1U;
  }

  return val;
}

void Simulator::write_mem( uint64_t addr, unsigned operand_size, uint64_t val, const Location &loc ) {
  Frame *frame = determine_frame( addr, "write", loc );
  Block *block = frame->get_block( loc );
  switch ( operand_size ) {
  case 1:
    block->storeb( addr, uint8_t( val ), loc );
    break;
  case 2:
    block->storew( addr, uint16_t( val ), loc );
    break;
  case 4:
    block->storel( addr, uint32_t( val ), loc );
    break;
  case 8:
    block->storeq( addr, uint64_t( val ), loc );
    break;
  default:
    assert( false );
  }
}

// Determine which stack frame should contain the memory location
// named by the address.
Frame *Simulator::determine_frame( uint64_t addr, const char *op, const Location &loc ) {
  uint64_t frame_num = ( addr >> Block::ADDR_RANGE_SHIFT );
  if ( frame_num == 0U )
    throw SimError( std::string( op ) + " of invalid address " + Stack::to_hex( addr ) + " (null pointer deref?)", loc );
   // frame number "1" corresponds to the stack frame for the
   // main function (frame number 0 is not used)
  --frame_num;

  if ( frame_num >= m_stack.get_num_frames() )
    throw SimError( std::string( op ) + " of invalid address " + Stack::to_hex( addr ), loc );

  return m_stack.get_frame( frame_num );
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
