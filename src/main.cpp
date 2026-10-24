#include <iostream>
#include <fstream>
#include <stdexcept>
#include "lexer.h"
#include "parser.h"
#include "exceptions.h"
#include "program.h"
#include "simulator.h"

int execute( int argc, char **argv ) {
  if ( argc != 2 )
    throw std::runtime_error( "Usage: hlsim <filename>" );

  const std::string filename = argv[1];

  std::ifstream in( filename );
  if ( !in.good() )
    throw std::runtime_error( "Couldn't open '" + filename + "'" );

  // Create lexer
  Lexer lexer( in, filename );
  Token tok;

  // Create Parser, parse input to create Program
  Program program;
  Parser parser( lexer, program );
  parser.parse();

  // We can close the input now
  in.close();

  // Add implementations of intrinsic (runtime) functions
  program.add_intrinsics();

  // Validate the program
  program.validate();

  // Simulate the program
  Simulator sim( program );
  uint64_t result = sim.execute();

  // Use simulated program result as exit code for simulator
  return int( result );
}

void print_error_message( const std::string &msg, const Location &loc = Location() ) {
  if ( loc.is_valid() )
    std::cerr << loc.get_filename()
              << ":"
              << loc.get_line()
              << ":" << loc.get_col()
              << ": ";
  std::cerr << "Error: " << msg << "\n";
}

int main( int argc, char **argv ) {
  // Note: we use an unusual exit code (127) when the
  // simulator exits due to an exception, since many of the
  // test programs use a non-zero exit code as a way of
  // reporting the result of the computation.

  try {
    return execute( argc, argv );
  } catch ( SimError &ex ) {
    print_error_message( ex.what(), ex.get_loc() );
    return 127;
  } catch ( std::exception &ex ) {
    print_error_message( ex.what() );
    return 127;
  }
}
