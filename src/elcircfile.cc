#include <fstream>
#include <stdio.h>
#include "array.h"
#include "access.h"
#include "grid.h"
#include "onegrid.h"
#include "gridfield.h"
#include "dataset.h"
#include "elcircfile.h"
#include <sstream>
#include "tuple.h"
#include "elio.h"
#include <string.h>
#include <iostream>

using namespace std;

ElcircFile::ElcircFile(string fname) {
  this->filename = fname;
  char *fn = (char *) fname.c_str();
  DEBUG << "CONSTRUCTING ELIO HEADER" << endl;
  int code = ElioGetHeader(fn, &this->h);
  if (code == ELIO_OK) {
    this->valid = true;
  } else {
    this->valid = false;
    Fatal("Elio library returned an error: %i", code);
  }
}

ElcircFile::~ElcircFile() {
  DEBUG << "FREEING ELIO HEADER" << endl;
  if (this->Valid()) {
    ElioFreeHeader(&this->h);
  }
}

ArrayReader *ElcircFile::getSurfReader(int timestep, string posattr="") {
  //cout << "ssize: " << this->h.ssize << endl;
  int offset = this->h.hsize + timestep*this->h.ssize + sizeof(int) + sizeof(float);
  FileArrayReader *ar = new FileArrayReader(this->filename, offset, posattr);  return ar;
}

/*
void ElcircFile::getSurfReader(Array *surf, int timestep, string posattr="") {
  //cout << "ssize: " << this->h.ssize << endl;
  int offset = this->h.hsize + timestep*this->h.ssize + sizeof(int) + sizeof(float);

  MMapIterator mi(this->filename, offset);
  PrimitiveIterator<int> pi(mi);

  surf->fill(pi); 
}
*/

string ElcircFile::getVarScheme() {
  const string &var = string(h.variable_nm);
  string str1=var.c_str();
  if (same(var,"salinity in psu")) str1= "salt:f";
  if (same(var,"salt.63")) str1= "salt:f";
  if (same(var,"horizontal velocity")) str1= "u:f, v:f";
  if (same(var,"hvel.64")) str1= "u:f, v:f";
  if (same(var,"temperature in C")) str1= "temp:f";
  if (same(var,"temp.63")) str1= "temp:f";
  if (same(var,"vertical velocity")) str1= "w:f";
  if (same(var,"vert.63")) str1= "w:f";
  if (same(var,"diffusivity for transport")) str1= "tdiff:f";
  if (same(var,"tdff.63")) str1= "tdiff:f";
  if (same(var,"air temperature")) str1= "airtemp:f";
  if (same(var,"airt.61")) str1= "airtemp:f";
  if (same(var,"z coordinates")) str1= "z:f";
  if (same(var,"zcor.63")) str1= "z:f";
  if (same(var,"surface elevation")) str1= "elev:f";
  if (same(var,"elev.61")) str1= "elev:f";
  if (same(var,"fluxsu")) str1= "fluxsu:f";
  if (same(var,"flsu.61")) str1= "fluxsu:f";
  if (same(var,"fluxlu")) str1= "fluxlu:f";
  if (same(var,"fllu.61")) str1= "fluxlu:f";
  if (same(var,"atmospheric pressure")) str1= "pressure:f";
  if (same(var,"pres.61")) str1= "pressure:f";
  if (same(var,"hradd")) str1= "hradd:f";
  if (same(var,"radd.61")) str1= "hradd:f";
  if (same(var,"hradu")) str1= "hradu:f";
  if (same(var,"radu.61")) str1= "hradu:f";
  Fatal("No scheme found for variable %s.", var.c_str());
return str1;
}

ArrayReader *ElcircFile::getVariableReader(const string &variable, 
                                   int timestep, const string &posattr) {
  int offset = this->getVariableOffset(timestep, 0, 0);
  FileArrayReader *ar = new ProjectArrayReader(this->filename, offset, posattr, getVarScheme(), variable);
  return ar;
}

int ElcircFile::getTimestepSize() {
  return h.ssize;
}
int ElcircFile::getHeaderSize() {
  return h.hsize;
}

int ElcircFile::getSurfOffset(int timestep, int hpos) {
    int offset = this->h.hsize + timestep*this->h.ssize + sizeof(int) + sizeof(float);
    offset += hpos*sizeof(int);
    return offset;
}

int ElcircFile::getVariableOffset(int timestep, int hpos, int vpos) {
    int offset = this->h.hsize + timestep*this->h.ssize + sizeof(int) + sizeof(float);
    offset += this->h.np *sizeof(int);
    if (this->h.i23d == 2) {
      offset += hpos*this->h.ivs;
      return offset;
    }
    if (this->h.v != 4) {
      offset += sizeof(float) * (this->h.no[hpos] + vpos);
    } else {
      offset += sizeof(float) * (hpos * this->h.nvrt + vpos);
    }
    //cout << hpos << ", " <<  vpos << ", " << this->h.v << ", " << this->h.bi[hpos] << ", " << offset << endl;
    return offset;
}

int ElcircFile::newid(int node, int *map, int size) {
  for (int i=0; i<size; i++) {
    if (map[i] == node) return i;
  }
  return -1;
}

GridField *ElcircFile::readDGrid() {
  Grid *gD = new UnitGrid();
  GridField *D = new GridField(gD);
  gD->unref();

  Array *as[18];
  
/*! Number of time steps skipped */
  as[0] = new Array("skip", INT);
  as[0]->copyIntData(&h.skip, 1);
/*! Vector or scalar data, values are 1 = scalar or 2 = vector */
  as[1] = new Array("ivs", INT);
  as[1]->copyIntData(&h.ivs, 1);
/*! i23d = 2 => 2d (such as elevations or wind) or i23d = 3 => 3d data (such as velocities and salinity) */
  as[2] = new Array("i23d", INT);
  as[2]->copyIntData(&h.i23d, 1);
/*! 1.0 or 0.5 depending whether variable is defined on the level or half level */
  as[3] = new Array("vpos", FLOAT);
  as[3]->copyFloatData(&h.vpos, 1);
/*! correction to mean sea level */
  as[4] = new Array("zmsl", FLOAT);
  as[4]->copyFloatData(&h.zmsl, 1);
/*! Version 4 sigma */
  as[5] = new Array("ivcor", INT);
  as[5]->copyIntData(&h.ivcor, 1);
/*! Version 4 sigma */
  as[6] = new Array("h0", FLOAT);
  as[6]->copyFloatData(&h.h0, 1);
/*! Version 5 sigma */
  as[7] = new Array("hs", FLOAT);
  as[7]->copyFloatData(&h.hs, 1);
/*! Version 4 sigma */
  as[8] = new Array("hc", FLOAT);
  as[8]->copyFloatData(&h.hc, 1);
/*! Version 4 sigma */
  as[9] = new Array("thetab", FLOAT);
  as[9]->copyFloatData(&h.thetab, 1);
/*! Version 4 sigma */
  as[10] = new Array("thetaf", FLOAT);
  as[10]->copyFloatData(&h.thetaf, 1);
/*! Number of vertical levels. Note: this is set to 1 for 2d variables where the
on-disk header will have the number of levels as in a 3d variable file */
  as[11] = new Array("nvrt", INT);
  as[11]->copyIntData(&h.nvrt, 1);
/*! Number of vertical levels for Z - version 5.0 */
  as[12] = new Array("kz", INT);
  as[12]->copyIntData(&h.kz, 1);
/*! Number of vertical levels for sigma - version 5.0 */
  as[13] = new Array("ks", INT);
  as[13]->copyIntData(&h.ks, 1);
/*! number of elements in a time step */
  as[14] = new Array("nitems", INT);
  as[14]->copyIntData(&h.nitems, 1);
/*! Size of the header on disk */
  as[15] = new Array("hsize", INT);
  as[15]->copyIntData(&h.hsize, 1);
/*! Number of nodes in the grid */
  as[16] = new Array("np", INT);
  as[16]->copyIntData(&h.np, 1);
/*! Number of elements in the grid */
  as[17] = new Array("ne", INT);
  as[17]->copyIntData(&h.ne, 1);
 
  Dataset ds(1);
  
  for (int i=0; i<18; i++) {
    ds.AddAttribute(as[i]);
    as[i]->unref();
  }

  D->Zip(0, ds);
  return D;
}

GridField *ElcircFile::readHGrid() {
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
  GridField *H = new GridField(G);
  G->unref();
  //cout << "checkwellformed...\n";typedef map<Node, int> NodeMap;
  //int wf = G->checkWellFormed();
  //cout << "Wellformed? " << wf << "\n";

  string name("x");
  Array *x = new Array(name.c_str(), FLOAT, h.np);
  x->copyFloatData(h.x, h.np);
  name = "y";
  Array *y = new Array(name.c_str(), FLOAT, h.np);
  y->copyFloatData(h.y, h.np);
  name = "h";
  Array *d = new Array(name.c_str(), FLOAT, h.np);
  d->copyFloatData(h.d, h.np);
  name = "b";
  Array *b = new Array(name.c_str(), INT, h.np);
  b->copyIntData(h.bi, h.np);

  Dataset ds(h.np);
  ds.AddAttribute(x);
  ds.AddAttribute(y);
  ds.AddAttribute(d);
  ds.AddAttribute(b);
   
  x->unref();
  y->unref();
  b->unref();
  d->unref();

  H->Zip(0, ds);
  //cout << "REFCOUNTS: " << x->refcount << ", " << y->refcount << ", " << b->refcount << endl;
  return H;
}

GridField *ElcircFile::readTGrid() {
  int nsteps = ElioGetNStepsInFile((char *) filename.c_str(), &this->h);
  //int *offset = new int[nsteps];

  int startpos = this->h.hsize;(void)startpos;
  
  Grid *g = (Grid *) new OneGrid("Time", nsteps);
  GridField *result = new GridField(g);
  g->unref();

  for (int i=0; i<nsteps; i++) {
    //offset[i] = i*this->h.ssize/sizeof(int);
  }
 
  Dataset ds(nsteps);
  
  Array *tstamp = new Array("tstamp", FLOAT, nsteps);
  float *stampdata;
  tstamp->getData(stampdata);
  Array *tstep = new Array("tstep", FLOAT, nsteps);
  float *stepdata;
  tstep->getData(stepdata);

  ElcircTimeStep t;(void)t;
  //ElioAllocateTimeStep(&this->h, &t);
  FILE *fp = fopen(filename.c_str(), "r");(void)fp;
  for (int i=0; i<nsteps; i++) {
    //ElioGetTimeStep(fp, i, &h, &t);
    stampdata[i] = h.timestep*(i+1);
    stepdata[i] =  i; //t.it;
  }
/*
  MMapIterator fmi(this->filename, startpos);
  PrimitiveIterator<float> fpi(fmi);
  SliceIterator<float> fsi(fpi, startpos, h.ssize*nsteps/sizeof(int), h.ssize/sizeof(int));
  tstamp->fill(fsi);

  MMapIterator mi(this->filename, startpos+sizeof(float));
  PrimitiveIterator<int> pi(mi);
  SliceIterator<int> si(pi, startpos, h.ssize*nsteps/sizeof(int), h.ssize/sizeof(int));
  tstep->fill(si);
  */
  ds.AddAttribute(tstamp);
  ds.AddAttribute(tstep);
  
  tstamp->unref(); 
  tstep->unref(); 
  
  result->Zip(0, ds);
  return result;
}

GridField *ElcircFile::readVGrid() {
  OneGrid *G = new OneGrid("V", h.nvrt);
  GridField *V = new GridField(G);
  G->unref();
  
  Array *z = new Array("z", FLOAT, h.nvrt);
  z->copyFloatData(h.zcor, h.nvrt);

  Dataset ds(h.nvrt);
  ds.AddAttribute(z);
  z->unref();

  V->Zip(0, ds);
  return V;
}


ElcircHeader *ElcircFile::makeHeader(GridField *GF, ElcircHeader *h) {
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
  out.np = GF->GetGrid()->Size(0);
  out.ne = GF->GetGrid()->Size(2);
  
  GF->GetAttribute(0, "x")->getData(out.x);
  GF->GetAttribute(0, "y")->getData(out.y); 
  GF->GetAttribute(0, "h")->getData(out.d); 
  GF->GetAttribute(0, "b")->getData(out.bi); 
  
  out.no = h->no;
  out.etype = new int[sizeof(int)*out.ne];  //(int *) malloc(sizeof(int)*out.ne);
  for (int k=0; k<out.ne; k++) out.etype[k] = 3;

  Node *nodes;
  for (int j=0; j<4; j++)
    out.icon[j] = new int[sizeof(int)*out.ne]; //(int *) malloc(sizeof(int)*out.ne);

  AbstractCellArray *cells = GF->GetGrid()->getKCells(2);
  for (int i=0; i<out.ne; i++) {
    nodes =  cells->getCellNodes(i);
    for (int j=0; j<out.etype[i]; j++) {
      out.icon[j][i] = nodes[j];
    }
  }
  //ElioPrintHeader(*h);
  return h; 
}


