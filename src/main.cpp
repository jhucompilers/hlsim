#include <iostream>
#include "lexer.h"
#include "parser.h"

int main( int argc, char **argv ) {
  Lexer lexer( std::cin );
  Token tok;

  /*
  while ( lexer.peek( tok ) ) {
    std::cout << int( tok.get_kind() ) << ":" << tok.get_lexeme() << "\n";
    lexer.next( tok );
  }
  */

  Parser parser( lexer );
  parser.parse();
  std::cout << "Everybody is good!\n";

  return 0;
}
