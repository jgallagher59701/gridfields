#include "config_gridfields.h"

#include "gridfield.h"
#include "refrestrict.h"
#include "expr.h"
#include "ordmap.h"
#include "subgridordmap.h"

namespace GF {

void RefRestrictOp::Execute()
{
	this->PrepareForExecution();
	SubgridOrdMap *ordmap = new SubgridOrdMap(GF->GetGrid());

	Result = RestrictOp::Restrict(this->expr, this->k, this->GF);
	Result->GetGrid()->setReferent((OrdMap *) ordmap);
}

GridField *RefRestrictOp::Restrict(const string &expr, Dim_t k, GridField *GF)
{
	GridField *result = RestrictOp::Restrict(expr, k, GF);
	SubgridOrdMap *ordmap = new SubgridOrdMap(GF->GetGrid());
	result->GetGrid()->setReferent(ordmap);
	return result;
}

} // namespace GF
