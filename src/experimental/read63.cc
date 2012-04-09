extern "C" {
#include "stdio.h"
#include "elio.h"
}
#include <iostream>
#include <iomanip>
#include <fstream>
#include "gridfield.h"
#include "array.h"
#include "onegrid.h"
#include "read63.h"

using namespace std;

GridField *readHGrid(string filename) {
  return readHGrid((char *) filename.c_str());
}

GridField *readHGrid(char *filename) {
  ElcircHeader h;
  GridField *G;
  ElioGetHeader(filename, &h);
//  ElioPrintHeader(h);
  G = gfH(h);
  ElioFreeHeader(&h);
  return G;
}

void GetHeader(ElcircHeader *h, char *fn) {
  ElioGetHeader(fn, h);
}

GridField *readVGrid(string filename) {
  return readVGrid((char *) filename.c_str());
}

GridField *readVGrid(char *filename) {
  ElcircHeader h;
  GridField *G;
  ElioGetHeader(filename, &h);
  G = gfV(h);
  ElioFreeHeader(&h);
  return G;
}

int newid(int node, int *map, int size) {
  for (int i=0; i<size; i++) {
    if (map[i] == node) return i;
  }
  return -1;
}

GridField *gfH(ElcircHeader &h) {
  Node nodes[4];
  
  CellArray *elements = new CellArray();
  for (int i=0; i<h.ne; i++) {
    for (int j=0; j<h.etype[i]; j++) {
      nodes[j] = (Node) h.icon[j][i];
    }
    elements->addCellNodes(nodes, h.etype[i]);
  }
    
  Grid *G = new Grid("gfGeo", 2);
  G->setImplicit0Cells(h.np);
  G->setKCells(elements, 2);
  //cout << "checkwellformed...\n";typedef map<Node, int> NodeMap;
  //int wf = G->checkWellFormed();
  //cout << "Wellformed? " << wf << "\n";
 
  string name("x");
  Array *x = new Array(name.c_str(), FLOAT);
  x->copyData(h.x.x, h.np);
  name = "y";
  Array *y = new Array(name.c_str(), FLOAT);
  y->copyData(h.y.y, h.np);
  name = "h";
  Array *d = new Array(name.c_str(), FLOAT);
  d->copyData(h.d, h.np);
  name = "b";
  Array *b = new Array(name.c_str(), INT);
  b->copyData(h.bi, h.np);

  GridField *GF = new GridField(G, 0);
  GF->Bind(x);
  GF->Bind(y);
  GF->Bind(d);
  GF->Bind(b);
    
  return GF;
}

ElcircHeader *makeHeader(GridField *GF, ElcircHeader *h) {
  ElcircHeader out;

  strcpy(out.magic,"DataFormat v2.0 (from GridFied)");
  strcpy(out.version, "GridFields v0.1");
  strcpy(out.start_time, h->start_time);
  strcpy(out.variable_nm, h->variable_nm);
  strcpy(out.variable_dim, h->variable_dim);
  out.nsteps = h->nsteps;
  out.timestep = h->timestep;
  out.skip = h->skip;
  out.ivs = h->ivs;
  out.i23d = h->i23d;
  out.vpos = h->vpos;
  out.zmsl = h->zmsl;
  out.nvrt = h->nvrt;
  out.nitems = h->nitems; //need to compute this!
  out.hsize = h->hsize;
  out.ssize = h->ssize;
  out.zcor = h->zcor;
  out.np = GF->grid->cellCount(0);
  out.ne = GF->grid->cellCount(2);
  
  GF->getAttribute("x")->getData(out.x.x);
  GF->getAttribute("y")->getData(out.y.y); 
  GF->getAttribute("h")->getData(out.d); 
  GF->getAttribute("b")->getData(out.bi); 
  
  out.no = h->no;
  out.etype = (int *) Malloc(sizeof(int)*out.ne);
  for (int k=0; k<out.ne; k++) out.etype[k] = 3;
  
  Node *nodes;
  for (int j=0; j<4; j++)
    out.icon[j] = (int *) Malloc(sizeof(int)*out.ne);

  AbstractCellArray *cells = GF->grid->getKCells(2);
  for (int i=0; i<out.ne; i++) {
    nodes =  cells->getCellNodes(i);
    for (int j=0; j<out.etype[i]; j++) {
      out.icon[j][i] = nodes[j];
    }
  }
  
  //ElioPrintHeader(*h);
  return h; 
}


GridField *gfV(ElcircHeader &h) {

  OneGrid *G = new OneGrid("V", h.nvrt);
  Array *z = new Array("z", FLOAT);
  //   print();i
  for (int i=0; i<h.nvrt; i++) {
    h.zcor[i] = h.zcor[i];
  }
  z->copyData(h.zcor, h.nvrt);
  GridField *V = new GridField(G, 0, z);
  return V;
}

