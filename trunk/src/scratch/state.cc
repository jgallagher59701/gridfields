#include "timing.h"
#include "state.h"


StateOp::StateOp(GridFieldOperator *op) : UnaryGridFieldOperator(op)
{
  //this->cleanup = false;
}

void StateOp::Execute() {
  this->GF = this->PreviousOp->getResult();
  
  Result = this->GF;
  Result->ref();
}

void StateOp::SetState(GridFieldOperator *op) {
  this->PreviousOp = op;
  this->Update();
}

