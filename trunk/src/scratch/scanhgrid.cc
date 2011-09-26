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
#include "onegrid.h"
#include "scanhgrid.h"

using namespace std;

ScanHGridOp::ScanHGridOp(string filename) : ScanOp(filename, 0) {
  this->filename = filename;
  this->offset = 0;
}

void ScanHGridOp::Execute() {
  Result =  Scan(this->filename);
}

GridField *ScanHGridOp::Scan(string filename) {

  //eventually need to check memory
  
  ElcircHeader h;
  GridField *G;
  ElioGetHeader((char *) filename.c_str(), &h);
//  ElioPrintHeader(h);
  G = ScanHGridOp::gfH(h);
  ElioFreeHeader(&h);
  return G;
}

GridField *ScanHGridOp::gfH(ElcircHeader &h) {
  Node nodes[4];
  
  CellArray *elements = new CellArray();
  for (int i=0; i<h.ne; i++) {
    for (int j=0; j<h.etype[i]; j++) {
      nodes[j] = (Node) h.icon[j][i];
    }
    elements->addCellNodes(nodes, h.etype[i]);
  }
    
  Grid *G = new Grid("gfGeo", 2);
  GridField *GF = new GridField(G);
  G->setImplicit0Cells(h.np);
  G->setKCells(elements, 2);
  //cout << "checkwellformed...\n";typedef map<Node, int> NodeMap;
  //int wf = G->checkWellFormed();
  //cout << "Wellformed? " << wf << "\n";
 
  string name("x");
  Array *x = new Array(name.c_str(), FLOAT);
  x->copyFloatData(h.x, h.np);
  name = "y";
  Array *y = new Array(name.c_str(), FLOAT);
  y->copyFloatData(h.y, h.np);
  name = "h";
  Array *d = new Array(name.c_str(), FLOAT);
  d->copyFloatData(h.d, h.np);
  name = "b";
  Array *b = new Array(name.c_str(), INT);
  b->copyIntData(h.bi, h.np);

  GF->Bind(0, x);
  GF->Bind(0, y);
  GF->Bind(0, d);
  GF->Bind(0, b);
    
  return GF;
}

