
#include <cstdio>
#include <cstring>
#include "../src/grid.h"
#include "../src/onegrid.h"
#include "../src/gridfield.h"
#include "../src/array.h"
#include "../src/arrayreader.h"
#include "../src/cross.h"

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
GridField *makeGridField(int size, char *gridname, char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(size, gridname);
  AbstractCellArray *cells = G->getKCells(k);
  data = makeFloatArray(cells->getsize(), datname);

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
    GridField *Aa;
    GridField *Bb;

    if (verbose) cout << "Test 1" << endl;
    OneGrid *A = new OneGrid("A", 3);
    Array *a = makeFloatArray(3, "a");
    //  GF = makeGridField(5, "A", "x", 0);
    OneGrid *B = new OneGrid("B", 3);
    Array *b = makeFloatArray(3, "b");
  
    Aa = new GridField(A, 0, a);
    //Aa->print(3);
    Bb = new GridField(B, 0, b);
    //  GF2 = makeGridField(12, "A", "x", 2);
  
    GridField *Result = CrossOp::Cross(Aa,Bb);
  
    if (verbose) Result->print();

    FileArrayReader *ar = new FileArrayReader("dat");    cerr << "here: "<< endl;
    Array *arr = new Array("io", FLOAT, 3*3);
    ar->Read(Result, 0, arr);
 
    if (verbose) arr->print();
  
    //  cout << "Test 2: Indirect Execution" << endl;

    return EXIT_SUCCESS;
  }
  catch (std::string &e) {
    cerr << "Error: " << e << endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    cerr << "Unknown error." << endl;
    return EXIT_FAILURE;
  }
}
