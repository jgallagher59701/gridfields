#include "restrict.h"

namespace GF {

class RefRestrictOp: public RestrictOp {
public:
	RefRestrictOp(const string &expr, Dim_t k, GridFieldOperator *GF) :
			RestrictOp(expr, k, GF)
	{
	}
	;
	void Execute();
	static GridField *Restrict(const string &expr, Dim_t k, GridField *GF);

};

} // namespace GF
