extern "C" {
#include "stdio.h"
#include "elio.h"
}
#include <iostream>
#include <iomanip>
#include <fstream>
#include "gridfield.h"
#include "array.h"
#include "type.h"
#include "write63.h"

using namespace std;
char SPACE = ' ';
ofstream *CORIEWriter::prepfile(string filename, int offset) {
  //prepfile is idempotent
  static ofstream f;
  if (f.is_open()) return &f;

  long off;

  f.open((const char *) filename.c_str(), ofstream::out | ofstream::binary);
  if (f.fail()) {
    cerr << "Unable to open " << filename << endl;
    exit(1);
  }

  long start = f.tellp();
  f.seekp(start + offset);

  return &f;
}

void CORIEWriter::writeTime(int step, float stamp, ofstream &f) {
  f.write((char *) &stamp, sizeof(float));
  f.write((char *) &step, sizeof(int));
}

void CORIEWriter::writeHGrid_bin(GridField *H, ofstream &f) {
  /* assumes H is a gridfield over 0-cells,
   * with attributes x:float,y:float,h:float,b:int.
   * The grid must also have 2-cells
   */
  
  Cell *c; 
  int s = H->card();
  
  H->grid->normalize();

  AbstractCellArray *twocells = H->grid->getKCells(2);
  int s2 = twocells->getsize();
  
  //write number of nodes, number of 2-cells
  f.write((char *) &s, sizeof(int));
  f.write((char *) &s2, sizeof(int));
  
  //write (node, x, y, h, b)
  AbstractCellArray *nodes = H->grid->getKCells(0);
  Array *x = H->getAttribute("x");
  Array *y = H->getAttribute("y");
  Array *h = H->getAttribute("h");
  Array *b = H->getAttribute("b");
  int bi;
  for (int i=0; i<s; i++) {
    c = nodes->getCell(i);
    //f.write((char *)&(c->nodes[0]), sizeof(int));
    f.write((char *)(x->getValPtr(i)), sizeof(float));
    f.write((char *)(y->getValPtr(i)), sizeof(float));
    f.write((char *)(h->getValPtr(i)), sizeof(float));
    //elcirc output uses 1-based arrays
    bi = (*((int*) b->getValPtr(i))) + 1;
    f.write((char *)&bi, sizeof(int));
  }
 
  int n;
  //write 2cells: (size, [node])
  for (int i=0; i<s2; i++) {
    c = twocells->getCell(i);
    f.write((char *) &(c->size), sizeof(int));
    for (int j=0; j<c->size; j++) {
      //elcirc output uses 1-based arrays
      n = c->nodes[j] + 1;
      f.write((char *) &n, sizeof(int));
    }
  }
}

void CORIEWriter::writeVGrid_bin(GridField *V, ofstream &f) {
  /* assumes V is a gridfield over 0-cells, 
   * with attribute z:float
   */
  int s = V->card();
  f.write((char *) &s, sizeof(int));
  Array *z = V->getAttribute("z");
  for (int i=0; i<V->card(); i++) {
    f.write((char *)(z->getValPtr(i)), sizeof(float));
  }
  
}

void CORIEWriter::writeTimeStep(GridField *HxV, ofstream &f) {

  add one to header
  writeTime(step, val)
  writeSurf()
  writeAttributes([attr]):
    for h in H:
     for v in V:
       for each attr:
         write attr
}

void CORIEWriter::writeHGrid_txt(GridField *H, ofstream &f) {
  /* assumes H is a gridfield over 0-cells,
   * with attributes x:float,y:float,h:float,b:int.
   * The grid must also have 2-cells
   * The grid must be normalized
   */
  
  Cell *c; 
  int s = H->card();
  
  AbstractCellArray *twocells = H->grid->getKCells(2);
  int s2 = twocells->getsize();
  f.precision(11); 
  //write number of nodes, number of 2-cells
  f << s << SPACE << s2 << endl;
  
  //write (node, x, y, h, b)
  AbstractCellArray *nodes = H->grid->getKCells(0);
  Array *x = H->getAttribute("x");
  Array *y = H->getAttribute("y");
  Array *h = H->getAttribute("h");
  Array *b = H->getAttribute("b");
  for (int i=0; i<s; i++) {
    c = nodes->getCell(i);
    f << c->nodes[0] << SPACE;
    f << *(float *)(x->getValPtr(i)) << SPACE;
    f << *(float *)(y->getValPtr(i)) << SPACE;
    f << *(float *)(h->getValPtr(i)) << SPACE;
//    f << *(int *)(b->getValPtr(i)) << SPACE;
    f << endl;
  }
  
  //write 2cells: (ord, size, [node])
  for (int i=0; i<s2; i++) {
    c = twocells->getCell(i);
    f << c->size << SPACE;
    for (int j=0; j<c->size; j++) {
      f << c->nodes[j] << SPACE;
    }
    f << endl;
  }
}

void CORIEWriter::writeVGrid_txt(GridField *V, ofstream &f) {
  /* assumes V is a gridfield over 0-cells, 
   * with attribute z:float
   */
  Array *z = V->getAttribute("z");
  for (int i=0; i<V->card(); i++) {
    f << *(float *)(z->getValPtr(i)) << endl;
  }
  
}

void CORIEWriter::copyHeader(string infile, ofstream &f) {
  ElcircHeader h;
  ElioGetHeader((char *) infile.c_str(), &h);
  writeHeader(h, f);
}

void CORIEWriter::writeHeader(ElcircHeader &h, ofstream &f) {
  int i;

  //fix a hack in elio.c
  h.magic[15] = ' ';
  f.write((char *) h.magic, 48);
  f.write((char *) h.version, 48);
  f.write((char *) h.start_time, 48);
  f.write((char *) h.variable_nm, 48);
  f.write((char *) h.variable_dim, 48);
  f.write((char *) &(h.nsteps), sizeof(int));
  f.write((char *) &(h.timestep), sizeof(float));
  f.write((char *) &(h.skip), sizeof(int));
  f.write((char *) &(h.ivs), sizeof(int));
  f.write((char *) &(h.i23d), sizeof(int));
  f.write((char *) &(h.vpos), sizeof(float));

  f.write((char *) &(h.zmsl), sizeof(float));

  return;
}
