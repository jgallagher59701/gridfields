#include "grid.h"
#include "onegrid.h"
#include "gridfield.h"
#include "array.h"
#include "arrayreader.h"
#include "join.h"

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

Array *makeFloatArray(int size, char *name, int factor) {
  Array *arr;
  float data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = factor*i-10;
  }
  arr = new Array(name, FLOAT);
  arr->copyData(data, size);
  return arr;  
}
GridField *makeGridField(int size, char *gridname, char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(size, gridname);
  CellArray *cells = G->getKCells(k);
  data = makeFloatArray(cells->getsize(), datname, 2);

  GF = new GridField(G, k, data);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {
  
  GridField *Aa;
  GridField *Bb;

  OneGrid *A = new OneGrid("A", 3);
  Array *a = makeFloatArray(3, "a", 2);
//  GF = makeGridField(5, "A", "x", 0);
  OneGrid *B = new OneGrid("B", 3);
  Array *b = makeFloatArray(3, "b", 1);
  
  Aa = new GridField(A, 0, a);
  Aa->print(3);
  Bb = new GridField(B, 0, b);
  Bb->print(3);
//  GF2 = makeGridField(12, "A", "x", 2);
  
  Condition *p = new Condition("a", "b", "<");
  GridField *Result = JoinOp::Join(p,Aa,Bb);
   
  Result->print();
/*
  ArrayReader *ar = new ArrayReader("dat", 3*4);
  Array *arr = new Array("io", FLOAT);
  ar->Read(Result, 0, arr);
 */
//  arr->print();
}
