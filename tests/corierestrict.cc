#include "grid.h"
#include "gridfield.h"
#include "array.h"
#include "restrict.h"
#include "elio.h"

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


Array *makeIntArray(int size, char *name) {
  Array *arr;
  int data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = i*2-10;
  }
  arr = new Array(name, INT);
  arr->copyData(data, size);
  return arr;  
}
GridField *makeGridField(int size, char *gridname, char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(12, "A");
  k = 0;
  data = makeIntArray(12, "x");

  GF = new GridField(G, k, &data, 1);
  //printf("Valid? %i\n", !notValid(GF));
  //printGridField(GF, 2);

  return GF;
}

int main(int argc, char **argv) {
  GridField *GF;
  GridField *Result;
  Condition *p;

  GF = makeGridField(12, "A", "x", 0);

  //GF->print(10);
  p = new Condition("x", 4, ">");
  printf("restricting...\n");
  Result = RestrictOp::Restrict(p,GF);

  printf("restricted...\n");
  Result->print(0);
}

