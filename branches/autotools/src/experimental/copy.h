#ifndef _COPY_H
#define _COPY_H

#include <iostream>
#include "gridfield.h"
#include "gridfieldoperator.h"
#include <ext/functional>
#include <string>

class CopyOp : public UnaryGridFieldOperator {
 public:
  CopyOp(string tupleexpr, Dim_t k, GridFieldOperator *op);
  void Execute();
  static GridField *Copy(GridField *Gg);

 private:

};

#endif /* COPY_H */
