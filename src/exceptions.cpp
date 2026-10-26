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
#include <stdexcept>
#include "exceptions.h"

SimError::SimError( const std::string &msg, const Location &loc )
  : std::runtime_error( msg )
  , m_location_only( true ) {
  m_stacktrace.append_loc( loc );
}

SimError::SimError( const std::string &msg, const StackTrace &stacktrace )
  : std::runtime_error( msg )
  , m_stacktrace( stacktrace )
  , m_location_only( false ) {
  assert( !stacktrace.empty() );
}

SimError::SimError( const SimError &other )
  : std::runtime_error( other )
  , m_stacktrace( other.m_stacktrace )
  , m_location_only( other.m_location_only ) {

}

SimError::~SimError() {

}

SimError &SimError::operator=( const SimError &rhs ) {
  if ( this != &rhs ) {
    std::runtime_error::operator=( rhs );
    m_stacktrace = rhs.m_stacktrace;
    m_location_only = rhs.m_location_only;
  }
  return *this;
}

const Location &SimError::get_loc() const {
  assert( !m_stacktrace.empty() );
  return *m_stacktrace.begin();
}

bool SimError::has_stacktrace() const {
  return !m_location_only;
}

const StackTrace &SimError::get_stacktrace() const {
  return m_stacktrace;
}
