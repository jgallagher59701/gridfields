#ifndef _STUEBE_H
#define _STUEBE_H

//#include "elio.h"
#include "netcdfcpp.h"
#include <iostream>

class StuebeNetCDFOp {
public:
  StuebeNetCDFOp(string fn, const Scheme xy, const Scheme z, const Scheme cross);
  
  string filename;
  void setFileName(char *fn) { filename = string(fn); };
  void WriteSELFENetCDF(GridFieldOperator *xyop, GridFieldOperator *zop, GridFieldOperator *crossop);
  void SetDate(string ds);

private:

  const Scheme xyscheme;
  const Scheme zscheme;
  const Scheme crossscheme;

  string datestr;
  NcFile *ncdf;
  NcVar *putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount);

  NcType mapType(Type t);
  void addAttributes(NcVar *ncvar, string datestr);
  void CreateVariables(const Scheme fsch, NcDim *dim1, NcDim *dim2, NcDim *dim3, NcDim *dim4);
  void WriteStaticVariable(const Scheme fsch, GridField *op, NcDim *nodedim);
  NcDim *get_dim(string dimname);

  // override these to handle 3D data differently
  void Create3DDimension(GridField *op);
  void Create3DVariables();
public:
  void Write3DTimestep(GridFieldOperator *gf, int index, float timestep);
};
#endif
