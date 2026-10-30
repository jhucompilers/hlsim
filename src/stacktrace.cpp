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

#include "stacktrace.h"
#include <string>

StackTrace::StackTrace() {

}

StackTrace::StackTrace( const StackTrace &other )
  : m_items( other.m_items ) {

}

StackTrace::~StackTrace() {

}

StackTrace &StackTrace::operator=( const StackTrace &rhs ) {
  if ( this != &rhs ) {
    m_items = rhs.m_items;
  }
  return *this;
}

void StackTrace::append_loc( const Location &loc ) {
  m_items.push_back( loc );
}

bool StackTrace::empty() const {
  return m_items.empty();
}

StackTrace::const_iterator StackTrace::begin() const {
  return m_items.begin();
}

StackTrace::const_iterator StackTrace::end() const {
  return m_items.end();
}

void StackTrace::print( std::ostream &out ) const {
  for ( auto i = begin(); i != end(); ++i ) {
    const Location &loc = *i;
    out << "    ";
    if ( loc.has_fn_name() )
      out << "in " << loc.get_fn_name() << " ";
    out << "(" << loc.get_filename() << ":" << loc.get_line() << ")\n";
  }
}

ThrowSimErrorWithStackTrace::ThrowSimErrorWithStackTrace() {

}

ThrowSimErrorWithStackTrace::~ThrowSimErrorWithStackTrace() {

}
