#include "gridfield.h"
extern "C" {
#include "stdio.h"
#include "elio.h"
}
#include "expr.h"
#include "timing.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include "array.h"
#include "tuple.h"
#include "string.h"
#include "tonetcdf.h"
#include "implicit0cells.h"
#include "arraywriter.h"

#include <netcdfcpp.h>
//#include <proj_api.h>

#define MAXCELLVERTEX 4
//#define DEBUG cout

using namespace std;

OutputNetCDFOp::OutputNetCDFOp(string fn, GridFieldOperator *op, 
		               const Scheme f, const Scheme t)  
 : UnaryGridFieldOperator(op), filename(fn), fixed(f), time(t),  ncdf(NULL) {
   GF = NULL;
}

void OutputNetCDFOp::SetDate(string ds) {
  this->datestr = ds;
}

void OutputNetCDFOp::Execute() {
  this->PrepareForExecution();
  vector<GridField *> gfs;
  //gfs.push_back(this->GF);
  this->WriteNetCDF(gfs, this->GF, this->filename);
  this->Result = this->GF;
}

NcType mapType(Type t) {NcType nyt=ncInt;
  switch (t) {
    case FLOAT:
      nyt= ncFloat;
      break;
    case INT:
      nyt= ncInt;
      break;
    default:
      Fatal("Unknown Type encountered during netCDF emission");
      exit(1);
  }
return nyt;
}

void OutputNetCDFOp::WriteTimeVars(GridFieldOperator *op, int index, float timestep) {
  GridField *gf = op->getResult();
  const Scheme &tsch = this->time;
  Array *attr;
  NcVar *var;
  NcDim *nodedim = ncdf->get_dim("node");
  if (!nodedim) {
    Fatal("Unable to find dimension 'node'");
  }
  for (unsigned int k=0; k<tsch.size(); k++) {
    attr = gf->GetAttribute(0, tsch.getAttribute(k));
    //cout << attr->sname() <<", " << endl;
    //attr->print();
    var = ncdf->get_var(attr->sname().c_str());
    //long *foo = var->edges();
    var->set_cur(index, 0);
    var->put((float *) attr->getVals(), 1, nodedim->size());
  }
  var = ncdf->get_var("time");
  var->set_cur(index);
  var->put(&timestep, 1);
  ncdf->sync();
}

void addAttributes(NcVar *var, string datestr) {
  DEBUG << "Adding attributes to " << var->name() << endl;
  NcBool success = true;
  if (strcmp(var->name(), "x") == 0) {
     success &= var->add_att("units", "m");
     success &= var->add_att("long_name", "X coordinate, in meters, epsg:32026, Oregon State Plane Index");
  }
  if (strcmp(var->name(), "y") == 0) {
     success &= var->add_att("units", "m");
     success &= var->add_att("long_name", "Y coordinate, in meters, epsg:32026, Oregon State Plane Index");
  }
  if (strcmp(var->name(), "time") == 0) {
     success &= var->add_att("units", (string("seconds since ") + datestr).c_str());
     success &= var->add_att("long_name", "Time in seconds from midnight");
  }
  if (strcmp(var->name(), "elev") == 0) {
     success &= var->add_att("units", "m");
     success &= var->add_att("positive", "up");
     success &= var->add_att("long_name", "Water elevation in meters above Mean Sea Level");
  }
  if (strcmp(var->name(), "h") == 0) {
     success &= var->add_att("units", "m");
     success &= var->add_att("positive", "down");
     success &= var->add_att("long_name", "bathymetry measured as meters below Mean Sea Level");
  }
  if (strcmp(var->name(), "b") == 0) {
     success &= var->add_att("long_name", "Bottom index: level index indicating the lowest level that is still above the bathymetry; 0-based indexing");
  }
  if (strcmp(var->name(), "salt") == 0) {
     success &= var->add_att("units", "psu");
     success &= var->add_att("long_name", "Salinity in Practical Salinity Units");
     success &= var->add_att("standard_name", "sea_water_salinity");
     success &= var->add_att("missing_value", -99.0f);
     success &= var->add_att("_FillValue", -99.0f);
  }
  if (strcmp(var->name(), "temp") == 0) {
     success &= var->add_att("units", "C");
     success &= var->add_att("long_name", "temperature in degrees celsius");
     success &= var->add_att("missing_value", -99.0f);
     success &= var->add_att("_FillValue", -99.0f);
  }
  if (strcmp(var->name(), "u") == 0) {
     success &= var->add_att("units", "m/s");
     success &= var->add_att("long_name", "velocity in the x direction in meters per second");
     success &= var->add_att("missing_value", 0.0f);
     success &= var->add_att("_FillValue", 0.0f);
  }
  if (strcmp(var->name(), "v") == 0) {
     success &= var->add_att("units", "m/s");
     success &= var->add_att("long_name", "velocity in the y direction in meters per second");
     success &= var->add_att("missing_value", 0.0f);
     success &= var->add_att("_FillValue", 0.0f);
  }
  if (strcmp(var->name(), "cell_vertices") == 0) {
     success &= var->add_att("long_name", "conectivity table; 0-based indices");
  }

  if (strcmp(var->name(), "node") == 0) {
     success &= var->add_att("long_name", "node number; 0-based");
  }

  if (!success) {
    Fatal("Error adding attribute to netCDF variable");
  }
}

void OutputNetCDFOp::WriteNetCDF(vector<GridField *> &, GridField *cross, 
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
   DEBUG << "Adding time dim in WriteNetCDF" << endl;
   NcDim *timedim = ncdf->add_dim("time");

   // maximum number of cell vertices
   NcDim *cellvertexdim = ncdf->add_dim("nv", MAXCELLVERTEX);

   // global attributes
   ncdf->add_att("title", "Hindcast of Columbia River Physical Variables");
   ncdf->add_att("institution", "STC Coastal Margin and Prediction");
   ncdf->add_att("source", "ELCIRC");
   if (this->datestr != "") {
     ncdf->add_att("date", this->datestr.c_str());
   }
   
   // write cells
   NcVar *cellvertices = ncdf->add_var("cell_vertices", ncInt, celldim, cellvertexdim);
//   int cd = celldim->size();
//   int cv = cellvertexdim->size();
   AbstractCellArray *ca = gf->GetGrid()->getKCells(2);
   int *nodes; 
   nodes = new int[MAXCELLVERTEX];
   for (int i=0; i<celldim->size(); i++) {
     Cell *c = ca->getCell(i);

     assert(c->getsize() <= MAXCELLVERTEX);
     
     for (unsigned int j=0; j<c->getsize(); j++) {
       nodes[j] = c->getnodes()[j]; 
     }
   
     cellvertices->set_cur(i, 0);
     
     if (!cellvertices->put(nodes, 1, c->getsize())) {
       Fatal("Error writing netCDF file");
     }

   }
   delete [] nodes;
   
   // write nodes
   NcVar *x = ncdf->add_var("x", ncFloat, nodedim);
   addAttributes(x, this->datestr);
   NcVar *y = ncdf->add_var("y", ncFloat, nodedim);
   addAttributes(y, this->datestr);
   
   NcVar *time = ncdf->add_var("time", ncFloat, timedim);
   DEBUG << "Date: " << this->datestr << endl;
   addAttributes(time, this->datestr);


   Array *attr;
   NcVar *var;

   // write fixed data
   const Scheme &fsch = this->fixed;
   for (unsigned int k=0; k<fsch.size(); k++) {
     attr = gf->GetAttribute(0, fsch.getAttribute(k));
     var = ncdf->add_var(attr->sname().c_str(), mapType(attr->type), nodedim);
     addAttributes(var, this->datestr);
   }

   // prepare time-varying data
   const Scheme &tsch = this->time;
   string name;
   for (unsigned int k=0; k<tsch.size(); k++) {
     name = tsch.getAttribute(k);
     var = ncdf->add_var(name.c_str(), mapType(tsch.getType(k)), timedim, nodedim);
     addAttributes(var, this->datestr);
   }

   // write data last so we don't need to return to define mode
   for (unsigned int k=0; k<fsch.size(); k++) {
     attr = gf->GetAttribute(0, fsch.getAttribute(k));
     var = ncdf->get_var(attr->sname().c_str());
     switch (fsch.getType(k)) {
      case INT:
        var->put((int *) attr->getVals(), nodedim->size());
        break;
      case FLOAT:
        var->put((float *) attr->getVals(), nodedim->size());
        break;
      default:
        Fatal("Cannot convert object types to netcdf types for attribute %s", attr->sname().c_str());
    }

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

NcVar *OutputNetCDFOp::putData(Array *, NcFile *, long *, NcDim **, int ) {

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
NcVar *var=NULL;
return var;}
/*
void OutputNetCDFOp::writeName(string name, ofstream &f) {
   int s = name.size();
   f.write((char *) &s, sizeof(int));
   f.write(name.c_str(), name.size());
}
*/

/* Write a gridfield as a dimension variable or a regular variable*/

OutputNetCDFVars::OutputNetCDFVars(
NcFile *f, 
const Scheme dims, 
Dim_t d, 
GridFieldOperator *Op,
unsigned int i,  //index for the unbounded dimension
float t // value for the unbounded dimension
)
: 
UnaryGridFieldOperator(Op),  
datestr(""),
time(t),
dimscheme(dims),
index(i),
ncdf(f), 
dim(d)

{
   GF = NULL;
};

OutputNetCDFDim::OutputNetCDFDim
(
  NcFile *f, 
  Dim_t d, 
  string dn, 
  GridFieldOperator *Op
) : UnaryGridFieldOperator(Op), 

    datestr(""),
    basedimname(dn),
    ncdf(f), 
    dim(d)
{
   GF = NULL;
};

void OutputNetCDFVars::Execute() {
  this->PrepareForExecution();
  DEBUG << "Exporting gridfield as Vars" << endl;

  GridField *gf = this->GF->getResult();
  const Scheme &dsch = this->dimscheme;

  //Array *attr;
  NcVar *var;
  const NcDim **vdims = new const NcDim *[dsch.size()+1];
  int sz = 1;
  // Dims are in reverse order in C and C++.  Record dimension is "first"
  for (unsigned int k=0; k<dsch.size(); k++) {
    int r = dsch.size() - k;
    string a = dsch.getAttribute(k);
    
    if (HasDim(ncdf, a.c_str())) {
      vdims[r] = ncdf->get_dim(a.c_str());
    } else {
      Fatal("Unable to find dimension %s in netcdf file", a.c_str());
    }
    sz *= vdims[r]->size();
    DEBUG << "DIM: " << vdims[r]->name() << ", " << vdims[r]->size() << endl;
  }
  
  // we always include a time dimension
  // an unlimited dimension

  NcDim *timedim;
  if (HasDim(ncdf, "time")) {
    // if it already has a time dimension, do nothing.
    DEBUG << "Getting time dim" << endl;
    timedim = ncdf->get_dim("time");
  } else {
    DEBUG << "Adding time dim" << endl;
    timedim = ncdf->add_dim("time");
    DEBUG << "Adding time var" << endl;
    // We always add a time var
    NcVar *nctime = ncdf->add_var("time", ncFloat, timedim);
    DEBUG << "Date: " << this->datestr << endl;
    addAttributes(nctime, this->datestr);
  }

  // record dimension is "first"
  vdims[0] = timedim;
  //for (int i=0; i<dsch.size()+1; i++) {
//    cout << vdims[i]->name() << endl;
  //}

  DEBUG << "Record dim: ";
  NcDim *d = ncdf->rec_dim();
  DEBUG << d->name() << endl;
  
  var = ncdf->get_var("time");
  var->set_cur(index);
  var->put(&time, 1);
  ncdf->sync();

  const Scheme &tsch = gf->GetScheme(this->dim);
  for (unsigned int k=0; k<tsch.size(); k++) {
    Array *attr = gf->GetAttribute(this->dim, tsch.getAttribute(k));
    if (attr->size() != sz) {
      Fatal("Cardinality %i does not match product of dimension sizes %i", attr->size(), sz);
    }

    if (!HasVar(ncdf, attr->sname().c_str())) {
      DEBUG << "Adding var " << attr->sname() << endl;
      NcVar *var = ncdf->add_var(attr->sname().c_str(), mapType(attr->type), dsch.size()+1, vdims);
      addAttributes(var, this->datestr);

//    DEBUG << "Putting data " << endl;
//    var->set_cur(index, 0);
      long *edges = var->edges();
/*  
    int d = var->num_dims();
    for (int i=0; i<d; i++) {
      DEBUG << edges[i] << ", " << vdims[i]->size()  << endl;
    }
*/
      DEBUG << "Putting data " << sz << endl;
    switch (attr->type) {
      case INT:
        var->put((int *) attr->getVals(), edges);
        break;
      case FLOAT:
        var->put((float *) attr->getVals(), edges);
        break;
      default:
        Fatal("Cannot convert object types to netcdf types for attribute %s", attr->sname().c_str());
    }

    }
  }

  ncdf->sync();
 
  this->Result = this->GF;
};

void OutputNetCDFDim::WriteCells(AbstractCellArray *ca, Dim_t d) {
  GridField *gf = GF->getResult();

  DEBUG << "writing Cells" << endl;
  // Compute maximum number of cell vertices
  unsigned int maxcellsize = 0;
  for (unsigned int i=0; i<ca->getsize(); i++) { 
     Cell *c = ca->getCell(i);
     if (maxcellsize < c->getsize()) maxcellsize = c->getsize();
  }

  DEBUG << "adding cell dim of size " << gf->Card(d) << endl;
  // add a dimension for the number of cells
  NcDim *celldim = ncdf->add_dim("cell", gf->Card(d));

  DEBUG << "adding cellsize dim of size " << maxcellsize << endl;
  // add a dimension for the number of vertices in a cell
  NcDim *cellvertexdim = ncdf->add_dim("nv", maxcellsize);

  // write cells
  DEBUG << "writing cells" << endl;
  NcVar *cellvertices = ncdf->add_var("cell_vertices", ncInt, celldim, cellvertexdim);

//  int cd = celldim->size();
//  int cv = cellvertexdim->size();
  int *nodes;

  nodes = new int[maxcellsize];

  for (int i=0; i<celldim->size(); i++) {
    Cell *c = ca->getCell(i);

    assert(c->getsize() <= maxcellsize);

    for (unsigned int j=0; j<c->getsize(); j++) {
      nodes[j] = c->getnodes()[j];
    }

    cellvertices->set_cur(i, 0);

    if (!cellvertices->put(nodes, 1, c->getsize())) {
      Fatal("Error writing netCDF file");
    }

  }
  delete [] nodes;
}

bool HasVar(NcFile *ncdf, const string varname) {
  for (int i=0; i<ncdf->num_vars(); i++) {
    NcVar *var = ncdf->get_var(i);
    if (string(var->name()) == varname) {
      return 1;
    }
  }
  return 0;
}

bool HasDim(NcFile *ncdf, const string dimname) {
  NcDim *nodedim;
  int i=0;
  DEBUG << "_" << endl;
  for (; i<ncdf->num_dims(); i++) {
    nodedim = ncdf->get_dim(i);
    DEBUG << "|" << nodedim->name() << endl;
    if (string(nodedim->name()) == dimname) break;
  }
  if (i==ncdf->num_dims()) {
    return 0;
  } else {
    return 1;
  }
}

bool HasAttr(NcFile *ncdf, const string attr) {
  DEBUG << "Has attr " << attr << "? ";
  for (int i=0; i<ncdf->num_atts(); i++) {
    NcAtt *a = ncdf->get_att(i);
    if (string(a->name()) == (attr)) {
      DEBUG << "yes" << endl;
      return 1;
    }
  }
  DEBUG << "no" << endl;
  return 0;
}

void OutputNetCDFDim::Execute() {
  this->PrepareForExecution();
  GridField *gf = GF->getResult();

  const Scheme &nodescheme = gf->GetScheme(0);
  const Scheme &cellscheme = gf->GetScheme(this->dim);
  NcDim *celldim = 0;

  DEBUG << "Exporting gridfield as netcdf dims" << endl;
  // add dims
  if (this->dim > 0) {
    AbstractCellArray *ca = gf->GetGrid()->getKCells(this->dim);
    this->WriteCells(ca, this->dim);

    // write cell vars
    DEBUG << "Adding cell vars" << endl;
    celldim = ncdf->get_dim("cell");
    for (unsigned int k=0; k<cellscheme.size(); k++) {
      DEBUG << "Adding cell var " << cellscheme.getAttribute(k) << endl;
      Array *attr = gf->GetAttribute(this->dim, cellscheme.getAttribute(k));
      NcVar *var = ncdf->add_var(attr->sname().c_str(), mapType(attr->type), celldim);
      addAttributes(var, this->datestr);
    }
  }

  bool writenodes = 0;
  NcDim *nodedim;
  if (HasDim(ncdf, basedimname.c_str())) {
    DEBUG << "Getting node dim " << basedimname.c_str() << endl;
    nodedim = ncdf->get_dim(basedimname.c_str());
  } else {
    DEBUG << "Adding node dim " << basedimname.c_str() << endl;
    nodedim = ncdf->add_dim(basedimname.c_str(), gf->Card(0));
    writenodes = 1;
  }

  DEBUG << "Adding attributes" << endl;
  // global attributes
  if (!HasAttr(ncdf, "title")) ncdf->add_att("title", "Hindcast of Columbia River Physical Variables");
  if (!HasAttr(ncdf, "institution")) ncdf->add_att("institution", "STC Coastal Margin and Prediction");
  if (!HasAttr(ncdf, "source")) ncdf->add_att("source", "SELFE");
  if (this->datestr != "") {
    if (!HasAttr(ncdf, "date")) ncdf->add_att("date", this->datestr.c_str());
  }

  if (writenodes) {
    // write node vars
    for (unsigned int k=0; k<nodescheme.size(); k++) {
      Array *attr = gf->GetAttribute(0, nodescheme.getAttribute(k));
      assert(attr->size() == nodedim->size());
      DEBUG << "Adding var for nodes " << attr->sname() << endl;
      if (!HasVar(ncdf, attr->sname().c_str())) {
        NcVar *var = ncdf->add_var(attr->sname().c_str(), mapType(attr->type), nodedim);
        addAttributes(var, this->datestr);
      }
    }
  }

  DEBUG << "Writing node data" << endl;
  if (writenodes) {
    // write data last so we don't need to return to define mode
    for (unsigned int k=0; k<nodescheme.size(); k++) {
      Array *attr = gf->GetAttribute(0, nodescheme.getAttribute(k));
      NcVar *var = ncdf->get_var(attr->sname().c_str());
    switch (attr->type) {
      case INT:
        var->put((int *) attr->getVals(), nodedim->size());
        break;
      case FLOAT:
        var->put((float *) attr->getVals(), nodedim->size());
        break;
      default:
        Fatal("Cannot convert object types to netcdf types for attribute %s", attr->sname().c_str());
    }

    }
  }

  if (this->dim > 0) {
    // write data last so we don't need to return to define mode
    for (unsigned int k=0; k<cellscheme.size(); k++) {
      Array *attr = gf->GetAttribute(0, cellscheme.getAttribute(k));
      NcVar *var = ncdf->get_var(attr->sname().c_str());

    switch (attr->type) {
      case INT:
        var->put((int *) attr->getVals(), celldim->size());
        break;
      case FLOAT:
        var->put((float *) attr->getVals(), celldim->size());
        break;
      default:
        Fatal("Cannot convert object types to netcdf types for attribute %s", attr->sname().c_str());
    }

    }
  }

  ncdf->sync();

  DEBUG << "done" << endl;
  this->Result = this->GF;
};

void OutputNetCDFDim::SetDate(string ds) {
  this->datestr = ds;
}

void OutputNetCDFVars::SetDate(string ds) {
  DEBUG << "Setting Date to " << ds << endl;
  this->datestr = ds;
}
