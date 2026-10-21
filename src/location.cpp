#include <cassert>
#include "location.h"

Location::Location()
  : m_line( -1 )
  , m_col( -1 ) {

}

Location::Location( const std::string &filename, int line, int col )
  : m_filename( filename )
  , m_line( line )
  , m_col( col ) {

}

Location::Location( const Location &other )
  : m_filename( other.m_filename )
  , m_line( other.m_line )
  , m_col( other.m_col ) {

}

Location::~Location() {

}

Location &Location::operator=( const Location &rhs ) {
  if ( this != &rhs ) {
    m_filename = rhs.m_filename;
    m_line = rhs.m_line;
    m_col = rhs.m_col;
  }
  return *this;
}

const std::string &Location::get_filename() const {
  return m_filename;
}

int Location::get_line() const {
  return m_line;
}

int Location::get_col() const {
  return m_col;
}

void Location::advance( int c ) {
  assert( c >= 0 );
  if ( c == '\n' ) {
    ++m_line;
    m_col = 1;
  } else
    --m_col;
}

void Location::unadvance( int c ) {
  assert( c >= 0 );
  if ( c == '\n' ) {
    // we don't actually know where the last line ended,
    // but since '\n' is whitespace, it doesn't really
    // matter, since it can't be the start of a token
    --m_line;
    m_col = 999;
  } else
    --m_col;
}
