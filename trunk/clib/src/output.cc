
#include "config_gridfields.h"

#include "gridfield.h"
#include <stdio.h>
//extern "C" {
// #include "stdio.h"
#include "elio.h"
//}
#include "expr.h"
#include "timing.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include "array.h"
#include "tuple.h"
#include "output.h"
#include "implicit0cells.h"
#include "arraywriter.h"

//#include <netcdfcpp.h>

using namespace std;

namespace GF {

OutputOp::OutputOp(string fn, GridFieldOperator *op)  
 : UnaryGridFieldOperator(op), offset(0), filename(fn) {
   GF = NULL;
   //cleanup = false;
}

OutputOp::OutputOp(string fn, long off, GridFieldOperator *op)  
 : UnaryGridFieldOperator(op), offset(off), filename(fn) {
   //cleanup = false;
   GF = NULL;
}


void OutputOp::Execute() {
  this->PrepareForExecution();
  this->Output(this->GF, this->filename, this->offset);
  this->Result = this->GF;
}
/*
void OutputOp::WriteNetCDF(vector<GridField *> &dims, GridField *cross) {

   vector<GridField *>::iterator p;
   
   NcFile *ncdf = new NcFile("./test.cdf", NcFile::Replace); 
   
   //put the dims
   NcDim *d[dims.size()];
   long counts[dims.size()];
   int expectedcard = 1;
   GridField *gf;
   string attr;
   
   for (int i=0; i<dims.size(); i++) {
      gf = dims[i];
      attr = gf->getScheme()->getAttribute(0);
      counts[i] = gf->card();
      expectedcard *= gf->card();
      d[i] = ncdf->add_dim(gf->getAttribute(attr.c_str())->name, gf->card());
   }
   
   //put the dim variables
   NcVar *var;
   Array *a;
   Scheme *sch;
   NcType t;
   long size;
   for (int i=0; i<dims.size(); i++) {
     gf = dims[i];
     sch = gf->getScheme();
     for (int j=0; j<gf->getarity(); j++) {
       attr = sch->getAttribute(j);
       cout << "(i,j)=" << i << ", " << j << " attr: " << attr << endl;
       a = gf->getAttribute(attr.c_str());
       size = (long) a->size;
       OutputOp::putData(a, ncdf, &size, &(d[i]), 1); 
     }
   }
   
   //sanity check
   if (cross->card() != expectedcard) {
     Fatal("product grid cardinality different than product of component grid cardinalities. (%i, %i)", cross->card(), expectedcard);
   }
   
   //put the cross variables
   delete sch;
   sch = cross->getScheme();
   for (int j=0; j<cross->getarity(); j++) {
     attr = sch->getAttribute(j);
     cout << "(j)=" << ", " << j << " attr: " << attr << endl;
     a = cross->getAttribute(attr.c_str());
     OutputOp::putData(a, ncdf, counts, d, dims.size()); 
   }
   

   delete ncdf;
   
  //Write each dim, using first attribute as 'name'
  //
  //Write each dim's arrays
  //
  //Write each cross attribute
  
}

NcVar *OutputOp::putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount) {
  Type t = a->type;
  NcVar *var = NULL;
  UnTypedPtr p;
  cout << a->name << ", " << a->size << endl; 
  switch (t) {
    case INT:
      var = ncdf->add_var(a->name, ncInt, dimcount, (const NcDim **) d);
      a->getData((int *) p);
      var->put((int *) p, counts);
      break;
    case FLOAT:
      var = ncdf->add_var(a->name, ncFloat, dimcount, (const NcDim **) d);
      a->getData((float *) p);
      var->put((float *) p, counts);
      break;
    default:
      break;
  }
  return var;
}
*/
void OutputOp::Output(GridField *GF, string filename, long ) {
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
   ofstream f(filename.c_str(), ios::binary | ios::out);
    
   char magic[7] = "GFGRID";
   f.write(magic, 6);
   writeGrid(GF->GetGrid(), f);
   writeGridField(GF, f);
   f.flush();
   f.close();
   GF->ref();
}

void OutputOp::writeGrid(Grid *G, ofstream &f) {
  
   writeName(G->name, f);
   
   int dim = G->getdim();
   f.write((char *) &dim, sizeof(int));
   int implicitflag = G->getKCells(0)->implicit();
   f.write((char *) &implicitflag, sizeof(int));

   int nodecount = G->getKCells(0)->getsize();
   f.write((char *) &nodecount, sizeof(int));
   
   if (!implicitflag) {
     
     AbstractCellArray *ca = G->getKCells(0);
     Node *ns;
     for (unsigned int i=0; i<ca->getsize(); ++i) {
       ns = ca->getCellNodes(i);
       //f.write((char *) &(c->size), sizeof(int));
       f.write((char *) ns, sizeof(int));
     }
   } 
   
   for (int i=1; i<=dim; i++) {
     writeCellArray(G->getKCells(i), f);
   }
}

void OutputOp::writeGridField(GridField *GF, ofstream &f) {
  for (int i=0; i<GF->Dim(); i++) {
    f.write((char *) &i, sizeof(int));
    int a = GF->Arity(i);
    f.write((char *) &a, sizeof(int));
    if (GF->Arity(i)) {
      writeDataset(GF->GetDataset(i), f);
    }
  }
}
void OutputOp::writeDataset(const Dataset &ds, ofstream &f) {
   int arity = ds.Arity();
   
   ArrayWriter aw(&f);
   Array *a;
   const Scheme &sch = ds.GetScheme();
   for (int i=0; i<arity; ++i) {
     a = ds.GetAttribute(sch.getAttribute(i));
     writeName(string(a->getName()), f);
     f.write((char *) &a->type, sizeof(Type));
     //f.write((char *) &a->size(), sizeof(int));
     aw.Write(ds, string(a->getName()));
   }
}


void OutputOp::writeCellArray(AbstractCellArray *ca, ofstream &f) {
   int size = ca->getsize();
   
   f.write((char *) &size, sizeof(int));
   
   int bytesize = ca->bytes();
   f.write((char *) &bytesize, sizeof(int)); 
 
   Cell *c;
   int s;
   for (int i=0; i<size; i++) {
     c = ca->getCell(i);
     s = c->getsize();
     f.write((char *) &s, sizeof(int));
     f.write((char *) c->getnodes(), sizeof(int)*s);
   } 
}

void OutputOp::writeName(string name, ofstream &f) {
   int s = name.size();
   f.write((char *) &s, sizeof(int));
   f.write(name.c_str(), name.size());
}


/*----------------------------------------*/
/* OutputElcircOp
 *
 *
 */ 
 /* 
OutputElcircOp::OutputElcircOp(string fn, ElcircHeader *copy, GridFieldOperator *op)  
 : OutputOp(fn, op), copy(copy) {
   GF = NULL;
}

void OutputElcircOp::Output(GridField *GF, string filename, long offset) {
  h.magic = copy->magic;
  h.version = copy->version;
  ElioAllocateHeader(ElcircHeader *h);
  cout << "elcircoutput" << endl;
  cout << GF->Card(0);
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
}
  */

} // namespace GF

