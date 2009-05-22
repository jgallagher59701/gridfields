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
#include "stuebe.h"
#include <string.h>
#include "implicit0cells.h"
#include "arraywriter.h"

#include <netcdfcpp.h>
//#include <proj_api.h>

#define MAXCELLVERTEX 4
//#define DEBUG cout

using namespace std;

StuebeNetCDFOp::StuebeNetCDFOp(string fn, const Scheme xy, const Scheme z, const Scheme cross)  
 : filename(fn), xyscheme(xy), zscheme(z), crossscheme(cross),ncdf(NULL) {}

void StuebeNetCDFOp::SetDate(string ds) {
  this->datestr = ds;
}

NcType StuebeNetCDFOp::mapType(Type t) {
  switch (t) {
    case FLOAT:
      return ncFloat;
    case INT:
      return ncInt;
    default:
      Fatal("Unknown Type encountered during netCDF emission");
      return ncInt;
  }
}

NcDim *StuebeNetCDFOp::get_dim(string dimname) {
  NcDim *d = this->ncdf->get_dim(dimname.c_str());
  if (!d) { Fatal("Unable to find dimension %s", dimname.c_str()); }
  return d;
}


void StuebeNetCDFOp::addAttributes(NcVar *var, string datestr) {
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
     success &= var->add_att("coordinates", "x y");
     success &= var->add_att("long_name", "Water elevation in meters above Mean Sea Level");
  }
  if (strcmp(var->name(), "eta") == 0) {
     success &= var->add_att("units", "m");
     success &= var->add_att("positive", "down");
     success &= var->add_att("long_name", "bathymetry measured as meters below Mean Sea Level");
  }
  if (strcmp(var->name(), "bottom_index") == 0) {
     success &= var->add_att("long_name", "Bottom index: level index indicating the lowest level that is still above the bathymetry; 0-based indexing");
  }
  if (strcmp(var->name(), "salt") == 0) {
     success &= var->add_att("units", "psu");
     success &= var->add_att("long_name", "Salinity in Practical Salinity Units");
     success &= var->add_att("coordinates", "x y z");
     success &= var->add_att("standard_name", "sea_water_salinity");
     success &= var->add_att("missing_value", -99.0f);
     success &= var->add_att("_FillValue", -99.0f);
  }
  if (strcmp(var->name(), "temp") == 0) {
     success &= var->add_att("units", "C");
     success &= var->add_att("long_name", "temperature in degrees celsius");
     success &= var->add_att("coordinates", "x y z");
     success &= var->add_att("missing_value", -99.0f);
     success &= var->add_att("_FillValue", -99.0f);
  }
  if (strcmp(var->name(), "u") == 0) {
     success &= var->add_att("units", "m/s");
     success &= var->add_att("long_name", "velocity in the x direction in meters per second");
     success &= var->add_att("coordinates", "x y z");
     success &= var->add_att("missing_value", 0.0f);
     success &= var->add_att("_FillValue", 0.0f);
  }
  if (strcmp(var->name(), "v") == 0) {
     success &= var->add_att("units", "m/s");
     success &= var->add_att("long_name", "velocity in the y direction in meters per second");
     success &= var->add_att("coordinates", "x y z");
     success &= var->add_att("missing_value", 0.0f);
     success &= var->add_att("_FillValue", 0.0f);
  }
  if (strcmp(var->name(), "w") == 0) {
     success &= var->add_att("units", "m/s");
     success &= var->add_att("long_name", "velocity in the z direction in meters per second");
     success &= var->add_att("coordinates", "x y z");
     success &= var->add_att("positive", "up");
     success &= var->add_att("missing_value", 0.0f);
     success &= var->add_att("_FillValue", 0.0f);
  }
  if (strcmp(var->name(), "cell_vertices") == 0) {
     success &= var->add_att("long_name", "connectivity table; 0-based indices");
     success &= var->add_att("cell_type", "triangle");
     success &= var->add_att("standard_name", "connectivity_array");
     success &= var->add_att("index_start", "1");
     success &= var->add_att("coordinates_node", "x y");
  }

  if (strcmp(var->name(), "node") == 0) {
     success &= var->add_att("long_name", "node number; 0-based");
  }

  if (!success) {
    Fatal("Error adding attribute to netCDF variable");
  }
}

void StuebeNetCDFOp::CreateVariables(const Scheme fsch, NcDim *dim1, NcDim *dim2=NULL, NcDim *dim3=NULL, NcDim *dim4=NULL) {
   // Generate 1-dimensional netcdf variables.  names and types are derived from the scheme argument.  
   // 
   string name;
   Type t;
   NcVar *var;
   for (int k=0; k<fsch.size(); k++) {
     name = fsch.getAttribute(k);
     t = fsch.getType(k);
     var = this->ncdf->add_var(name.c_str(), mapType(t), dim1, dim2, dim3, dim4);
     addAttributes(var, this->datestr);
   }
}

void StuebeNetCDFOp::WriteStaticVariable(const Scheme fsch, GridField *op, NcDim *nodedim) {
   Array *attr;
   NcVar *var;
   for (size_t k=0; k<fsch.size(); k++) {
     attr = op->GetAttribute(0, fsch.getAttribute(k));
     var = this->ncdf->get_var(attr->sname().c_str());
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
}

/* Reimplement the next three methods to represent 3D data differently */

void StuebeNetCDFOp::Create3DDimension(GridField *op) {
 // In this representation, 3D data is represented as a 1D variable with its own dimension.
   NcDim *nodedim = ncdf->add_dim("n3DNode", op->Card(0));
}

void StuebeNetCDFOp::Create3DVariables() {
  // This representation uses one netcdf dimension.
   const Scheme &sch = this->crossscheme;
   NcDim *nodedim = this->get_dim("n3DNode");
   NcDim *timedim = this->get_dim("time");
   this->CreateVariables(sch, timedim, nodedim);
}

void StuebeNetCDFOp::Write3DTimestep(GridFieldOperator *op, int index, float timestep) {
  /* Write out one timestep's worth of 3D variables gleaned from op */
  GridField *gf = op->getResult();
  Array *attr;
  NcVar *var;
  NcDim *nodedim = this->get_dim("n3DNode");
  const Scheme &sch = this->crossscheme;

  for (size_t k=0; k<sch.size(); k++) {
    attr = gf->GetAttribute(0, sch.getAttribute(k));
    var = ncdf->get_var(attr->sname().c_str());
    var->set_cur(index, 0);
    switch (sch.getType(k)) {
      case INT:
        var->put((int *) attr->getVals(), 1, nodedim->size());
        break;
      case FLOAT:
        var->put((float *) attr->getVals(), 1, nodedim->size());
        break;
      default:
        Fatal("Cannot convert object types to netcdf types for attribute %s", attr->sname().c_str());
    }
  }
  var = ncdf->get_var("time");
  var->set_cur(index);
  var->put(&timestep, 1);
  ncdf->sync();
}

void StuebeNetCDFOp::WriteSELFENetCDF(GridFieldOperator *xyop, GridFieldOperator *zop, GridFieldOperator *crossop) {

   GridField *xy = xyop->getResult();
   xy->GetGrid()->normalize();
   GridField *z = zop->getResult();
   z->GetGrid()->normalize();
   GridField *cross = crossop->getResult();
   cross->GetGrid()->normalize();

   GridField *gf = cross;
   // only works for 2-D gridfields
   assert(xy->Dim() == 2);
   assert(xy->IsAttribute(0, "x"));
   assert(xy->GetAttribute(0, "x")->type == FLOAT);
   assert(xy->IsAttribute(0, "y"));
   assert(xy->GetAttribute(0, "y")->type == FLOAT);
   
   assert(z->Dim() == 1);
   assert(z->IsAttribute(0, "depth_percent"));
   assert(z->GetAttribute(0, "depth_percent")->type == FLOAT);

   assert(gf->Dim() == 3);

   this->ncdf = new NcFile(this->filename.c_str(), NcFile::Replace); 
   
   // write metadata
   // add dims
   NcDim *celldim = ncdf->add_dim("nCell", xy->Card(2));
   NcDim *nodedim = ncdf->add_dim("nNode", xy->Card(0));
   NcDim *depthdim = ncdf->add_dim("nDepth", z->Card(0));
  
   this->Create3DDimension(gf);
 
   // an unlimited dimension
   NcDim *timedim = ncdf->add_dim("time");

   // maximum number of cell vertices
   NcDim *cellvertexdim = ncdf->add_dim("nConnect", MAXCELLVERTEX);

   // global attributes
   this->ncdf->add_att("title", "Hindcast of Columbia River Physical Variables");
   this->ncdf->add_att("institution", "STC Coastal Margin and Prediction");
   this->ncdf->add_att("source", "ELCIRC");
   if (this->datestr != "") {
     this->ncdf->add_att("date", this->datestr.c_str());
   }
   
   // write cells
   NcVar *cellvertices = ncdf->add_var("grid1", ncInt, cellvertexdim, celldim);
   addAttributes(cellvertices, this->datestr);
   AbstractCellArray *ca = xy->GetGrid()->getKCells(2);
   int *nodes; 
   nodes = new int[MAXCELLVERTEX];
   DEBUG << "Writing grid topology..."<< endl;
   for (int i=0; i<celldim->size(); i++) {
     Cell *c = ca->getCell(i);

     assert(c->getsize() <= MAXCELLVERTEX);
     
     for (int j=0; j<c->getsize(); j++) {
       nodes[j] = c->getnodes()[j]; 
     }
   
     if (!cellvertices->set_cur(0,i)) {
       Fatal("Index %s exceeds dimensions bounds %s", i, celldim->size());
     };
     
     if (!cellvertices->put(nodes, c->getsize(), 1)) {
       Fatal("Error writing netCDF file");
     }
   }
   delete [] nodes;

   // write cell attributes according to Stuebe's conventions
   cellvertices->add_att("cell_type", "tri_ccw");
   cellvertices->add_att("index_start", 0);
   cellvertices->add_att("standard_name", "connectivity_array");
   cellvertices->add_att("coordinates_node", "x y");

   NcVar *time = ncdf->add_var("time", ncFloat, timedim);
   DEBUG << "Date: " << this->datestr << endl;
   addAttributes(time, this->datestr);


   // write fixed XY data
   this->CreateVariables(this->xyscheme, nodedim);

   // write fixed Z data
   this->CreateVariables(this->zscheme, depthdim);

   // prepare time-varying data
   this->Create3DVariables();

   // write data last so we don't need to return to define mode
   
   // xy data
   this->WriteStaticVariable(this->xyscheme, xy, nodedim);
   // z data
   this->WriteStaticVariable(this->zscheme, z, depthdim);
   
   ncdf->sync();
}
