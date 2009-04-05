#include "grid.h"
#include "gridfield.h"
#include "array.h"
#include "restrict.h"
#include "refrestrict.h"
#include "arrayreader.h"
#include "apply.h"

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
  
  G = makeGrid(12, "A");
  k = 0;
  data = makeFloatArray(12, "x");

  GF = new GridField(G);
  GF->AddAttribute(k, data);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {
  GridField *GF;
  GridField *Result;

  GF = makeGridField(12, "A", "x", 0);
  
  cout << "TEST 1" << endl;
  string p = "x<4";
  string q = "x>4";
  
  GF->PrintTo(cout, 9);
  Result = RestrictOp::Restrict("x<-100", 0, GF);
  Result->PrintTo(cout, 9);
  /*
  printf("restricting...\n");
  Result = RefRestrictOp::Restrict(p, 0, GF);
  //Result->PrintTo(cout, 0);
  Result = RefRestrictOp::Restrict(q, 0, Result);
  Result->PrintTo(cout, 10);

  ArrayReader *ar = new FileArrayReader("dat", 0);
  Array *arr = new Array("io", FLOAT);
  ar->Read(Result, 0, arr);
 
  arr->print();
 
  cout << "TEST 1: Indirect execution." << endl;
  */

  GF = makeGridField(12, "A", "x", 0);
  GridFieldOperator *op1 = new RestrictOp(p, 0, GF);
  cout << "Got op" << endl;
  Result = op1->getResult();
  Result->PrintTo(cout, 10);
  GridFieldOperator *op2 = new RestrictOp(q, 0, op1);
  cout << "Got op2" << endl;
  
  GridFieldOperator *op3 = new ApplyOp("a=x*2", 0, op2);
  cout << "Got op3" << endl;
  GridFieldOperator *op4 = new RestrictOp("a>-2", 0, op3);
  cout << "Got op4" << endl;
  op4->Execute();
  cout << "Executed" << endl;
 
  Result = op4->getResult();
  Result->PrintTo(cout, 10);
  
}

