#ifndef SIMULATOR_H
#define SIMULATOR_H

class Program;

class Simulator {
private:
  const Program &m_program;

  // no value semantics
  Simulator( const Simulator & ) = delete;
  Simulator &operator=( const Simulator & ) = delete;

public:
  Simulator( const Program &program );
  ~Simulator();

  void execute();
};

#endif // SIMULATOR_H
