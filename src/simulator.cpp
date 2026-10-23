#include <cassert>
#include "program.h"
#include "location.h"
#include "function.h"
#include "instruction.h"
#include "exceptions.h"
#include "simulator.h"

Simulator::Simulator( const Program &program )
  : m_program( program )
  , m_done( false ) {

}

Simulator::~Simulator() {

}

void Simulator::execute() {
  Location startup_loc( "<start>", 1, 1 );
  std::shared_ptr<Function> main = m_program.lookup_fn( "main", startup_loc );
  const Function *cur_fn = main.get();
  m_stack.call( Block::ADDR_RANGE, nullptr, cur_fn, 0 );

  unsigned pc = 0;

  while ( !m_done ) {
    assert( pc < unsigned( cur_fn->get_num_instructions() ) );
    const Instruction &ins = cur_fn->at( pc );
    pc = execute_ins( ins, pc );
  }
}

unsigned Simulator::execute_ins( const Instruction &ins, unsigned pc ) {
  Frame *cur_frame = m_stack.get_top();
  unsigned next = pc + 1;

  switch ( ins.get_opcode() ) {
  case Opcode::HINS_enter:
    cur_frame->enter( ins.at(0).get_imm(), ins.get_loc() );
    break;

  case Opcode::HINS_leave:
    cur_frame->leave( ins.at(0).get_imm(), ins.get_loc() );
    break;
  
  default:
    throw SimError( "instruction not simulated yet", ins.get_loc() );
  }

  return next;
}
