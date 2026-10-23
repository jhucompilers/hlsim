#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <memory>

class Function;

class Program {
private:
  std::vector<std::shared_ptr<Function>> m_functions;

  // no value semantics
  Program( const Program & ) = delete;
  Program &operator=( const Program & ) = delete;

public:
  Program();
  ~Program();

  void add_function( std::shared_ptr<Function> fn );
};

#endif // PROGRAM_H
