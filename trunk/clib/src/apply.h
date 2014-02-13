#ifndef _APPLY_H
#define _APPLY_H

#include "config.h"

#include <iostream>
#include "gridfield.h"
#include "gridfieldoperator.h"
#if HAVE_TR1_FUNCTIONAL
#include <tr1/functional>
#endif
// old code jhrg #include <ext/functional>
#include <string>

namespace GF {

class ApplyOp : public UnaryGridFieldOperator {
 public:
//  ApplyOp(GridFieldOperator *op, string tupleexpr, Scheme *outscheme);
  ApplyOp(string tupleexpr, Dim_t k, GridFieldOperator *op);
  void Execute();
  static GridField *Apply(string tupleexpr, Dim_t k, GridField *Gg);

 void SetExpression(const string &expr) { 
    unparsedExpr = expr; Update(); 
 };
 void SetRank(Dim_t _k) { k=_k; Update();};
  
 private:
  Dim_t k;
  string unparsedExpr;

};

} // namespace GF

#endif /* APPLY_H */
