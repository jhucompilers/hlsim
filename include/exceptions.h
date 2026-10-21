#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include "location.h"

class SimError : public std::runtime_error {
private:
  Location m_loc;

public:
  SimError( const std::string &msg, const Location &loc );
  SimError( const SimError &other );
  virtual ~SimError();

  SimError &operator=( const SimError &rhs );

  const Location &get_loc() const;
};

#endif // EXCEPTIONS_H
