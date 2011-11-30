#include "src/grid.h"
#include "src/gridfield.h"
#include "src/array.h"
#include "src/restrict.h"
#include "src/merge.h"
#include<cstring>

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

Array *makeFloatArray(int size,const char *name) {
  Array *arr;
  arr = new Array(name, FLOAT, size);
  float *data;
  arr->getData(data);
  int i;

  for (i=0; i<size; i++) {
      data[i] = 2*i-10;
  }
  return arr;  
}
GridField *makeGridField(int size,const char *gridname,const char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(12, "A");
  k = 0;
  data = makeFloatArray(12, "x");

  GF = new GridField(G);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();
 
  GF->Bind(0,data);
  return GF;
}

int main(int argc, char **argv) {

  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  GridField *GF;
  GridField *rGF1;
  GridField *rGF2;
  GridField *mGF;

  if (verbose) cout << "Test 1: indirect execution" << endl;

  GF = makeGridField(12, "A", "x", 0);

  if (verbose) GF->PrintTo(cout, 4);
  string p = "x<6";
  string q = "x>-2";
  if (verbose) printf("restricting...\n");
  rGF1 = RestrictOp::Restrict(p, 0, GF);
  //rGF1->print(0);
  rGF2 = RestrictOp::Restrict(q, 0, GF);
  //rGF2->print(0);
  //getchar();
  if (verbose) printf("merging...");
  mGF = MergeOp::Merge(rGF1, rGF2); 
  if (verbose) mGF->PrintTo(cout, 5);
  if (verbose) printf("merged...\n");

#if 0
  if (verbose) cout << "Test 2: indirect execution" << endl;

  GridFieldOperator *r1 = new RestrictOp(p, 0, GF);
  GridFieldOperator *r2 = new RestrictOp(q, 0, GF);
  GridFieldOperator *m1 = new MergeOp(r1, r2);

  m1->Execute();

  if (verbose) r1->getResult()->PrintTo(cout, 4);
  if (verbose) r2->getResult()->PrintTo(cout, 4);
  if (verbose) m1->getResult()->PrintTo(cout, 4);
#endif
}

