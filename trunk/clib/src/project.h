#ifndef _PROJECT_H
#define _PROJECT_H

#include <iostream>
#include "tuple.h"
#include "array.h"
#include "gridfield.h"
#include "fparser.hh"
#include "gridfieldoperator.h"
#include <ext/functional>
#include <string>

namespace GF {

class ProjectOp : public UnaryGridFieldOperator {
 public:
  ProjectOp(GridFieldOperator *op, Dim_t k, vector<string> &keepers);
  ProjectOp(GridFieldOperator *op, Dim_t k, string attribute_list);
  
  void Execute();
  static GridField *Project(GridField *Gg, Dim_t k, vector<string> ks);
  static GridField *Project(GridField *Gg, Dim_t k, string keeper);
 private:
  Dim_t _k;
  vector<string> keepers;
};

} // namespace GF

#endif /* PROJECT_H */
