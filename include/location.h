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

#ifndef LOCATION_H
#define LOCATION_H

#include <string>

class Location {
private:
  std::string m_filename;
  int m_line, m_col;
  std::string m_fn_name;

public:
  Location();
  Location( const std::string &filename, int line, int col, const std::string &fn_name = "" );
  Location( const Location &other );
  ~Location();

  Location &operator=( const Location &rhs );

  bool is_valid() const;
  const std::string &get_filename() const;
  int get_line() const;
  int get_col() const;

  bool has_fn_name() const;
  const std::string &get_fn_name() const;

  void advance( int c );
  void unadvance( int c );

  Location operator++( int );
};

#endif // LOCATION_H
