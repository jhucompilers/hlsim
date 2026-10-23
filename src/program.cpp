#include "program.h"

Program::Program() {

}

Program::~Program() {

}

void Program::add_function( std::shared_ptr<Function> fn ) {
  m_functions.push_back( fn );
}
