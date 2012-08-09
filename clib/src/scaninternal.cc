
#include "config.h"

#include "gridfield.h"
#include <stdio.h>
//extern "C" {
//#include "stdio.h"
//}
#include "expr.h"
#include "timing.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include "array.h"
#include "scaninternal.h"
#include "implicit0cells.h"
#include "arrayreader.h"

using namespace std;

namespace GF {

ScanInternal::ScanInternal(string fn, long offset) : ScanOp(fn, offset) {
  this->bytestream = new ifstream(filename.c_str(), ios_base::binary | ios::in);
}

void ScanInternal::setFileName(char *fn) {
  this->filename = string(fn);
  delete this->bytestream;
  this->bytestream = new ifstream(filename.c_str(), ios_base::binary | ios::in);
  this->Update();
}

ScanInternal::ScanInternal(string rawbytes) {
  this->bytestream = new istringstream(rawbytes, ios::binary | ios::in);
}

void ScanInternal::setRawBytes(string rawbytes) {
  this->filename = "";
  delete this->bytestream;
  this->bytestream = new istringstream(rawbytes, ios::binary | ios::in);
  this->Update();
}

void ScanInternal::Execute() {
  //this->PrepareForExecution();
  DEBUG << "ScanInternal Operator" << endl;
  this->Result =  this->Scan(*this->bytestream);
}

GridField *ScanInternal::Scan(istream &str) {
  /*
   * internal representation:
   *  magic : 'GFGRID'
   *  gridnamelength : i
   *  gridname : gridnamelength*(char : c)
   *  dim : i
   *  implicitflag : i
   *  implicitflag==0 ? (
   *    zerocellcount : i
   *    zerocells : zerocellcount*(id : i)
   *  ) ( 
   *    zerocellcount : i
   *  )
   *  kcelldata : dim*(
   *    kcellcount : i
   *    kcells : kcellcount*(
   *      size : i
   *      nodes : size*(node : i)
   *    )
   */

   
   if (!str.good()) {
     Fatal("Cannot read from stream. (not good())");
   }
   char magic[7] = "xxxxx\0";
   str.read(magic, 6);
   if (string(magic) != "GFGRID") {
     Fatal("Scan target is not a grid: (%s)", magic);
   }
  
   Grid *G = readGrid(str);
   int k;(void)k;
   //cout << k << endl;
   DEBUG << "ScanInternal: Grid refcount before reading gridfield:" << G->refcount << endl;
   GridField *GF = readGridField(G, str);
   G->unref();
   //GF->print();

   return GF;
}

Grid *ScanInternal::readGrid(istream &f) {
   //cout << "name :";
   string name = readName(f);
   //cout << name << endl;
   
   int dim;
   f.read((char *) &dim, sizeof(int));
   //cout << dim << endl;
   Grid *G = new Grid(name, dim);

   //read zerocells
   int implicitflag;
   f.read((char *) &implicitflag, sizeof(int));
   int nodecount;
   f.read((char *) &nodecount, sizeof(int));
   DEBUG << nodecount << endl;
   
   AbstractCellArray *zerocells;
   if (!implicitflag) {
     Node *nodes = new Node[nodecount];
     f.read((char *) nodes, nodecount*4);

     zerocells = (AbstractCellArray *) new CellArray(nodes, nodecount, 1);
   } else {
     zerocells = (AbstractCellArray *) new Implicit0Cells(nodecount);
   }

   G->setKCells(zerocells, 0);

   for (int i=1; i<=dim; i++) {
     G->setKCells(readCellArray(f), i);
   }
     
   return G;
  }

  GridField *ScanInternal::readGridField(Grid *G, istream &f) {
    int k;
    GridField *GF = new GridField(G);
    for (int i=0; i<G->getdim(); i++) {
    f.read((char *) &k, sizeof(int));
    readDataset(GF, i, f);
  }
  return GF;
}

void ScanInternal::readDataset(GridField *GF, int k, istream &f) {
   int arity;
   Type t;
   f.read((char *) &arity, sizeof(int));
   //cout << "arity: " << arity << endl;
   ArrayReader ar(&f);
   Array *a;
   for (int i=0; i<arity; ++i) {
     string name = readName(f);
     f.read((char *) &t, sizeof(Type));
     
     a = new Array(name, t, GF->Card(k));
     ar.setOffset(f.tellg());
     ar.Read(GF, k, a);
     GF->Bind(k, a);
     a->unref();
     //a->print();
   }
}


CellArray *ScanInternal::readCellArray(istream &f) {
   int bytesize;
   int size;
   int s;(void)s;
   
   f.read((char *) &size, sizeof(int));
   f.read((char *) &bytesize, sizeof(int)); 

   if (size == 0) {
     return new CellArray();
   } 
   
   int *data = new int[bytesize];
   f.read((char *) data, bytesize);

   Node *nodes = new Node[bytesize];
   for (int i=0; i<bytesize; i++) {
     nodes[i] = data[i];
   }

   CellArray *ca = new CellArray(nodes, size);

   delete [] data;

   return ca;
}

string ScanInternal::readName(istream &f) {
   int namesize=0;
   f.read((char *) &namesize, sizeof(int));
   //cout << "namesize: " << namesize << endl;
   char *name = new char[namesize+1];
   f.read(name, namesize);
   name[namesize] = '\0';
   string s(name);
   //cout << s.c_str() << endl;
   delete [] name;
   return s;
}

} // namespace GF

