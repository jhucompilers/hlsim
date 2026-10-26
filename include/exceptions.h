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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include "stacktrace.h"

class SimError : public std::runtime_error {
private:
  StackTrace m_stacktrace;
  bool m_location_only;

public:
  SimError( const std::string &msg, const Location &loc );
  SimError( const std::string &msg, const StackTrace &stacktrace );
  SimError( const SimError &other );
  virtual ~SimError();

  SimError &operator=( const SimError &rhs );

  const Location &get_loc() const;

  bool has_stacktrace() const;
  const StackTrace &get_stacktrace() const;
};

#endif // EXCEPTIONS_H
