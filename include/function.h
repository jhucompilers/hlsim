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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <unordered_map>
#include <string>
#include "instruction.h"
#include "location.h"
class Location;

class Function {
public:
  typedef std::vector< Instruction > InsList;

private:
  std::string m_name;
  InsList m_instructions;
  // map of label names to instruction indices
  std::unordered_map< std::string, unsigned > m_labels;
  int m_highest_vreg_num;
  Location m_end_loc;

  // No value semantics
  Function( const Function & ) = delete;
  Function &operator=( const Function & ) = delete;

public:
  Function( const std::string &name );
  ~Function();

  const std::string &get_name() const;

  void set_label( const std::string &label );
  void add_instruction( const Instruction &ins );

  unsigned lookup_label( const std::string &label, const Location &loc ) const;

  // Support for iterating over the instructions
  typedef InsList::iterator iterator;
  iterator begin();
  iterator end();

  unsigned get_num_instructions() const;
  const Instruction &at( unsigned index ) const;

  int get_highest_vreg_num() const;

  //void set_end_loc( const Location &loc );
  const Location &get_end_loc() const;
};

#endif // FUNCTION_H
