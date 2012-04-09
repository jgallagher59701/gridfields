#ifndef _JOIN_H
#define _JOIN_H

#include <iostream>
#include "expr.h"
#include "gridfieldoperator.h"
#include <string>

class GridField;

class JoinOp : public BinaryGridFieldOperator {
 public:
  JoinOp(Condition *p, GridFieldOperator *A, GridFieldOperator *B);
  Condition *condition;
  
  void Execute(); 
  static GridField *Join(Condition *p, 
			 GridField *Aa, 
     			 GridField *Bb);
 private:
  static string newName(string Aname, string Bname);
  static Cell *Cross_if_wellformed(Cell &a, 
				   Cell &b, 
				   set<Node> &nodes, 
				   CrossNodeMap &h);
  
};

#endif /* JOIN_H */
