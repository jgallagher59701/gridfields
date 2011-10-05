#ifndef _NETCDF_H
#define _NETCDF_H

#include "gridfieldoperator.h"
//#include "netcdfcpp.h"
#include <iostream>

class NetCDFOp : public UnaryGridFieldOperator {
public:
  NetCDFOp(string fn, long off, GridFieldOperator *Op);
  NetCDFOp(string fn, GridFieldOperator *Op);
  
  string filename;
  int offset;
  void Execute();
  void setFileName(char *fn) { filename = string(fn); };
  static void NetCDF(GridField *GF, string filename, long offset);
  static void WriteNetCDF(vector<GridField *> &dims, GridField *cross);

private:

  static NcVar *putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount);
};

#endif
