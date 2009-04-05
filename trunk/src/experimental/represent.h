#ifndef REPRESENT_H
#define REPRESENT_H

#include <string>
#include "gridfield.h"
#include "gridfieldoperator.h"

class RepresentOp : public UnaryGridFieldOperator {
 public:
  RepresentOp(GridFieldOperator *subexpr);
  void Execute(); 
  static GridField *Represent(GridField *A);
 private:
  static string newName(string Aname, string Bname);

};

#endif
