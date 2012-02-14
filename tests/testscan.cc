#include "grid.h"
#include "gridfield.h"
#include "array.h"
#include "arrayreader.h"
#include "scanhgrid.h"
#include "scanvgrid.h"
#include "scaninternal.h"


int main(int argc, char **argv) {
  //GridField *H = ScanHGridOp::Scan("../../../1_salt.63");
  //GridField *V = ScanVGridOp::Scan("../../../1_salt.63");
  
  //H->print();
  //V->print();


  GridField  *GF = ScanInternal::Scan("temp.gf", 0);
  GF->print();
}

