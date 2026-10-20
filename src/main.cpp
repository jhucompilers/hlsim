#include <iostream>
#include "lexer.h"

int main( int argc, char **argv ) {
  Lexer lexer( std::cin );
  Token tok;

  while ( lexer.peek( tok ) ) {
    std::cout << int( tok.get_kind() ) << ":" << tok.get_lexeme() << "\n";
    lexer.next( tok );
  }

  return 0;
}
