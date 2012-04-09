#ifndef _CROSS_H
#define _CROSS_H

#include <iostream>
#include "gridfield.h"
#include "gridfieldoperator.h"
#include <string>

class CrossOp : public BinaryGridFieldOperator {
 public:
  CrossOp(GridFieldOperator *A, GridFieldOperator *B);
  void Execute(); 
  static GridField *Cross(GridField *T, 
     			  GridField *S);


 private:
  static string newName(string Aname, string Bname);
  static void crossData(GridField *Gg, GridField *Aa, Dim_t i, GridField *Bb, Dim_t j);

};

#endif /* CROSS_H */
