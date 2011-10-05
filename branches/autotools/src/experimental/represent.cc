#include "represent.h"
#include "timing.h"

TransposeRepresentOp::RepresentOp(GridField *A) { 
  this->GF = A;
  this->PreviousOp = NULL;
}

RepresentOp::RepresentOp(GridFieldOperator *subexpr) {
  this->GF = NULL;
  this->PreviousOp = subexpr;
}
    
void RepresentOp::Execute() {
  this->PrepareForExecution();
  Result = this->Represent(this->A, this->B);
}

GridField *RepresentOp::Represent(GridField *Gf) {
}
