#include "gridfield.h"
#include "constarray.h"
#include "timing.h"
#include "bindconstant.h"

BindConstantOp::BindConstantOp(Dim_t k, const string &attr, float val, 
                               GridFieldOperator *op) 
   : UnaryGridFieldOperator(op), _k(k), attr(attr), val(val)
{
  //this->cleanup = false;
}

void BindConstantOp::Execute() {
  this->PrepareForExecution();
  Result =  BindConstantOp::BindConstant(this->_k, this->attr, this->val, this->GF);
}


void BindConstantOp::setAttributeVal(const string &a, float v) {
  this->attr = a;
  this->val = v;
  this->Update();
}

GridField *BindConstantOp::BindConstant(Dim_t k, const string &attr, float val, 
                                        GridField *GF) {

  ConstArray *a = new ConstArray(attr, GF->Size(k), val);

  GridField *Out = GF;
  
  Out->unBind(k, attr);

  Out->Bind(k, a);
  
  return Out;
}
