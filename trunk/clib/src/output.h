#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "gridfieldoperator.h"
//#include "elio.h"
//#include "netcdfcpp.h"
#include <iostream>

namespace GF {

class OutputOp: public UnaryGridFieldOperator {
public:
	OutputOp(string fn, long off, GridFieldOperator *Op);
	OutputOp(string fn, GridFieldOperator *Op);

	int offset;
	string filename;
	void Execute();
	void setFileName(char *fn)
	{
		filename = string(fn);
	}
	;
	virtual void Output(GridField *GF, string filename, long offset);
	//static void WriteNetCDF(vector<GridField *> &dims, GridField *cross);

private:

	static void writeCellArray(AbstractCellArray *ca, ofstream &f);
	static void writeGrid(Grid *G, ofstream &f);
	static void writeGridField(GridField *GF, ofstream &f);
	static void writeDataset(const Dataset &ds, ofstream &f);
	static void writeName(string name, ofstream &f);

	//static NcVar *putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount);
};
/*
 class OutputElcircOp : public OutputOp {
 public:
 OutputElcircOp(string fn, ElcircHeader *copy, GridFieldOperator *Op);

 virtual void Output(GridField *GF, string filename, long offset);
 //static void WriteNetCDF(vector<GridField *> &dims, GridField *cross);

 private:
 ElcircHeader h;
 ElcircHeader *copy;
 ElcircTimeStep t;

 //static NcVar *putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount);
 };
 */

} // namespace GF

#endif
