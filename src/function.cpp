#include "function.h"

Function::Function( const std::string &name )
  : m_name( name ) {

}

Function::~Function() {

}

void Function::add_instruction( const Instruction &ins ) {
  m_instructions.push_back( ins );
}

Function::const_iterator Function::begin() const {
  return m_instructions.begin();
}

Function::const_iterator Function::end() const {
  return m_instructions.end();
}
