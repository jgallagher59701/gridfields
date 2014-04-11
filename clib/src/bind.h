#ifndef _BINDOP_H
#define _BINDOP_H

#include "arrayreader.h"
#include "array.h"
#include "gridfieldoperator.h"

namespace GF {

class BindOp: public UnaryGridFieldOperator {
public:
	BindOp(Array *arr, Dim_t k, GridFieldOperator *op);
	BindOp(Array *arr, ArrayReader *rdr, Dim_t k, GridFieldOperator *op);
	BindOp(string attr, Type t, ArrayReader *r, Dim_t k, GridFieldOperator *op);
	BindOp(string attr, Type t, string filename, int offset, Dim_t k, GridFieldOperator *op);
	BindOp(string attr, Type t, string filename, int offset, string addresses, Dim_t k, GridFieldOperator *op);

	void setArrayReader(ArrayReader *ar)
	{
		delete this->reader;
		this->reader = ar;
		this->Update();
	}
	;
	void setOffset(UnTypedPtr value);
	void setOffsetInt(UnTypedPtr value);
	void setOffsetInt(int value);

	void Execute();
	static GridField *Bind(const string &attr, Type t, ArrayReader *ar, Dim_t k, GridField *GF);
private:
	int temp;
	// array Unused. jhrg 4/4/14 Array *array;
	string attr;
	Type type;
	ArrayReader *reader;
	Dim_t _k;

};

} // namespace GF

#endif
