#include "gridfield.h"
#include "array.h"
#include "restrict.h"
extern "C" {
#include "stdio.h"
#include "elio.h"
}
#include "expr.h"
#include "timing.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include "gridfield.h"
#include "array.h"
#include "onegrid.h"
#include "scanvgrid.h"

using namespace std;

ScanVGridOp::ScanVGridOp(string filename) : ScanOp(filename, 0)
{
  this->filename = filename;
}

void ScanVGridOp::Execute() {
  this->Result =  Scan(this->filename);
}

GridField *ScanVGridOp::Scan(string filename) {
  ElcircHeader h;
  GridField *G;
  ElioGetHeader((char *) filename.c_str(), &h);
  G = gfV(h);
  ElioFreeHeader(&h);
  return G;
}


GridField *ScanVGridOp::gfV(ElcircHeader &h) {

  OneGrid *G = new OneGrid("V", h.nvrt);
  Array *z = new Array("z", FLOAT);
  //   print();i
  for (int i=0; i<h.nvrt; i++) {
    h.zcor[i] = h.zcor[i];
  }
  z->copyFloatData(h.zcor, h.nvrt);
  GridField *V = new GridField(G, 0, z);
  return V;
}

