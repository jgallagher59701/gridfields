
#include <cstdio>

#include "src/grid.h"
#include "src/gridfield.h"
#include "src/bind.h"
#include "src/array.h"
#include "src/restrict.h"
#include "src/refrestrict.h"
#include "src/arrayreader.h"
#include "src/accumulate.h"

Grid *makeGrid(int scale, string name) {
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
GridField *makeGridField(int size, string gridname,const char *datname, int k) {

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
  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  try {
    GridField *GF;
    GridField *Result;

    GF = makeGridField(12, "A", "x", 0);
    Array *arr = new Array("io", FLOAT, 12);
    GF->Bind(0, arr);

    GridField *aGF = AccumulateOp::Accumulate(GF, 0, "result", "result+1", "0", 0);

    if (verbose) aGF->print(9);
    printf("restricting...\n");
    Result = RefRestrictOp::Restrict("x<4",0,GF);
    if (verbose) Result->print(0);
    Result = RefRestrictOp::Restrict("x>-4",0,Result);
    if (verbose) Result->print(10);

    FileArrayReader *ar = new FileArrayReader("dat", 0);
    ar->setPatternAttribute("result");
    GridField *G = BindOp::Bind("io", FLOAT, ar, 0, Result);
 
    if (verbose) G->print();

    return EXIT_SUCCESS;
  }
  catch (std::string &e) {
    cerr << "Error: " << e << endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    cerr << "Unknown Error." << endl;
    return EXIT_FAILURE;
  }
}

