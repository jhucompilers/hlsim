#ifndef OPERAND_H
#define OPERAND_H

#include <cstdint>
#include <string>

enum class OperandKind {
  INVALID,
  VREG,
  VREG_MEMREF,
  IMM,
  LABEL,
  IMM_LABEL, // immediate label (used only for string constants)
  // TODO: memrefs with fancier addressing modes?
};

class Operand {
private:
  OperandKind m_kind;
  int64_t m_ival; // register number or immediate value
  std::string m_label;

public:
  Operand( OperandKind kind = OperandKind::INVALID, int64_t ival = 0 );
  Operand( OperandKind kind, const std::string &label );
  Operand( const Operand &other );
  ~Operand();

  Operand &operator=( const Operand &rhs );

  OperandKind get_kind() const;
  bool has_vreg() const;
  int get_vreg() const;
  int64_t get_imm() const;
  bool is_memref() const;
  std::string get_label() const;

  bool is_assignable() const;
  bool is_readable() const;

  // Set the instruction offset of the instruction targeted by a
  // control flow label.
  void set_target_offset( unsigned offset );

  // Get the instruction offset of the instruction targeted by a
  // label.
  unsigned get_target_offset() const;
};

#endif // OPERAND_H
