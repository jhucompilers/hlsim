#include <cstdint>
#include <cassert>
#include <cctype>
#include "lexer.h"
#include "opcode.h"
#include "exceptions.h"

Token::Token( TokenKind kind, const Location &loc, const std::string &lexeme )
  : m_kind( kind )
  , m_lexeme( lexeme )
  , m_loc( loc ) {

}

Token::Token( const Token &other )
  : m_kind( other.m_kind )
  , m_lexeme( other.m_lexeme )
  , m_loc( other.m_loc ) {

}

Token::~Token() {

}

Token &Token::operator=( const Token &rhs ) {
  if ( this != &rhs ) {
    m_kind = rhs.m_kind;
    m_lexeme = rhs.m_lexeme;
    m_loc = rhs.m_loc;
  }
  return *this;
}

TokenKind Token::get_kind() const {
  return m_kind;
}

const std::string &Token::get_lexeme() const {
  return m_lexeme;
}

const Location &Token::get_loc() const {
  return m_loc;
}

std::string Token::get_string_literal_value() const {
  std::string str_value;

  // Iterate over all of the characters in the string except
  // the beginning and end '"' characters
  for ( auto i = m_lexeme.begin() + 1; i < m_lexeme.end() - 1; ++i ) {
    char c = *i;
    if ( c != '\\' )
      str_value += c;
    else {
      // handle escape sequence (we only handle some very basic ones)
      ++i;
      if ( i < m_lexeme.end() - 1 ) {
        c = *i;
        switch ( c ) {
        case 't':
          str_value += '\t';
          break;
        case 'n':
          str_value += '\n';
          break;
        case 'v':
          str_value += '\v';
          break;
        case 'f':
          str_value += '\f';
          break;
        case 'b':
          str_value += '\b';
          break;
        default:
          str_value += c;
        }
      }
    }
  }

  return str_value;
}

namespace {

// Make token length
constexpr int LEXER_MAX_TOK_LEN = 255;

// An instance of LexemeBuf accumulates the characters of a
// lexeme during lexical analysis.
struct LexemeBuf {
  char buf[LEXER_MAX_TOK_LEN+1];
  int n;

  LexemeBuf() : n( 0 ) { }
};

typedef Lexer *LEXER_TYPE;
typedef LexemeBuf *LEXEME_BUF_TYPE;

// The GET, UNGET, and ADD_TO_LEXEME functions are called
// by the generated lexer code to do I/O and to add
// recognized characters to the lexeme buffer.

int GET(Lexer *lex) {
  return lex->get();
}

void UNGET(Lexer *lex, int ch) {
  lex->unget( ch );
}

void ADD_TO_LEXEME(LexemeBuf *lb, int ch) {
  if (lb->n < LEXER_MAX_TOK_LEN)
    lb->buf[lb->n] = ch;
  ++lb->n;
}

#include "leximpl.cpp"

}

Lexer::Lexer( std::istream &in, const std::string &filename )
  : m_in( in )
  , m_next( TokenKind::INVALID )
  , m_at_eof( false )
  , m_loc( filename, 1, 1 ) {

}

Lexer::~Lexer() {

}

bool Lexer::peek( Token &tok ) {
  if ( !fill() )
    return false;
  tok = m_next;
  return true;
}

void Lexer::next( Token &tok ) {
  if ( !fill() )
    throw SimError( "Unexpected end of input", m_loc );
  tok = m_next;
  m_next = Token( TokenKind::INVALID );
}

int Lexer::get() {
  int c = doget();
  if ( c >= 0 )
    m_loc.advance( c );
  return c;
}

void Lexer::unget( int c ) {
  assert( c >= 0 );
  m_buf.push_back( char( c ) );
  m_loc.unadvance( c );
}

const Location &Lexer::get_loc() const {
  return m_loc;
}

int Lexer::doget() {
  // Was a character put back?
  // If so, remove it from the buffer
  // and return it.
  if ( !m_buf.empty() ) {
    char c = m_buf.back();
    m_buf.pop_back();
    return int( c );
  }

  // Have we reached EOF?
  if ( m_at_eof )
    return -1;

  // Try to read a character
  int c = m_in.get();
  if ( c < 0 )
    m_at_eof = true; // reached EOF
  return c;
}

bool Lexer::fill() {
  if ( m_at_eof )
    return false;
  if ( m_next.get_kind() != TokenKind::INVALID )
    return true; // have a valid token

  // Skip as many whitespace and/or comments
  // as we find here
  while ( skip_ws_or_comment() )
    ;

  // See if we've reached EOF
  int c = get();
  if ( c < 0 )
    return false;
  unget( c );

  // Remember the start location of the token
  Location loc = m_loc;

  // Try to scan one token
  struct LexemeBuf lexbuf;
  int rc = yylex( this, &lexbuf );
  if ( rc < 0 )
    throw SimError( "Invalid token", loc );
  if ( lexbuf.n > LEXER_MAX_TOK_LEN )
    throw SimError( "Max token length exceeded", loc );

  // NUL-terminate the lexeme string
  lexbuf.buf[ lexbuf.n ] = '\0';

  // As a special case, if the token is an IDENT whose
  // lexeme matches an instruction mnemonic, change its
  // kind to be OPCODE
  if ( TokenKind( rc ) == TokenKind::IDENT &&
       opcode_from_s( lexbuf.buf ) != Opcode::INVALID )
    rc = int( TokenKind::OPCODE );

  // We have a valid token
  m_next = Token( TokenKind( rc ), loc, std::string( lexbuf.buf ) );

  return true;
}

bool Lexer::skip_ws_or_comment() {
  int c;
  
  c = get();
  if ( c < 0 )
    return false;

  if ( isspace( c ) ) {
    // Skip whitespace
    do {
      c = get();
    } while ( c >= 0 && isspace( c ) );

    if ( c >= 0 )
      unget( c );

    return true; // skipped whitespace
  }

  if ( c == '/' ) {
    int c2 = get();
    if ( c2 != '*' ) {
      // not the beginning of a comment
      unget( c2 );
      unget( c );
      return false;
    }

    // Read until the end of the comment
    bool last_was_asterisk = false;
    for (;;) {
      c = get();
      if ( c < 0 )
        throw SimError( "EOF while reading comment", m_loc );
      if ( last_was_asterisk && c == '/' )
        return true; // skipped a comment
      last_was_asterisk = ( c == '*' );
    }
  }

  // We should be at the start of a token
  unget( c );
  return false;
}
