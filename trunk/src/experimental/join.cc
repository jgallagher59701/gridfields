//#include "join.h"
#include "crossordmap.h"
#include "crossnodemap.h"
#include "timing.h"
#include "expr.h"
#include "gridfield.h"
#include "array.h"

JoinOp::JoinOp(Condition *p, GridFieldOperator *left, GridFieldOperator *right) {
  this->A = NULL;
  this->B = NULL;
  this->condition = p;
  this->LeftOp = left;
  this->RightOp = right;
}
    
void JoinOp::Execute() {
  
  this->PrepareForExecution();
  Result =  Join(this->condition,
		 this->A, 
		 this->B);
}

GridField *JoinOp::Join(Condition *p,
			GridField *Aa, 
			GridField *Bb) {
  Grid *A = Aa->grid;
  Grid *B = Bb->grid;

  assert( (Aa->rank() == A->getdim() && Bb->rank() == B->getdim()) ||  (Aa->rank() == 0 && Bb->rank() == 0) );
  
  string gname = newName(A->name, B->name);
  Grid *G = new Grid(gname, A->getdim() + B->getdim());
  GridField *Gg = new GridField(G, Aa->rank() + Bb->rank());

  int arityA = Aa->getarity();
  int arityB = Bb->getarity();

  int cardA = Aa->card();
  int cardB = Bb->card();

  Scheme *schA = Aa->getScheme();
  Scheme *schB = Bb->getScheme();

  CrossNodeMap h(A->getKCells(0), B->getKCells(0));

  Operand &o1 = p->left;
  Operand &o2 = p->right;

  const char *var1 = o1.var;
  const char *var2 = o2.var;

  Type t;
  if (o1.type != o2.type) {
    Fatal("Type mismatch");
  } else {
    t = o1.type;
  }

  if (o1.tag == VAR) {
    if (!Aa->isAttribute(o1.var)) {
	Fatal("%s is not an attribute of the left-hand gridfield.",o1.var);
    }
  } else {
    Fatal("%s is not a variable operand, so do not use Join");
  }

  if (o2.tag == VAR) {
    if (!Bb->isAttribute(o2.var)) {
	Fatal("%s is not an attribute of the right-hand gridfield.",o2.var);
    }
  } else {
    Fatal("%s is not a variable operand, so do not use Join");
  }
  
  UnTypedPtr valA;
  UnTypedPtr valB;

  bool keep;

  AbstractCellArray *kacells = A->getKCells(Aa->rank());
  AbstractCellArray *kbcells = B->getKCells(Bb->rank());
  CellArray *kcells = (CellArray *) G->getKCells(Aa->rank() + Bb->rank());
  Cell *acell;
  Cell *bcell;
  Cell *c;

  //Estimate selectivity to pre-allocate
  int estimate = int(Aa->card() * Bb->card() * 0.5);
  vector< vector <float> > gdata;
  gdata.resize(arityA + arityB);
  for (int k=0; k<arityA + arityB; k++) {
    gdata[k].reserve(estimate);
  }

  //Nested Loop join...
  for (int i=0; i<cardA; i++) {
    valA = Aa->getAttributeVal(var1, i);
    acell = kacells->getCell(i);

    for (int j=0; j<cardB; j++) {
      valB = Bb->getAttributeVal(var2, j);
      
      keep = (bool) p->evalCondition(p->op, valA, valB, t);
      //      cout << *(float *) valA << ", " << *(float *)valB << ": " << keep << endl;
      if (keep) {
	//build a k_c-cell from a k_a-cell and a k_b-cell
	bcell = kbcells->getCell(j);
	//	cout << "A cell: " << endl;
	//	acell->print(1);
	//	cout << "B cell: " << endl;
	//	bcell->print(1);
	c = acell->Cross(*bcell, h);
	//	c->print(3);
	kcells->addCell(c);
	
	//get an a_tuple and a b_tuple, concatenate them
	for (int k=0; k<arityA; k++) {
	  gdata[k].push_back(*(float *) Aa->getAttributeVal(schA->getAttribute(k).c_str(), i));
	}
	for (int k=arityA; k<arityB+arityA; k++) {
	  gdata[k].push_back(*(float *)Bb->getAttributeVal(schB->getAttribute(k-arityA).c_str(), j));
	}
      }
    } 
  }


  G->setKCells(kcells, Aa->rank()+Bb->rank());

  //Now make arrays out of the vectors of untyped data.
  Array *attr;
  string name;
  Scheme *sch;
  float *untypeddata;

  for (int k=0; k<arityA+arityB; k++) {

    if (k<arityA) {
      sch = schA;
      name = sch->getAttribute(k);
    } else {
      sch = schB;
      name = sch->getAttribute(k-arityA);
    }

    attr = new Array(name.c_str(), sch->getType(name));

    untypeddata = new float[gdata[k].size()];
    vector<float> &column = gdata[k];
    for (unsigned int i=0; i<column.size(); i++) {
      untypeddata[i] = column[i];
    }
    attr->setVals((UnTypedPtr) untypeddata, gdata[k].size());
    Gg->Bind(attr);
  }

  //build the other cells, maintaining well-formedness

  if (Aa->rank() == Bb->rank() && Aa->rank() == 0) {
    set<Node> goodnodes;

    kcells->toNodeSet(goodnodes);
    CellArray *dcells = new CellArray();

    kacells = A->getKCells(A->getdim());
    kbcells = B->getKCells(B->getdim());    
//    kacells = A->getKCells(k);
//    kbcells = B->getKCells(Aa->getdim() + Bb->getdim() - k);    
    for (int i=0;i<kacells->getsize(); i++) {
      acell = kacells->getCell(i);
      for (int j=0;j<kbcells->getsize(); j++) {
	bcell = kbcells->getCell(j);
	c = Cross_if_wellformed(*acell, *bcell, goodnodes, h);
	if (c!=NULL) {
	  dcells->addCell(c);
	}
      }
    }
    G->setKCells(dcells, A->getdim() + B->getdim());
  }
  // else {

  /*
  for (int i=1; i<A->getdim(); i++) {
    for (int j=1; i<B->getdim(); j++) {
      kacells = A->getKCells(i);
      kbcells = B->getKCells(j);
      
    }    
  }
*/
  return Gg;

}

Cell *JoinOp::Cross_if_wellformed(Cell &a, 
				  Cell &b, 
				  set<Node> &nodes, 
				  CrossNodeMap &h) {
  Node *an = a.getnodes();
  Node *bn = b.getnodes();
  for (int i=0; i<a.getsize(); i++) {
    for (int j=0; j<b.getsize(); j++) {
      if (nodes.find(h.map(an[i], bn[j])) == nodes.end()) {
	return NULL;
      }
    }
  }
  return a.Cross(b, h);
}

string JoinOp::newName(string Tname, string Sname) {

  string gname = "j(" + Tname + ", " + Sname + ")";
  return gname;
}

