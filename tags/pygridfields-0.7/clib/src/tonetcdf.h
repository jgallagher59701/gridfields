#ifndef _TONETCDF_H
#define _TONETCDF_H

#include "gridfieldoperator.h"
//#include "elio.h"
#include "netcdfcpp.h"
#include <iostream>

class OutputNetCDFOp : public UnaryGridFieldOperator {
public:
  OutputNetCDFOp(string fn, GridFieldOperator *Op, const Scheme f, const Scheme t);
  
  string filename;
  void Execute();
  void setFileName(char *fn) { filename = string(fn); };
  //virtual void Output(GridField *GF, string filename, long offset);
  void WriteNetCDF(vector<GridField *> &dims, GridField *cross, const string &fname);
  void WriteTimeVars(GridFieldOperator *gf, int index, float timestep);
  void SetDate(string ds);

private:

  const Scheme fixed;
  const Scheme time;

  string datestr;
  NcFile *ncdf;
  NcVar *putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount);
};


class OutputNetCDFDim : public UnaryGridFieldOperator {
public:
OutputNetCDFDim
(
  NcFile *f,
  Dim_t d, 
  string dn, 
  GridFieldOperator *Op
);

  void WriteCells(AbstractCellArray *ca, Dim_t d);
  void Execute();
  void SetDate(string ds);

private:
  string datestr;
  string basedimname;
  NcFile *ncdf;
  Dim_t dim;
};

class OutputNetCDFVars : public UnaryGridFieldOperator {
public:
OutputNetCDFVars(
NcFile *f,
const Scheme dims,
Dim_t d, 
GridFieldOperator *Op,
unsigned int i=0,  //index for the unbounded dimension
float t=0 // value for the unbounded dimension
);   
  void Execute();
  void SetDate(string ds);

private:
  string datestr;
  float time;
  Scheme dimscheme;
  unsigned int index;
  NcFile *ncdf;
  Dim_t dim;

};
// Utility netcdf functions
//void addAttributes(NcVar *var);
NcType mapType(Type t);
bool HasDim(NcFile *ncdf, const string dimname);
bool HasVar(NcFile *ncdf, const string varname);
bool HasAttr(NcFile *ncdf, const string attr);
#endif
