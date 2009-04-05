#ifndef _ELCIRC2NETCDF_H
#define _ELCIRC2NETCDF_H

#include "netcdfcpp.h"

class GridField;

void WriteNetCDF(vector<GridField *> &dims, GridField *cross, const string &fname);
void WriteTimeVars(GridFieldOperator *gf, int timestep);
NcType mapType(Type t);

#endif
