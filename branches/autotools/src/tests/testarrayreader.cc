#include <iostream>
#include <fstream>
#include <sstream>
#include "array.h"
#include "grid.h"
#include "elcircfile.h"
#include "arrayreader.h"
#include "gridfield.h"

using namespace std;

Grid *makeGrid(int scale, string name);

int main(int argc, char **argv) {
  ElcircFile ef("/home/workspace/ccalmr/forecasts/2003-25-RWKoo/run/1_hvel.64");
  cout << ef.h.variable_nm << "|" << endl;
  string nm(ef.h.variable_nm);
  string nm2(ef.h.variable_nm);
  cout << nm << endl;
  cout << (nm == "horizontal velocity") << endl;
  cout << (nm == string("horizontal velocity")) << endl;
  cout << (nm == nm2) << endl;
  
  cout << "offset: " << ef.getVariableOffset(0, 0, 0) << endl;
  cout << "offset: " << ef.getVariableOffset(3, 0, 0) << endl;
  string s("u:f, v:f");
  Scheme sch(s);
  sch.print();
  ProjectArrayReader pa("/home/workspace/ccalmr/forecasts/2003-25-RWKoo/run/1_hvel.64", 1204616, "foo", s, "u");
  pa.GetScheme().print();
}

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
  printf("foo\n");
  return grid; 
}
