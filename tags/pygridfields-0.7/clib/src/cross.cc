#include "cross.h"
#include "crossordmap.h"
#include "timing.h"
#include "array.h"


CrossOp::CrossOp(GridFieldOperator *left, GridFieldOperator *right) {
  this->A = NULL;
  this->B = NULL;
  this->LeftOp = left;
  this->RightOp = right;
}

void CrossOp::Execute() {
  this->PrepareForExecution();  
  Result = Cross(this->A,this->B);
}

GridField *CrossOp::Cross(GridField *Aa, 
			  GridField *Bb) {
  
  Grid *A = Aa->GetGrid();
  Grid *B = Bb->GetGrid();

  /*
  assert( (Aa->rank() == A->getdim() && Bb->rank() == B->getdim())
	  || 
	  (Aa->rank() == 0 && Bb->rank() == 0) 
	);
  */


  Grid *G;
  GridField *Gg;
  
  if (A->empty() || B->empty()) {
    G = new Grid("empty", 0);
    Gg = new GridField(G);  
  } else {
    G = Aa->GetGrid()->Cross(Bb->GetGrid());
    //  cout << gettime() - start << '\t' << "( Cross(Grid) )" << endl;;
    Gg = new GridField(G);
  }
  
  crossData(Gg, Aa, 0, Bb, 0);
  if (Aa->Dim() + Bb->Dim() > 0) {
    crossData(Gg, Aa, Aa->Dim(), Bb, Bb->Dim());
  }
  
  //cout << gettime() - start << '\t' << "( Cross(Data) )" << endl;;

  CrossOrdMap *ordmap = new CrossOrdMap(A, B, Gg);
  G->setReferent(ordmap);

  G->unref();
  
  return Gg;

}

void CrossOp::crossData(GridField *Gg, GridField *Aa, Dim_t i, GridField *Bb, Dim_t j) {
  int Asize = Aa->Card(i);
  int Bsize = Bb->Card(j);

  Scheme Asch = Aa->GetScheme(i);
  Scheme Bsch = Bb->GetScheme(j);

  string attr;
  Array *temparr;

  float start = gettime();(void)start;
  for (unsigned int a=0; a<Aa->Arity(i); a++) {
    attr = Asch.getAttribute(a);
    temparr = Aa->GetAttribute(i, attr)->expand(Bsize);
    Gg->Bind(i+j, temparr);
    temparr->unref();
  }

  for (unsigned int b=0; b<Bb->Arity(j); b++) {    
    attr = Bsch.getAttribute(b);
    temparr = Bb->GetAttribute(j, attr)->repeat(Asize);
    Gg->Bind(i+j, temparr);
    temparr->unref();
  }

}

string CrossOp::newName(string Tname, string Sname) {

  string gname = "a(" + Tname + ", " + Sname + ")";
  return gname;
}

