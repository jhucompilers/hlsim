#ifndef OPERAND_H
#define OPERAND_H

#include <string>

enum class OperandKind {
  INVALID,
  VREG,
  VREG_MEMREF,
  IMM,
  LABEL,
  // TODO: memrefs with fancier addressing modes?
};

class Operand {
private:
  OperandKind m_kind;
  long m_ival; // register number or immediate value
  std::string m_label;

public:
  Operand( OperandKind kind = OperandKind::INVALID, long ival = 0 );
  Operand( const std::string &label );
  Operand( const Operand &other );
  ~Operand();

  Operand &operator=( const Operand &rhs );

  OperandKind get_kind() const;
  bool has_vreg() const;
  int get_vreg() const;
  long get_imm() const;
  bool is_memref() const;
  std::string get_label() const;
};

#endif // OPERAND_H
