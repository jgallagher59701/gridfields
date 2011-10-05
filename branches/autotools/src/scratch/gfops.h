
#ifndef GFOPS_H
#define GFOPS_H
#include <string>
#include <sstream>
#include "CmdLine.h"

class GridField;

template <class T>
bool from_string(T &t, const std::string &s) {
  std::istringstream iss(s);
  return !(iss >> std::dec >> t).fail();
};

void checkopts( int argc, char **argv );
void showhelp();

#endif
