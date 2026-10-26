//  hlsim - simulate nearly_cc high-level intermediate code
//  Copyright (C) 2026, David H. Hovemeyer <david.hovemeyer@gmail.com>

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <cassert>
#include "location.h"

Location::Location()
  : m_line( -1 )
  , m_col( -1 ) {

}

Location::Location( const std::string &filename, int line, int col, const std::string &fn_name )
  : m_filename( filename )
  , m_line( line )
  , m_col( col )
  , m_fn_name( fn_name ) {

}

Location::Location( const Location &other )
  : m_filename( other.m_filename )
  , m_line( other.m_line )
  , m_col( other.m_col )
  , m_fn_name( other.m_fn_name ) {

}

Location::~Location() {

}

Location &Location::operator=( const Location &rhs ) {
  if ( this != &rhs ) {
    m_filename = rhs.m_filename;
    m_line = rhs.m_line;
    m_col = rhs.m_col;
    m_fn_name = rhs.m_fn_name;
  }
  return *this;
}

bool Location::is_valid() const {
  return m_line >= 1;
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

bool Location::has_fn_name() const {
  return !m_fn_name.empty();
}

const std::string &Location::get_fn_name() const {
  assert( has_fn_name() );
  return m_fn_name;
}

void Location::advance( int c ) {
  assert( c >= 0 );
  if ( c == '\n' ) {
    ++m_line;
    m_col = 1;
  } else
    ++m_col;
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

// Advance by one line, and return the original Location.
// This is useful for generating fake Locations
// (e.g., for the generated implementations of intrinsic functions.)
Location Location::operator++( int ) {
  Location result = *this;
  advance( '\n' );
  return result;
}
