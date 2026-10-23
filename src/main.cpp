#include <iostream>
#include <memory>
#include "lexer.h"
#include "parser.h"
#include "exceptions.h"
#include "program.h"
#include "simulator.h"

int execute( int argc, char **argv ) {
  Lexer lexer( std::cin, "<stdin>" );
  Token tok;

  /*
  while ( lexer.peek( tok ) ) {
    std::cout << int( tok.get_kind() ) << ":" << tok.get_lexeme() << "\n";
    lexer.next( tok );
  }
  */

  Program program;
  Parser parser( lexer, program );
  parser.parse();

  // Validate the program
  program.validate();

  Simulator sim( program );
  uint64_t result = sim.execute();

  return int( result );
}

int main( int argc, char **argv ) {
  try {
    return execute( argc, argv );
  } catch ( SimError &ex ) {
    const Location &loc = ex.get_loc();
    if ( loc.is_valid() )
      std::cerr << loc.get_filename()
                << ":"
                << loc.get_line()
                << ":" << loc.get_col()
                << ": ";
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }
}
