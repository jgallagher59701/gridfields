#include "apply.h"
#include "timing.h"
#include "gridfield.h"
#include "array.h"
#include "tuple.h"
#include "expr.h"
#include <sstream>
#include <math.h>

/*
CopyOp::CopyOp(GridFieldOperator *op, string tupleexpr, Scheme *outscheme) 
   : UnaryGridFieldOperator(op), unparsedExpr(tupleexpr), _sch(outscheme)
{
  this->SaveReservedWords();
  this->cleanup = false;
}
*/

CopyOp::CopyOp(string tupleexpr, Dim_t k, GridFieldOperator *op) 
   : UnaryGridFieldOperator(op), k(k), unparsedExpr(tupleexpr)
{
  //this->cleanup = false;
}

void CopyOp::Execute() {
  
  this->PrepareForExecution();

  float start = gettime();
  this->Result =  Copy(this->unparsedExpr, this->k,
                        this->GF);
}

GridField *CopyOp::Copy(GridField *Gg) {
  GridField *Out = Gg->DeepCopy(); 
  cout << "Copy()" << endl;
  Out->ref();
  return Out;
}

