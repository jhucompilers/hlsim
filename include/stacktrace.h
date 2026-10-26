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

#ifndef STACKTRACE_H
#define STACKTRACE_H

#include <vector>
#include "location.h"

class StackTrace {
private:
  // First item is the "top" of the trace, i.e.,
  // the innermost point
  std::vector<Location> m_items;

public:
  StackTrace();
  StackTrace( const StackTrace &other );
  ~StackTrace();

  StackTrace &operator=( const StackTrace &rhs );

  // This should be called to add locations starting
  // with the innermost one and ending with the outermost one.
  void append_loc( const Location &loc );

  bool empty() const;

  typedef std::vector<Location>::const_iterator const_iterator;

  const_iterator begin() const;
  const_iterator end() const;
};

#endif // STACKTRACE_H
