#ifndef _DATADUMP_H
#define _DATADUMP_H

#include "gridfieldoperator.h"
#include <iostream>

namespace GF {

class DataDumpOp: public UnaryGridFieldOperator {
public:
	DataDumpOp(Dim_t k, string fn, long off, GridFieldOperator *Op);

	void Execute();
	static void DataDump(GridField *GF, Dim_t k, string filename, long offset);

private:
	Dim_t _k;
	string filename;
	int offset;

	static void writeGridField(GridField *GF, ofstream &f);
};

} // namespace GF

#endif
