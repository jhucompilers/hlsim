#include <cassert>
#include "operand.h"

Operand::Operand( OperandKind kind, int64_t ival )
  : m_kind( kind )
  , m_ival( ival ) {
  assert( kind != OperandKind::LABEL );
}

Operand::Operand( OperandKind kind, const std::string &label )
  : m_kind( kind )
  , m_ival( 0 )
  , m_label( label ) {
  assert( kind == OperandKind::LABEL || kind == OperandKind::IMM_LABEL );
}

Operand::Operand( const Operand &other )
  : m_kind( other.m_kind )
  , m_ival( other.m_ival )
  , m_label( other.m_label ) {

}

Operand::~Operand() {

}

Operand &Operand::operator=( const Operand &rhs ) {
  if ( this != &rhs ) {
    m_kind = rhs.m_kind;
    m_ival = rhs.m_ival;
    m_label = rhs.m_label;
  }
  return *this;
}

OperandKind Operand::get_kind() const {
  return m_kind;
}

bool Operand::has_vreg() const {
  return m_kind == OperandKind::VREG || m_kind == OperandKind::VREG_MEMREF;
}

int Operand::get_vreg() const {
  assert( has_vreg() );
  return int( m_ival );
}

int64_t Operand::get_imm() const {
  assert( m_kind == OperandKind::IMM );
  return m_ival;
}

bool Operand::is_memref() const {
  return m_kind == OperandKind::VREG_MEMREF;
}

std::string Operand::get_label() const {
  assert( m_kind == OperandKind::LABEL );
  return m_label;
}

bool Operand::is_assignable() const {
  return m_kind == OperandKind::VREG || m_kind == OperandKind::VREG_MEMREF;
}

bool Operand::is_readable() const {
  return is_assignable() || m_kind == OperandKind::IMM || m_kind == OperandKind::IMM_LABEL;
}

void Operand::set_target_offset( unsigned offset ) {
  assert( m_kind == OperandKind::LABEL );
  m_ival = offset;
}

unsigned Operand::get_target_offset() const {
  assert( m_ival >= 0 );
  return unsigned( m_ival );
}
