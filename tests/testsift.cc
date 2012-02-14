#include "src/grid.h"
#include "src/gridfield.h"
#include "src/array.h"
#include "src/sift.h"

Grid *makeGrid(int scale,const char *name) {
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

Array *makeIntArray(int size,const char *name) {
  Array *arr;
  int data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = i*2-10;
  }
  arr = new Array(name, INT);
  arr->copyIntData(data, size);
  return arr;
}

GridField *makeGridField(int size,const char *gridname,const char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(12, "A");
  k = 2;
  data = makeIntArray(6, "x");

  GF = new GridField(G, k, data);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {

  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  GridField *GF;

  GF = makeGridField(12, "A", "x", 0);
  if (verbose) GF->PrintTo(cout, 4);
  GridField *GF2 = SiftOp::Sift(0, GF);
  if (verbose) GF2->PrintTo(cout, 5);
}

