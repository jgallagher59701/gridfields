#include <cstdlib>
#include "grid.h"

using namespace GF;

Grid *makeGrid(int start, int stop, char *name);
int main(int argc, char **argv) {

  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  CellArray *zerocells;  
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;
  int i;
  bool wf;

  if (verbose) cout << "making grid \n";
  grid = makeGrid(7, 14, "triangles");

  //printGrid(grid, 0);
  if (verbose) printf("grid well formed? "); 
  wf = grid->checkWellFormed();
  if (verbose) printf("%i\n", wf);
  //getchar(); 

#if 0
  zerocells = new CellArray();
  for (i=0; i<12; i++) {
    node = i;
    zerocells->addCellNodes(&node, 1);
  }
  
  if (verbose) printf("set 0 cells again\n"); 
  grid->setKCells(zerocells, 0);
  
  grid->print();
  if (verbose) printf("grid well formed? "); 
  wf = grid->checkWellFormed();
  if (verbose) printf("%i\n", wf); 
#endif
  //getchar(); 
  
  //grid->setImplicit0Cells(4);
   
  //printGrid(grid, 0);
  //printf("grid well formed? "); 
  //wf = grid->checkWellFormed();
  //printf("%i\n", wf); 
  //getchar(); 
  
  Grid *grid2 = makeGrid(11, 17, "small");
  if (verbose) grid2->print();
  if (verbose) cout << "intersecting...\n";
  Grid *igrid = grid->Intersection(grid2);
  wf = igrid->checkWellFormed();
  if (verbose) cout << "Intersection well formed? " << (int) wf << "\n";
  if (verbose) igrid->print();
  
  delete grid;
}


Grid *makeGrid(int start, int stop, char *name) {
  CellArray *twocells;
  CellArray *onecells;
  CellArray *zerocells;
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;
  int scale = stop-start;
  
  bool wf;
  int i;
  twocells = new CellArray();
  
  for (i=start; i<=stop-2; i++) {
    triangle[0] = i;
    triangle[1] = i+1;
    triangle[2] = i+2;
    twocells->addCellNodes(triangle, 3);
    //cout << "triangle: " << triangle[0] << "," << triangle[1] << ", " << triangle[2] << "\n";
  }
  //twocells->print();
  //getchar(); 
  onecells = new CellArray();
  for (i=start; i<=stop-1; i++) {
    segment[0] = i;
    segment[1] = i+1;
    onecells->addCellNodes(segment, 2);
  }
  //onecells->print();
  
  zerocells = new CellArray();
  for (i=start; i<=stop; i++) {
    Node n = Node(i);
    zerocells->addCellNodes(&n, 1);
  }

  //getchar(); 
  grid = new Grid(name, 2);
  grid->setKCells(zerocells, 0);
  grid->setKCells(onecells, 1);
  grid->setKCells(twocells, 2);
  //grid->print(0);
  //getchar();
  printf("made grid: "); 
  wf = grid->checkWellFormed();
  printf("%i\n", wf);
  getchar();
  return grid;
  
}

