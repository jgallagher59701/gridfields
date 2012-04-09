#include "grid.h"
#include "gridfield.h"
#include "array.h"
#include "restrict.h"
#include "merge.h"

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
  triangle[0] = 2;
  triangle[1] = 0;
  triangle[2] = 1;
  twocells->addCellNodes(triangle, 3);
  for (i=1; i<scale/2; i++) {
    triangle[0] = scale-i;
    triangle[1] = scale-1-i;
    triangle[2] = scale-2-i;
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
  
  G = makeGrid(12, "A");
  k = 0;
  data = makeFloatArray(12, "x");

  GF = new GridField(G, k, data);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {
  GridField *GF;
  GridField *rGF1;
  GridField *rGF2;
  GridField *mGF;
  Condition *p,*q;

  cout << "Test 1: indirect execution" << endl;

  GF = makeGridField(12, "A", "x", 0);

  //GF->print(9);
  p = new Condition("x", 4.0f, "<");
  q = new Condition("x", -4.0f, ">");
  printf("restricting...\n");
  rGF1 = RestrictOp::Restrict(p,GF);
  //rGF1->print(0);
  rGF2 = RestrictOp::Restrict(q,GF);
  //rGF2->print(0);
  //getchar();
  printf("merging...");
  mGF = MergeOp::Merge(rGF1, rGF2); 
  //mGF->print();
    
  printf("merged...\n");

  cout << "Test 2: indirect execution" << endl;

  GridFieldOperator *r1 = new RestrictOp(p, GF);
  GridFieldOperator *r2 = new RestrictOp(q, GF);
  GridFieldOperator *m1 = new MergeOp(r1, r2);

  m1->Execute();

  r1->getResult()->print();
  r2->getResult()->print();
  m1->getResult()->print();
}

