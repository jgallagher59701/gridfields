#ifndef _NETCDFADAPTOR_H
#define _NETCDFADAPTOR_H

#include "gridfield.h"
#include "grid.h"
#include "cell.h"
#include "netcdfcpp.h"
#include <iostream>
#include <string>
#include <vector>

namespace GF {

class NetCDFAdaptor {
public:
	NetCDFAdaptor(string fn) :
			filename(fn), ncdf(NULL)
	{
	}
	;
	NetCDFAdaptor(const char *fn) :
			filename(fn), ncdf(NULL)
	{
	}
	;
	void Open(string mode = "r");
	void Close();
	void setFileName(char *fn)
	{
		filename = string(fn);
	}
	;
	void setFileName(string fn)
	{
		filename = string(fn);
	}
	;

	// Read functions
	void NodesFromDim(string ncdim, Grid *G);
	void WellSupportedPolygonsFromVars(string cnodes, string cedges, Grid *G);
	void HomogeneousCellsFromVar(Dim_t d, string ncvar, Grid *G);
	void AttributeFromVar(Dim_t d, string ncvar, GridField *G);

	// Write functions
	void CreateDim(const string &name, unsigned int size);
	void DimFromDim(const string &name, GridField *gf, Dim_t d);
	void VarFromAttribute(const string &name, GridField *gf, Dim_t d, const vector<string> &dims);
	void VarFromIncidence(const string &name, GridField *gf, Dim_t c, Dim_t d, string d1, string d2);

	// Utility netcdf functions
	static NcType mapType(Type t);
	static Type mapType(NcType t);
	static bool HasDim(NcFile *ncdf, const string dimname);
	static bool HasVar(NcFile *ncdf, const string varname);
	static bool HasAttr(NcFile *ncdf, const string attr);

private:
	string filename;
	NcFile *ncdf;
};

} // namespace GF

#endif
