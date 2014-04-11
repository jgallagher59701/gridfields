#ifndef _PARTITION_H
#define _PARTITION_H

#include <iostream>
#include "gridfield.h"
#include "gridfieldoperator.h"
#include <string>

namespace GF {

class Stencil {
	/*
	 pattern of cells forming an atomic unit for partitioning

	 we want to tag each cell with a partition number

	 Cell is an integer

	 Incidence relation on disk:
	 2 -> 0, 0, 0

	 Or potentially:
	 2,0
	 2,0
	 2,0

	 <bad>
	 We need to partition the incidence relation into pages such that
	 connectedness, well-supportedness, homogeneity, are preserved in
	 the derived page-grids.

	 For example, if nodes a,b,c are incident to a 2-cell e in a grid G, then if a subgrid Gi that contains e, it must also contain a,b,c to maintain well-supportedness

	 Some of these properties can be encoded as an assignment function.  For a cell c, return all the cells in G that need to be included with it in Gi to maintain the appropriate property.

	 Properties of the form:
	 if P(a,b) in G, then P(a,b) in Gi
	 can be written as a function Cell -> [Cell]
	 (\a . [x | x in P(a,x)])

	 keep adding cells to a page
	 </bad>

	 Each query plan has a topological access pattern for each cell of each dimension.  Some need only the data of the cells of one dimension.  Some need the surrounding cells (gradient, particle tracking).  These access patterns are precisely the assignment functions of the aggregate operators (and special assignment functions for enforcing certain properties)

	 restrict: (\x. [y | y in cells(G), x > y])

	 Given an assignment function m, try to partition the grid such that
	 x in Gi => m(x) in Gi

	 regrid operators have different source and target grids and use data inthe assignment functions.  These are processed as nested-loops joins with a lumped, materialized output.   Another partition operation may be necessary to repair fractured stencils upstream.

	 Stencil is an assignment function G -> G that involves only topology

	 S = Stencil

	 if |S| > MAX_P,
	 raise an error

	 for each cell c,
	 plug it into the stencil
	 return neighboring cells N
	 if |P| + |N| > MAX_P,
	 flush P


	 */
}

class PartitionOp: public UnaryGridFieldOperator {
public:
	PartitionOp(GridFieldOperator *op, string tupleexpr);
	PartitionOp(GridFieldOperator *op, string attr, string expr);
	void Execute();
	static GridField *Partition(GridField *Gg, string tupleexpr);

private:
	string unparsedExpr;

};

} // namespace GF

#endif /* APPLY_H */
