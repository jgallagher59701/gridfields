#include "config_gridfields.h"

#include "gridfield.h"
#include <stdio.h>
//extern "C" {
//#include "stdio.h"
#include "elio.h"
//}
#include "expr.h"
#include "timing.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include "array.h"
#include "datadump.h"
#include "arraywriter.h"

using namespace std;

namespace GF {

DataDumpOp::DataDumpOp(Dim_t k, string fn, long off, GridFieldOperator *op) :
		UnaryGridFieldOperator(op), _k(k), filename(fn), offset(off)
{
}

void DataDumpOp::Execute()
{
	this->PrepareForExecution();
	this->DataDump(this->GF, this->_k, this->filename, this->offset);
	this->Result = this->GF;
}

void DataDumpOp::DataDump(GridField *GF, Dim_t k, string filename, long)
{

	ofstream f(filename.c_str(), ios::binary | ios::out | ios::app);

	int arity = GF->Arity(k);
	f.write((char *) &arity, sizeof(int));

	ArrayWriter aw(&f);
	Array *a;
	const Scheme &sch = GF->GetScheme(k);
	for (int i = 0; i < arity; ++i) {
		a = GF->GetAttribute(k, sch.getAttribute(i));
		//writeName(string(a->name), f);
		//f.write((char *) &a->type, sizeof(Type));
		//f.write((char *) &a->size, sizeof(int));
		aw.Write(GF->GetDataset(k), string(a->getName()));
	}
}

} // namespace GF

