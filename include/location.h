#ifndef LOCATION_H
#define LOCATION_H

#include <string>

class Location {
private:
  std::string m_filename;
  int m_line, m_col;

public:
  Location();
  Location( const std::string &filename, int line, int col );
  Location( const Location &other );
  ~Location();

  Location &operator=( const Location &rhs );

  bool is_valid() const;
  const std::string &get_filename() const;
  int get_line() const;
  int get_col() const;

  void advance( int c );
  void unadvance( int c );

  Location operator++( int );
};

#endif // LOCATION_H
