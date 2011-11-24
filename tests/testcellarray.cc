#include "cellarray.h"
#include "implicit0cells.h"
#include "util.h"
#include "implicitcrossnodemap.h"

int main(int argc, char **argv) {
  
  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  CellArray *cells1 = new CellArray();
  CellArray *cells2 = new CellArray();
	//cout << "allocated\n";
  CellArray *cellsout;
  Node nodes[3];
  int i;

  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 2;
  cells1->addCellNodes(nodes, 3);
  nodes[0] = 2;
  nodes[1] = 3;
  nodes[2] = 0;
  cells1->addCellNodes(nodes, 3);
  nodes[0] = 2;
  nodes[1] = 4;
  nodes[2] = 3;
  cells1->addCellNodes(nodes, 3);
  nodes[0] = 0;
  nodes[1] = 3;
  nodes[2] = 5;
  cells1->addCellNodes(nodes, 3);
  nodes[0] = 3;
  nodes[1] = 6;
  nodes[2] = 5;
  cells1->addCellNodes(nodes, 3);
  nodes[0] = 6;
  nodes[1] = 3;
  nodes[2] = 4;
  cells1->addCellNodes(nodes, 3);
  nodes[0] = 7;
  nodes[1] = 5;
  nodes[2] = 6;
  cells1->addCellNodes(nodes, 3);
  
  if (verbose) printf("loading...\n"); 
 
  for (i=0; i<10; i++) {
    nodes[0] = i;
    nodes[1] = i+1;
    nodes[2] = i+2;
    cells2->addCellNodes(nodes, 3);
  }

#if 0
  if (verbose) printf("loading another...\n");  
  for (i=0; i<10; i++) {
    nodes[0] = 2-i;
    nodes[1] = 3-i;
    nodes[2] = 4-i;
    cells2->addCellNodes(nodes, 3);
  }
#endif
 
  cellsout = cells1->Intersection(cells2);
  Cell *c1 = cells1->getCell(0);
  Cell *c2 = cells2->getCell(0);

  set<CellId> cs;
  cells1->getIncidentCells(3, cs);
  set<CellId>::iterator p;
  if (verbose)
    for (p=cs.begin(); p!=cs.end(); ++p)
      cout << (*p) << endl; 
  
//  c1->print();
//  c2->print();
//  cout << "lessthan? " << (*c2 < *c1) << "\n";
  
//  printf("printing...\n");  
//  cellsout->print(0); 

  Node *rawcells = new Node[9];
  
  rawcells[0] = 3;
  rawcells[1] = 0;
  rawcells[2] = 1;
  rawcells[3] = 2;
  
  rawcells[4] = 4;
  rawcells[5] = 1;
  rawcells[6] = 2;
  rawcells[7] = 3;
  rawcells[8] = 4;

  CellArray *newtest = new CellArray(rawcells, 2); 
  if (verbose) newtest->print();  

  CellArray *bad = new CellArray(rawcells, 2, 3);
  if (verbose) bad->print();

  Implicit0Cells x(5);
  if (verbose) x.print();

  Implicit0Cells y(3);
  if (verbose) y.print();
  
  Implicit0Cells *z = x.Intersection(&y);
  if (verbose) z->print();

  ImplicitCrossNodeMap h(&x,&y); 
  z = x.Cross(&y, h);
  if (verbose) z->print();

  ImplicitCrossNodeMap h2(&x, &x);
  if (verbose) x.Cross(newtest, h2)->print();

  Cell c(x.getCellCopy(2));
  if (verbose) c.print();

  z->unref();
}
