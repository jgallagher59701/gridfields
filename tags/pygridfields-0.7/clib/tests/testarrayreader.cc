
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

#include "src/array.h"
#include "src/grid.h"
#include "src/elcircfile.h"
#include "src/arrayreader.h"
#include "src/gridfield.h"

// Was originally used explicitly in the code
// "/home/workspace/ccalmr/forecasts/2003-25-RWKoo/run/1_hvel.64"
#define TEST_DATA "data/1_flsu.61"

using namespace std;

Grid *makeGrid(int scale, string name);

int main(int argc, char **argv) {

  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  try {
    ElcircFile ef(TEST_DATA);
    if (verbose) cout << ef.h.variable_nm << "|" << endl;
    string nm(ef.h.variable_nm);
    string nm2(ef.h.variable_nm);
    if (verbose) {
      cout << nm << endl;
      cout << (nm == "horizontal velocity") << endl;
      cout << (nm == string("horizontal velocity")) << endl;
      cout << (nm == nm2) << endl;
  
      cout << "offset: " << ef.getVariableOffset(0, 0, 0) << endl;
      cout << "offset: " << ef.getVariableOffset(3, 0, 0) << endl;
    }
    string s("u:f, v:f");
    Scheme sch(s);
    if (verbose) sch.print();
    ProjectArrayReader pa(TEST_DATA, 1204616, "foo", s, "u");
    if (verbose) pa.GetScheme().print();

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
