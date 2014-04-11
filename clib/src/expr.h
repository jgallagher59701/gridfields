#ifndef _EXPR_H
#define _EXPR_H

#include "type.h"
#include "gridfield.h"
#include "fparser.hh"

#include "gridfields_hash_map.h"

#if 0
#ifdef HAVE_UNORDERED_MAP
#include <unordered_map>
#define HASH_MAP std::unordered_map
#else
#include <ext/hash_map>
#define HASH_MAP hash_map
#endif
#endif

namespace GF {

#if 0
#ifndef HAVE_UNORDERED_MAP
using namespace __gnu_cxx;  // for hash_map jhrg 4/16/12
#endif
#endif

class TupleFunction {
public:
	TupleFunction();
	~TupleFunction();
	void Parse(string tupleexpr);
	Scheme *ReturnType()
	{
		return &outScheme;
	}
	;
	Scheme *InputType()
	{
		return &inScheme;
	}
	;
	void Eval(Tuple &input, Tuple &output);
private:
	void SaveReservedWords();
	void BindVars(Tuple &intup, double *bindings);
	void getVars(string expr, vector<string> &returnval);
	void deleteFunctions();
	set<string> reservedWords;
protected:
	map<string, FunctionParser *> functions;
	double *bindings;
	Scheme inScheme;
	Scheme outScheme;
};

class SpecializedTupleFunction: public TupleFunction {
public:
	void SpecializeFor(Scheme &out);
	void Eval(Tuple &in, Tuple &out);
private:
	int in_tup_size;HASH_MAP<int, pair<int, Type> > in_position_map;
	HASH_MAP<int, pair<int, FunctionParser *> > out_position_map;
};

} // namespace GF

#endif /* _EXPR_H */

