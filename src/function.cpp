#include <cassert>
#include "exceptions.h"
#include "function.h"

Function::Function( const std::string &name )
  : m_name( name ) {

}

Function::~Function() {

}

const std::string &Function::get_name() const {
  return m_name;
}

void Function::set_label( const std::string &label ) {
  m_labels[ label ] = unsigned( m_instructions.size() );
}

void Function::add_instruction( const Instruction &ins ) {
  m_instructions.push_back( ins );
}

unsigned Function::lookup_label( const std::string &label, const Location &loc ) const {
  auto i = m_labels.find( label );
  if ( i == m_labels.end() )
    throw SimError( "unknown control flow label", loc );
  return i->second;
}

Function::iterator Function::begin() {
  return m_instructions.begin();
}

Function::iterator Function::end() {
  return m_instructions.end();
}

unsigned Function::get_num_instructions() const {
  return unsigned( m_instructions.size() );
}

const Instruction &Function::at( unsigned index ) const {
  assert( index < m_instructions.size() );
  return m_instructions.at( index );
}
