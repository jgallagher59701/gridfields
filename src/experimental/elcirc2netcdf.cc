#include "gridfield.h"
#include "expr.h"
#include "timing.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include "array.h"
#include "tuple.h"

#include <netcdfcpp.h>

#define MAXCELLVERTEX 4

using namespace std;

NcType mapType(Type t) {
  switch (t) {
    case FLOAT:
      return ncFloat;
    case INT:
      return ncInt;
    default:
      Fatal("Unknown Type encountered during netCDF emission");
  }
}

void WriteTimeVars(GridFieldOperator *op, int timestep) {
  GridField *gf = op->getResult();
  Scheme tsch("salt:f,temp:f,u:f,v:f");
  Array *attr;
  NcVar *var;
  NcDim *nodedim = ncdf->get_dim("node");
  if (!nodedim) {
    Fatal("Unable to find dimension 'node'");
  }
  for (int k=0; k<tsch.size(); k++) {
    attr = gf->GetAttribute(0, tsch.getAttribute(k));
    var = ncdf->get_var(attr->sname().c_str());
    long *foo = var->edges();
    var->set_cur(timestep, 0);
    var->put((float *) attr->getVals(), 1, nodedim->size());
  }
  ncdf->sync();
}

void OutputNetCDFOp::WriteNetCDF(vector<GridField *> &dims, GridField *cross, 
		                  const string &filename) {

   GridField *gf = cross;
   // only works for 2-D gridfields
   assert(gf->Dim() == 2);
   assert(gf->IsAttribute(0, "x"));
   assert(gf->GetAttribute(0, "x")->type == FLOAT);
   assert(gf->IsAttribute(0, "y"));
   assert(gf->GetAttribute(0, "y")->type == FLOAT);
   
   vector<GridField *>::iterator p;
   
   ncdf = new NcFile(filename.c_str(), NcFile::Replace); 
   
   // write metadata
   // add dims
   NcDim *celldim = ncdf->add_dim("cell", gf->Card(2));
   NcDim *nodedim = ncdf->add_dim("node", gf->Card(0));
 
   // an unlimited dimension
   NcDim *timedim = ncdf->add_dim("time");

   // maximum number of cell vertices
   NcDim *cellvertexdim = ncdf->add_dim("nv", MAXCELLVERTEX);

   // write cells
   NcVar *cellvertices = ncdf->add_var("cell_vertices", ncInt, celldim, cellvertexdim);
   int cd = celldim->size();
   int cv = cellvertexdim->size();
   AbstractCellArray *ca = gf->GetGrid()->getKCells(2);
   int *nodes; 
   for (int i=0; i<celldim->size(); i++) {
     Cell *c = ca->getCell(i);

     assert(c->getsize() <= MAXCELLVERTEX);
     nodes = c->getnodes(); 
     cellvertices->set_cur(i, 0);
     
     if (!cellvertices->put(nodes, 1, c->getsize())) {
       Fatal("Error writing netCDF file");
     }
   }
   
   // write nodes
   NcVar *x = ncdf->add_var("x", ncFloat, nodedim);
   x->put((float *) gf->GetAttribute(0, "x")->getVals(), nodedim->size());
   NcVar *y = ncdf->add_var("y", ncFloat, nodedim);
   y->put((float *) gf->GetAttribute(0, "y")->getVals(), nodedim->size());
   
   Array *attr;
   NcVar *var;

   // write fixed data
   const Scheme &fsch = this->fixed;
   for (int k=0; k<fsch.size(); k++) {
     attr = gf->GetAttribute(0, fsch.getAttribute(k));
     var = ncdf->add_var(attr->sname().c_str(), mapType(attr->type), nodedim);
//     if (!var->add_attr(")) {
//       Fatal("Error adding attribute to netCDF variable");
//     }
   }

   // prepare time-varying data
   const Scheme &tsch = this->time;
   string name;
   for (int k=0; k<tsch.size(); k++) {
     name = tsch.getAttribute(k);
     var = ncdf->add_var(name.c_str(), mapType(tsch.getType(k)), timedim, nodedim);
   }

   // write data last so we don't need to return to define mode
   for (int k=0; k<fsch.size(); k++) {
     var->put((float *) attr->getVals(), nodedim->size());
   }
   
   ncdf->sync();
}


/*   
void OutputNetCDFOp::WriteNetCDF(vector<GridField *> &dims, GridField *cross, 
		                  const string &filename) {
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
  */

NcVar *OutputNetCDFOp::putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount) {
	/*
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
  */
}
/*
void OutputNetCDFOp::writeName(string name, ofstream &f) {
   int s = name.size();
   f.write((char *) &s, sizeof(int));
   f.write(name.c_str(), name.size());
}
*/

