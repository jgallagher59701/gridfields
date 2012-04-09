#ifndef _CONNECTOP_H
#define _CONNECTOP_H

#include "gridfieldoperator.h"

class ConnectOp : public UnaryGridFieldOperator {
public:
  ConnectOp(GridFieldOperator *op);
  
  void Execute();
  
  static GridField *Connect(GridField *GF);
  static GridField *ConnectTriangle(GridField *GF);
  static GridField *ConnectDelaunay(GridField *GF);

protected:
private:
 
};

#endif
