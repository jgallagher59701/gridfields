#ifndef _ITERATE_H
#define _ITERATE_H

#include <iostream>
#include "gridfield.h"
#include "gridfieldoperator.h"
#include <string>

class StateOp;

class IterateOp : public BinaryGridFieldOperator {
 public:
  IterateOp(GridFieldOperator *Outer, Dim_t i,
            GridFieldOperator *Inner, Dim_t j,
            StateOp *State) 
          : BinaryGridFieldOperator(Outer, Inner),
            state(State), _i(i), _j(j)
            {};

  void Execute(); 
  /*
  static GridField *Iterate(GridField *T, 
     			    GridField *S,
                            StateOp *State);
  */
 private:
  StateOp *state;
  Dim_t _i, _j;
  
  static string newName(string Aname, string Bname);

};

#endif /* CROSS_H */
