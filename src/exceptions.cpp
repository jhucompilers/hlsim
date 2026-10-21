#include "exceptions.h"
#include <stdexcept>

SimError::SimError( const std::string &msg, const Location &loc )
  : std::runtime_error( msg )
  , m_loc( loc ) {

}

SimError::SimError( const SimError &other )
  : std::runtime_error( other )
  , m_loc( other.m_loc ) {

}

SimError::~SimError() {

}

SimError &SimError::operator=( const SimError &rhs ) {
  if ( this != &rhs ) {
    std::runtime_error::operator=( rhs );
    m_loc = rhs.m_loc;
  }
  return *this;
}

const Location &SimError::get_loc() const {
  return m_loc;
}
