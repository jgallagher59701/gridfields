#ifndef _IMPLICITCROSSNODEMAP_H
#define _IMPLICITCROSSNODEMAP_H

#include "crossnodemap.h"
#include "abstractcellarray.h"
#include "implicit0cells.h"

namespace GF {

class ImplicitCrossNodeMap: public CrossNodeMap {

public:
	ImplicitCrossNodeMap(Implicit0Cells *An, Implicit0Cells *Bn) :
			CrossNodeMap(An, Bn), /* Unused. jhrg 4/4/14 Anodes(An),*/Bnodes(Bn)
	{
	}
	;

	// Added because of complaints from gcc about missing virtual dtor
	// in a class with virtual methods. jhrg 4/4/14
	virtual ~ImplicitCrossNodeMap()
	{
	}

	inline virtual Node map(Node a, Node b)
	{
		//return Anodes->getOrd(a) * Bnodes->getsize() + Bnodes->getOrd(b) ;
		return a * Bnodes->getsize() + b;
	}
	;
	/*
	 bool sameCells(AbstractCellArray *cells) {
	 return ((*cells == *Anodes) || (*cells == *Bnodes))
	 }
	 */
	Node inv_b(Node o, Node)
	{
		Cell *c;
		int i = o % Bnodes->getsize();
		c = Bnodes->getCell(i);
		return c->getnodes()[0];
	}
	;

	Node inv_a(Node o, Node)
	{
		Cell *c;
		int i = o / Bnodes->getsize();
		c = Bnodes->getCell(i);
		return c->getnodes()[0];
	}
	;

private:
	// Anodes Unused. jhrg 4/4/14 Implicit0Cells *Anodes;
	Implicit0Cells *Bnodes;
};

} // namespace GF

#endif /* _CROSSNODEMAP_H */
