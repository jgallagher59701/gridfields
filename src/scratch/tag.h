#ifndef _TAG_H
#define _TAG_H

#include <iostream>
#include "gridfield.h"
#include "gridfieldoperator.h"
#include "constarray.h"
#include <string>

class TagOp : public BinaryGridFieldOperator {
 public:
  TagOp(GridFieldOperator *A, Dim_t i, GridFieldOperator *B, Dim_t j);
  ~TagOp();
  void Execute(); 
  GridField *Tag(GridField *A, Dim_t i, GridField *B, Dim_t j, int idx);
  
 private:
  int idx;
  Dim_t _i;
  Dim_t _j;
  string newName(string Aname, string Bname);
  map<string, ConstArray *> consts;

};

#endif 
