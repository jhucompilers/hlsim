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
