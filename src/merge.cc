#include "merge.h"
#include "array.h"
#include "timing.h"


MergeOp::MergeOp(GridFieldOperator *left, GridFieldOperator *right) {
  this->A = NULL;
  this->B = NULL;
  this->LeftOp = left;
  this->RightOp = right;
}
    
void MergeOp::Execute() {
  this->PrepareForExecution();
  Result = this->Merge(this->A, this->B);
}

GridField *MergeOp::Merge(GridField *Aa, GridField *Bb) {

  Grid *G;
  GridField *Gg;
  Grid *A = Aa->GetGrid();
  Grid *B = Bb->GetGrid();
  cout << "Merge(...)" << endl;
//  Aa->getAttribute("salt")->print();
//  Bb->getAttribute("sumsalt")->print();
  string gname = newName(A->name, B->name);

  if (A == B) {
    Gg = new GridField(Aa);
    for (Dim_t k=0; k<=Aa->Dim(); k++) {
      Scheme sch = Bb->GetScheme(k);
      for (unsigned int i=0; i<Bb->Arity(k); i++) {
        Gg->Bind(k, Bb->GetAttribute(k, sch.getAttribute(i)));
      }
    }
  } else {
     
    G = A->Intersection(B);

    Gg = new GridField(G);

    if (G->empty()) return Gg;
 
    Gg->RestrictAll(*Aa);
    Gg->RestrictAll(*Bb);

  }
  return Gg;
}


string MergeOp::newName(string Aname, string Bname) {

  string gname = "m(" + Aname + ", " + Bname + ")";
  return gname;
}
