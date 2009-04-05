#include <fstream>
#include <stdio.h>
#include <map>
#include "gridwriter.h"
#include "gridfield.h"
#include "array.h"
#include "ordmap.h"

using namespace std;

GridWriter::GridWriter(string fn, long off) {
  filename = fn;
  offset = off;
}

void GridWriter::prepfile() {
  //prepfile is idempotent
  if (this->outstream != NULL) return;
  
  ofstream f;
  long off;
  
  Grid *G = Gf->grid;
  f.open((const char *) filename.c_str(), ofstream::app | ofstream::out);
  if (f.fail()) { 
    cerr << "Unable to open " << filename << endl;
    exit(1);
  }
  
  long start = f.tellp();
  cout << "seekto " << start + offset << endl;
  f.seekp(start + offset); 

  this->outstream = f;
}

void GridWriter::Write(Grid *G) {
  this->prepfile();
  int s = G->dim();
  this->outstream.write((char *) &s, sizeof(int));
      
  CellArray *ca;
  for (int k=0; k<=G->dim(), k++) {
    ca = G->getKCells(k);
    this->Write(ca);
  }
}

void GridWriter::Write(CellArray *ca) {
  this->prepfile();
  int s = ca->getsize();
  this->outstream.write((char *) &s, sizeof(int));
  for (int i=0; i<s; i++) {
    this->Write(ca->getCell(i));
  }
}

void GridWriter::Write(Cell *c) {
  this->prepfile();
  int s = c->size;
  this->outstream.write(c->nodes, sizeof(int)*s);
}

