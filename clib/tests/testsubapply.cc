#include "grid.h"
#include "gridfield.h"
#include "array.h"
#include "restrict.h"
#include "subapply.h"
#include "arrayreader.h"
#include "accumulate.h"
#include "bind.h"

using namespace GF;

Grid *makeGrid(int scale, char *name) {
  CellArray *twocells;
  CellArray *onecells;
  CellArray *zerocells;
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;

  bool wf;
  int i;
  twocells = new CellArray();
  for (i=0; i<scale/2; i++) {
    triangle[0] = i;
    triangle[1] = i+1;
    triangle[2] = i+2;
    twocells->addCellNodes(triangle, 3);
  }
  //twocells->print();
  //getchar(); 
  onecells = new CellArray();
  for (i=0; i<scale-1; i++) {
    segment[0] = i;
    segment[1] = i+1;
    onecells->addCellNodes(segment, 2);
  }
  //onecells->print();
  
  //getchar(); 
  grid = new Grid(name, 2);
  grid->setImplicit0Cells(scale);
  grid->setKCells(onecells, 1);
  grid->setKCells(twocells, 2);
  //grid->print(0);
  //getchar();
  return grid; 
}

Array *makeFloatArray(int size, char *name) {
  Array *arr;
  float data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = 2*i-10;
  }
  arr = new Array(name, FLOAT);
  arr->copyData(data, size);
  return arr;  
}

GridField *makeGridField(int size, char *gridname, char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(12, gridname);
  k = 0;
  data = makeFloatArray(12, datname);

  GF = new GridField(G, k, &data, 1);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {

  GridField *outer = makeGridField(4, "outer", "x", 0); 
  GridField *inner = makeGridField(4, "inner", "y", 0); 
  outer->print();
  inner->print();

  outer = AccumulateOp::Accumulate(outer, "b", "b+4", "0", 0);
  outer->print();
  
  BindOp *bnd = new BindOp("d", FLOAT, "dat", 0, inner);
  
  //RestrictOp *r = new RestrictOp(new Condition("y", -5.0f, ">"), bnd);
  
  SubApplyOp *sub = new SubApplyOp(bnd, "inner", outer);
//  sub->parameterize("x", r, (ParameterAssigner::ParamFunc) &RestrictOp::setFloatValue);
  sub->parameterize("b", bnd, (ParameterAssigner::ParamFunc) &BindOp::setOffset);
  
  GridField *gf;
  GridField *nested = sub->getResult();
  if (nested == NULL) cout << "NO GRID?" << endl;

  for (int i=0; i< outer->card(); i++) {
    gf = (GridField *) nested->getAttributeVal("inner", i);
    gf->getAttribute("y")->print();
    gf->getAttribute("d")->print();
  }
}
