#ifndef _STATE_H
#define _STATE_H

#include <iostream>
#include "gridfield.h"
#include "gridfieldoperator.h"
#include <string>

class StateOp : public UnaryGridFieldOperator {
 public:
  StateOp(GridFieldOperator *A);

  void SetState(GridFieldOperator *GF);
  void Execute(); 
 private:

};

#endif /* CROSS_H */
