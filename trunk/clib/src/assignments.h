#ifndef ASSIGNMENTS_H
#define ASSIGNMENTS_H

#include "aggregate.h"
#include "cellarray.h"
#include "array.h"
#include "gridfield.h"
#include "crossnodemap.h"
#include "expr.h"
#include "RTree.h"

namespace Assign {

using namespace GF;

bool equal(Type t, UnTypedPtr p, UnTypedPtr q);
bool TestCallback(long unsigned id, void *arg);
int pnpoly(int npol, float *xp, float *yp, float x, float y);

class Both: public AssignmentFunction {
public:
	AssignmentFunction &left;
	AssignmentFunction &right;

	Both(AssignmentFunction &left, AssignmentFunction &right) :
			left(left), right(right)
	{
	}
	;

	virtual void operator()(const CellId &c, vector<CellId> &out)
	{
		vector<CellId> leftcells;
		vector<CellId> rightcells;

		set<CellId> leftset;
		set<CellId> rightset;

		left(c, leftcells);
		right(c, rightcells);

		leftset.insert(leftcells.begin(), leftcells.end());
		rightset.insert(rightcells.begin(), rightcells.end());

		insert_iterator<vector<CellId> > ii(out, out.begin());

		set_intersection(leftset.begin(), leftset.end(), rightset.begin(), rightset.end(), ii);

	}
	;

	void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j)
	{
		this->T = t;
		this->S = s;
		this->_i = i;
		this->_j = j;
		left.setEnvironment(t, i, s, j);
		right.setEnvironment(t, i, s, j);
	}
	;
};

class Either: public AssignmentFunction {
public:
	AssignmentFunction &left;
	AssignmentFunction &right;

	Either(AssignmentFunction &left, AssignmentFunction &right) :
			left(left), right(right)
	{
	}
	;

	virtual void operator()(const CellId &c, vector<CellId> &out)
	{
		vector<CellId> leftcells;
		vector<CellId> rightcells;

		set<CellId> uniquecells;

		left(c, leftcells);
		right(c, rightcells);

		uniquecells.insert(leftcells.begin(), leftcells.end());
		uniquecells.insert(rightcells.begin(), rightcells.end());

		out.insert(out.begin(), uniquecells.begin(), uniquecells.end());
	}
	;

	void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j)
	{
		this->T = t;
		this->S = s;
		this->_i = i;
		this->_j = j;
		left.setEnvironment(t, i, s, j);
		right.setEnvironment(t, i, s, j);
	}
	;
};

class IncidentTo: public AssignmentFunction {
public:
	IncidentTo()
	{
	}
	;
	virtual void operator()(const CellId &cid, vector<CellId> &out)
	{
		S->GetGrid()->IncidentTo(cid, this->_i, out, this->_j);
	}
};
/*
 class ToIncident : public AssignmentFunction {
 public:
 int dim;
 ToIncident(int d) : dim(d) {};
 virtual void ToIncident::operator()(const CellId &cid,
 vector<CellId> &out) {
 S->GetGrid()->ToIncident(cid, d, out);
 }
 }
 */

class Nodes: public AssignmentFunction {
public:
	Nodes()
	{
	}
	;
	virtual void operator()(const CellId &cid, vector<CellId> &out)
	{
		Cell *c = T->getKCell(this->_i, cid);
		for (unsigned int i = 0; i < c->getsize(); i++) {
			out.push_back(c->getnodes()[i]);
		}
	}
};

class adjacentNodes: public AssignmentFunction {
	// I_{0k0}
	// find vertices incident to some k-cell to which I am also incident
public:
	adjacentNodes(Dim_t k) :
			k(k)
	{
	}
	;

	virtual void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j)
	{
		this->T = t;
		this->S = s;
		assert(((i == 0) & (j == 0)));
		this->_i = i;
		this->_j = j;
		this->kcells = S->GetGrid()->getKCells(this->k);
		this->zerocells = T->GetGrid()->getKCells(0);
	}

	virtual void operator()(const CellId &c, vector<CellId> &out)
	{
		Cell *nc = this->zerocells->getCell(c);
		assert(nc->getsize() == 1);
		Node m = nc->getnodes()[0];

		Cell *kc;
		set<CellId> ks;
		this->kcells->getIncidentCells(m, ks);
		FOR(set<CellId>, kci, ks)
		{
			kc = this->kcells->getCell(*kci);
			for (unsigned int i = 0; i < kc->getsize(); i++) {
				Node n = kc->getnodes()[i];
				if (n != m) out.push_back(n);
			}
		}
	}

private:
	AbstractCellArray *kcells;
	AbstractCellArray *zerocells;
	Dim_t k;
};

class adjacent: public AssignmentFunction {
	// I_{i0j}
	// find j-cells to which one of my vertices is incident
public:
	adjacent()
	{
	}
	;
	virtual void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j)
	{
		this->T = t;
		this->S = s;
		this->_i = i;
		this->_j = j;
		this->cells = S->GetGrid()->getKCells(this->_j);

		/*
		 // build adjacency index
		 Grid *G = S->GetGrid();
		 unsigned int n = S->Size(this->_j);

		 adj.clear();
		 adj.resize(n);

		 set<CellId> adjs;
		 for (int i=0; i<n; i++) {
		 S->GetGrid()->adjacent(this->_j, i, adjs);
		 adj[i].insert(adj[i].end(), adjs.begin(), adjs.end());
		 adjs.clear();
		 }
		 */
	}

	virtual void operator()(const CellId &c, vector<CellId> &out)
	{
		this->cells->getAdjacentCells(c, out);
		//COPY(vector<CellId>, adj[c], out, ii);
	}

private:
	AbstractCellArray *cells;
};
/*
 class I : public AssignmentFunction {
 public:
 //I(Dim_t i, Dim_t j)
 virtual void setEnvironment(GridField *t, Dim_t i,
 GridField *s, Dim_t j) {
 this->T=t;
 this->S=s;
 this->_i = i;
 this->_j = j;
 this->cells = S->GetGrid()->getKCells(this->_j);
 }

 virtual void I::operator()(const CellId &cid, vector<CellId> &out) {

 }

 };
 */
class neighbors: public AssignmentFunction {
public:
	virtual void operator()(const CellId &cid, vector<CellId> &out)
	{

		Cell *nc = T->getKCell(this->_i, cid);

		set<CellId> setOut;
		AbstractCellArray *dcells = S->GetGrid()->getKCells(this->_j);
		for (unsigned int i = 0; i < nc->getsize(); i++) {
			dcells->getIncidentCells(nc->getnodes()[i], setOut);
		}

		set<CellId>::iterator p;
		for (p = setOut.begin(); p != setOut.end(); p++) {
			out.push_back(*p);
		}
	}
};

class intervalContains: public AssignmentFunction {
public:
	string rangeattr;
	string valueattr;
	string pointattr;
	intervalContains(string ra, string va, string pa) :
			rangeattr(ra), valueattr(va), pointattr(pa)
	{
	}
	;

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);

		UnTypedPtr r = t.get(rangeattr);

		vector<Tuple *> *rng = (vector<Tuple *> *) r;

		assert(rng->size() == 2);

		float v1 = *(float *) (*rng)[0]->get(valueattr);
		float v2 = *(float *) (*rng)[1]->get(valueattr);

		float v;

		for (unsigned int i = 0; i < S->Size(this->_j); i++) {
			v = *(float *) dssource->GetAttributeVal(pointattr, i);
			if ((v >= v1) & (v < v2)) {
				out.push_back(i);
			}
		}

	}
};

template<typename ValueType>
class memberof: public AssignmentFunction {
public:
	string item_attr;
	string set_attr;
	string set_value_attr;

	memberof(string item_attr, string set_attr, string set_value_attr) :
			item_attr(item_attr), set_attr(set_attr), set_value_attr(set_value_attr)
	{
	}
	;

	virtual void operator()(const CellId &c, vector<CellId> &out)
	{
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);

		ValueType v = *(ValueType *) t.get(item_attr);

		vector<Tuple *> *vec;

		ValueType tst;

		vector<Tuple *>::iterator p;
		for (unsigned int i = 0; i < dssource->Size(); i++) {
			vec = (vector<Tuple *> *) dssource->GetAttributeVal(set_attr, i);
			for (p = vec->begin(); p != vec->end(); p++) {
				tst = *(ValueType *) (*p)->get(set_value_attr);
				if (tst == v) {
					out.push_back(i);
					break;
				}

			}
		}
	}
	;
};

class byPointerSet: public AssignmentFunction {
public:
	byPointerSet(string sa, string pa) :
			setattr(sa), pattr(pa)
	{
	}
	;

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);
		vector<Tuple> &pset = *(vector<Tuple> *) t.get(setattr);

		for (unsigned int i = 0; i < pset.size(); i++) {
			out.push_back(*(CellId *) pset[i].get(pattr));
		}
	}
	;
private:
	string setattr;
	string pattr;
};

class nearest: public AssignmentFunction {
public:
	string a;  //S attribute
	string b;  //T attribute
	nearest(string attra, string attrb)
	{
		a = attra;
		b = attrb;
	}
	;
	nearest(string attra)
	{
		a = attra;
		b = attra;
	}
	;
	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);
		UnTypedPtr p = t.get(a);
		S->nearest(b, _j, p, out);
	}
};

class sortedmatch: public AssignmentFunction {
	CellId position;
	string Tattr;
	string Sattr;
	Type type;
public:
	virtual void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j)
	{
		position = 0;
		T = t;
		S = s;
		_i = i;
		_j = j;
		dstarget = &T->GetDataset(_i);
		dssource = &S->GetDataset(_j);
		schtarget = dstarget->GetScheme();
		schsource = dssource->GetScheme();
		Type tt = schtarget.getType(Tattr);
		Type st = schsource.getType(Sattr);
		if (tt != st) {
			Fatal("Target attribute '%s' is type '%i', but source attribute '%s' is type '%i'.", Tattr.c_str(), tt,
					Sattr.c_str(), st);
		}
		type = tt;
	}

	sortedmatch(string a, string b) :
			Tattr(a), Sattr(b)
	{
	}
	;
	sortedmatch(string a) :
			Tattr(a), Sattr(a)
	{
	}
	;

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);
		if (t.isNull()) {
			cout << Tattr << endl;
			T->PrintTo(cout, 5);
			Fatal("Cell not found.");
		}
		UnTypedPtr p = t.get(Tattr);
		Tuple s(&schsource);
		UnTypedPtr val;
		unsigned int size = S->Size(_j);
		while (position < size) {
			dssource->FastBindTuple(position, s);
			val = s.get(Sattr);
			if (equal(type, val, p)) {
				out.push_back(position++);
			}
			else {
				break;
			}
		}
		/*
		 int count = -1;
		 int last = S->Size(_j) - 1;
		 do {
		 count++;
		 dssource->FastBindTuple(position+count, s);
		 cout << position << ", " << count << ", " << last << endl;
		 val = s.get(Sattr);
		 s.print();
		 } while (equal(type, val, p) && (position+count < last));

		 for (int i=0; i<count; i++){
		 out.push_back(position++);
		 }
		 */
	}
};

class match: public AssignmentFunction {
public:
	string Tattr;
	string Sattr;

	match(string a, string b) :
			Tattr(a), Sattr(b)
	{
	}
	;
	match(string a) :
			Tattr(a), Sattr(a)
	{
	}
	;
	/*
	 virtual void setEnvironment(GridField *t, Dim_t i,
	 GridField *s, Dim_t j) {
	 this->T=t;
	 this->S=s;
	 this->_i = i;
	 this->_j = j;
	 this->cells = S->GetGrid()->getKCells(this->_j);
	 }
	 */
	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		/*
		 if (!T->isAttribute(attr.c_str()) || !S->isAttribute(attr.c_str())) {
		 return;
		 }
		 */
		//      cout << "evaluate gridfield..." << endl;
		//cout << "m:" << c.nodes[0] << endl;
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);
		if (t.isNull()) {
			cout << Tattr << endl;
			T->PrintTo(cout, 5);
			Fatal("Cell not found.");
		}
		//t.print();

		if (schtarget.getType(Tattr) != schsource.getType(Sattr)) {
			Fatal(("Type mismatch: type(" + Tattr + ") != type(" + Sattr + ")").c_str());
		}

		UnTypedPtr p = t.get(Tattr);
		switch (schtarget.getType(Tattr)) {
		case FLOAT:
			S->lookupFloat(Sattr, _j, *(float *) p, out);
			break;
		case INT:
			S->lookupInt(Sattr, _j, *(int *) p, out);
			break;
		case OBJ:
			S->lookupInt(Sattr, _j, *(int *) p, out);
			break;
		case TUPLE:
			S->lookupInt(Sattr, _j, *(int *) p, out);
			break;
		case GRIDFIELD:
			S->lookupInt(Sattr, _j, *(int *) p, out);
			break;
		}
		//cout << *(int *) p << ", " << Tattr << endl;
		//cout << *(int *)T->getAttributeVal(Tattr.c_str(), 0) << endl;
		//cout << out.size() << " -> ";
		//cout << "any? " << out.size() << endl;
	}

};

class pointpoly2: public AssignmentFunction {
public:
	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		typedef vector<Tuple *> Poly;

		Tuple tup(&schsource);
		Poly poly;
		int pts;

		//the target point
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);
		float x = *(float *) t.get("x");
		float y = *(float *) t.get("y");

		float xs[10];
		float ys[10];

		//for each cell in the source grid
		for (unsigned int i = 0; i < S->Size(this->_j); i++) {
			dssource->FastBindTuple(i, tup);
			poly = *(Poly *) tup.get("poly");
			pts = poly.size();
			if (pts == 0) continue;

			for (int j = 0; j < pts; j++) {
				xs[j] = *(float *) poly[j]->get("x");
				ys[j] = *(float *) poly[j]->get("y");
			}

			if (pnpoly(pts, xs, ys, x, y)) {
				out.push_back(i);
			}
		}
	}
};

class containedby: public AssignmentFunction {
	//from source polgons to target points
	//max polygon size is 10.

public:

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{

		float x;
		float y;
		x = *(float *) dstarget->GetAttributeVal("x", c);
		y = *(float *) dstarget->GetAttributeVal("y", c);

		for (unsigned int i = 0; i < S->Size(this->_j); i++) {
			if (PointInCell(x, y, i)) {
				out.push_back(i);
			}
		}
	}

	bool PointInCell(float x, float y, CellId i)
	{
		typedef vector<Tuple> Poly;
		Poly *poly;
		Tuple *t;
		Tuple polytup(&schsource);

		float xs[10];
		float ys[10];

		bool containedBy = false;
		dssource->FastBindTuple(i, polytup);
		poly = *(Poly **) polytup.get("poly");
		int s = poly->size();

		assert(s <= 10);

		for (int j = 0; j < s; j++) {
			t = &(*poly)[j];
			xs[j] = *(float *) t->get("x");
			ys[j] = *(float *) t->get("y");
		}
		if (pnpoly(s, xs, ys, x, y)) {
			containedBy = true;
		}

		return containedBy;
	}
};

class fastcontainedby: public containedby {
	// Uses an in-memory RTree to speed up the spatial join

	RTree<CellId, float, 2> tree;

public:
	virtual void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j)
	{
		AssignmentFunction::setEnvironment(t, i, s, j);
		assert(i == 0);
		assert(S->GetGrid()->getdim() == 2);

		tree.RemoveAll();

		Tuple *tup;
		Tuple polytup(&schsource);

		for (unsigned int i = 0; i < S->Size(this->_j); i++) {
			dssource->FastBindTuple(i, polytup);
			typedef vector<Tuple> Poly;
			Poly *poly;

			poly = *(Poly **) polytup.get("poly");
			int s = poly->size();

			float x, y;
			float min[2];
			float max[2];

			tup = &(*poly)[0];
			x = *(float *) tup->get("x");
			y = *(float *) tup->get("y");
			min[0] = x;
			min[1] = y;
			max[0] = x;
			max[1] = y;

			for (int j = 1; j < s; j++) {
				tup = &(*poly)[j];
				x = *(float *) tup->get("x");
				y = *(float *) tup->get("y");
				if (x < min[0]) min[0] = x;
				if (x > max[0]) max[0] = x;
				if (y < min[1]) min[1] = y;
				if (y > max[1]) max[1] = y;
			}

			tree.Insert(min, max, i); // Note, all values including zero are fine in this version
		}
	}

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{

		float x, y;
		float coords[2];

		x = *(float *) dstarget->GetAttributeVal("x", c);
		y = *(float *) dstarget->GetAttributeVal("y", c);

		coords[0] = x;
		coords[1] = y;

		set<CellId> hits;

		assert((unsigned )tree.Search(coords, coords, TestCallback, &hits) == hits.size());

		set<CellId>::iterator it;
		for (it = hits.begin(); it != hits.end(); it++) {
			if (PointInCell(x, y, *it)) {
				cout << "cell id containing:" << *it << endl;
				out.push_back(*it);
			}
		}
	}

};

class contains: public AssignmentFunction {
	//from source points to target polygons
public:

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		assert(T->GetGrid()->getdim() == 2);
		typedef vector<Tuple> Poly;
		Poly *poly;

		Tuple *t;
		Tuple polytup(&schtarget);
		dstarget->FastBindTuple(c, polytup);
		poly = *(Poly **) polytup.get("poly");

		int s = poly->size();

		float *xs = new float[s];
		float *ys = new float[s];

		for (int j = 0; j < s; j++) {
			t = &(*poly)[j];
			xs[j] = *(float *) t->get("x");
			ys[j] = *(float *) t->get("y");
			//cout << " | " << xs[j] << ", " << ys[j];
		}
		//cout << "----" << endl;

		float x;
		float y;
		for (unsigned int i = 0; i < S->Size(this->_j); i++) {
			x = *(float *) dssource->GetAttributeVal("x", i);
			y = *(float *) dssource->GetAttributeVal("y", i);
			if (pnpoly(s, xs, ys, x, y)) {
				out.push_back(i);
			}
		}

	}
};

class pointpoly: public AssignmentFunction {

public:
	float xs[10];
	float ys[10];

	/* Expects 2-dimensional gridfields with geometry
	 * defined on the nodes as attributes "x" and "y"
	 * Returns the nodes incident to the cells in S that
	 * contain the target cell.
	 * Requires that the source grid be normalized
	 * maximum polygon size is 10 nodes.
	 */

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		assert(S->GetGrid()->getdim() == 2);
		int s = 0;
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);
		float x = *(float *) t.get("x");
		float y = *(float *) t.get("y");
		AbstractCellArray *twocells = S->GetGrid()->getKCells(2);
		Cell *Sc;

		int zt, zs, b;
		int *hck;
		int tpos;

		//cout << "checking point " << c.nodes[0] << "...." << flush;
//      if (c.nodes[0] >= 94) cout << "("<< x << "," <<y<<")" << flush;
		int size = twocells->getsize();
		for (int i = 0; i < size; i++) {
			Sc = twocells->getCell(i);
			s = Sc->getsize();
//          if (S->card() <10000)
			//          cout << "cell----" << endl;
			for (int j = 0; j < s; j++) {
				xs[j] = *(float *) dssource->GetAttributeVal("x", Sc->getnodes()[j]);
				ys[j] = *(float *) dssource->GetAttributeVal("y", Sc->getnodes()[j]);
			}
			// cout << "----" << endl;
			if (pnpoly(s, xs, ys, x, y)) {
				for (int j = 0; j < s; j++) {
					if (dssource->IsAttribute("zpos") && dstarget->IsAttribute("zpos")) {
						zs = *(int *) dssource->GetAttributeVal("zpos", Sc->getnodes()[j]);
						zt = *(int *) dstarget->GetAttributeVal("zpos", T->GetGrid()->getKCells(this->_i)->getOrd(c));
						if (zt == zs) {
							b = *(int *) dssource->GetAttributeVal("b", Sc->getnodes()[j]);
							if (zs >= b) {
								out.push_back(Sc->getnodes()[j]);
							}
						}
					}
					else {
						out.push_back(c);
					}

					//Rewrite this garbage.

					if (dssource->IsAttribute("hack")) {
						tpos = c;
						hck = (int *) dssource->GetAttribute("hack")->getValPtr(tpos);
						//cout << "changed?" << endl;
						//cout << *hck << endl;
						*hck = 1;
						//                *hck = twocells->getOrd(*Sc);
						//cout << *hck << endl;
						//hck = (int *) dstarget->GetAttribute("hack")->getValPtr(tpos);
						//cout << *hck << endl;
					}
				}

			}
		}
	}

};

class inverse_pointer: public AssignmentFunction {
	/* an attribute in the source grid refs a cellid in the target grid
	 */
public:
	string attr;
	inverse_pointer(string a)
	{
		attr = a;
	}
	;

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		int k = this->_i;
		int id = T->GetGrid()->getKCells(k)->getOrd(c);

		//search for id in source atribute
		S->lookupInt(attr, _j, id, out);

		// cout << id << ", assigned: " << out.size() << endl;
	}
};

class bypointer: public AssignmentFunction {
public:
	string attr;
	bypointer(string a)
	{
		attr = a;
	}
	;

	virtual void operator()(const CellId &c, vector<CellId>&out)
	{
		Tuple t(&schtarget);
		dstarget->FastBindTuple(c, t);
		Scheme *sch = &schtarget;
		unsigned int p;
		if (sch->getType(this->attr) == FLOAT) {
			p = int(*(float *) t.get(this->attr));
		}
		else {
			p = *(int *) t.get(this->attr);
		}

		if (/* p>=0 && p is unsigned jhrg 10/5/11 */p < S->Size(this->_j)) {
			out.push_back(p);
		}
	}
};
/*
 class cross : public AssignmentFunction {
 AbstractCellArray *kcells ;
 AbstractCellArray *searchcells;
 CrossNodeMap h;
 public:
 cross(GridField *gf) {
 this->kcells = gf->GetGrid()->getKCells(gf->rank());
 this->kcells->ref();
 }
 cross(Grid *g, int k) {
 this->kcells = g->getKCells(k);
 }
 ~cross() { this->kcells->unref(); }
 // each cell c in target assigned cells c X G_k
 void setEnvironment(GridField *T, GridField *S){
 this->S = S;
 this->T = T;

 AbstractCellArray *jcells = T->GetGrid()->getKCells(T->rank());
 h.setInputs(jcells, kcells);

 searchcells = S->GetGrid()->getKCells(S->rank());
 searchcells->buildInvertedIndex();
 jcells->buildInvertedIndex();
 };
 virtual void cross::operator()(const CellId &cid, vector<CellId>&out) {

 //cout << "cross" << endl;
 //cout << grid->getName() << endl;
 //cout << "cross" << endl;

 Cell e(10);
 Cell *c1;
 AbstractCellArray *jcells = T->GetGrid()->getKCells(T->rank());
 Cell *c = jcells->getCell(cid);
 int s;
 for (int i=0; i< kcells->getsize(); i++) {
 c1 = kcells->getCell(i);
 s = e.getsize();
 c.Cross2(*c1, h, s, e.getnodes());
 if (this->searchcells->contains(e)) {
 //e.print();
 //cout << this->searchcells->getOrd(e) << endl;
 //getchar();
 out.push_back(e);
 //} else {
 //  cout << "~ ";
 }
 }
 //if (out.size() == 0) c.print();
 //getchar();
 }

 };
 */
class unify: public AssignmentFunction {
public:
	virtual void operator()(const CellId &, vector<CellId>&out)
	{
		//map all cells of S to the given CellId.
		//used by unify and as a shortcut to cross->aggregate
		int k = this->_j;
		AbstractCellArray *kcells = S->GetGrid()->getKCells(k);
		for (unsigned int i = 0; i < kcells->getsize(); i++) {
			out.push_back(i);
		}
	}
};

class ident: public AssignmentFunction {
public:
	virtual void operator()(const CellId &cid, vector<CellId>&out)
	{
		out.push_back(cid);
	}

};

} //namespace

#endif
