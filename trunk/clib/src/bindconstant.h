#ifndef _BINDCONSTANTOP_H
#define _BINDCONSTANTOP_H

#include "arrayreader.h"
#include "array.h"
#include "gridfieldoperator.h"

namespace GF {

class BindConstantOp: public UnaryGridFieldOperator {
public:
//  BindConstantOp(const string &attr, int val, GridFieldOperator *op);
	BindConstantOp(Dim_t k, const string &attr, float val, GridFieldOperator *op);

	void Execute();
//  static GridField *BindConstant(const string &attr, GridField *GF);
	void setAttributeVal(const string &a, float v);
	static GridField *BindConstant(Dim_t k, const string &attr, float val, GridField *GF);
private:
	Dim_t _k;
	string attr;
	float val;
};

} // namespace GF

#endif
