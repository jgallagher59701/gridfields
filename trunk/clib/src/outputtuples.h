#ifndef _OUTPUTTUPLES_H
#define _OUTPUTTUPLES_H

#include "gridfieldoperator.h"
//#include "netcdfcpp.h"
#include <iostream>

namespace GF {

class OutputTuplesOp: public UnaryGridFieldOperator {
public:
	OutputTuplesOp(string fn, long off, GridFieldOperator *Op);
	OutputTuplesOp(string fn, GridFieldOperator *Op);

	int offset;
	string filename;
	void Execute();
	void setFileName(char *fn)
	{
		filename = string(fn);
	}
	;
	static void Output(GridField *GF, string filename, long offset);
	//static void WriteNetCDF(vector<GridField *> &dims, GridField *cross);

private:

	static void writeCellArray(AbstractCellArray *ca, ofstream &f);
	static void writeGrid(Grid *G, ofstream &f);
	static void writeGridField(GridField *GF, ofstream &f);
	static void writeDataset(const Dataset &ds, ofstream &f);
	static void writeName(string name, ofstream &f);

	//static NcVar *putData(Array *a, NcFile *ncdf, long *counts, NcDim **d, int dimcount);
};

} // namespace GF

#endif
