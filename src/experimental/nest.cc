#include "merge.h"
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
  Grid *A = Aa->grid;
  Grid *B = Bb->grid;

  string gname = newName(A->name, B->name);

  if (A == B) {
    Gg = new GridField(Aa);
    Scheme *sch = Bb->getScheme();
    for (int i=0; i<Bb->getarity(); i++) {
      Gg->Bind(Bb->getAttribute(sch->getAttribute(i).c_str()));
    }
  } else {
     
    float start = gettime();
    //cout << "A size: " << A->getKCells(0)->getsize() << endl;
    //cout << "B size: " << B->getKCells(0)->getsize() << endl;
    G = A->Intersection(B);
    //cout << "Intersection time: " << gettime() - start << "\n";

    //cout << "G size: " << G->getKCells(0)->getsize() << endl;
    Gg = new GridField(G, Aa->k);

    if (G->empty()) return Gg;

    vector<Array *> attributes;
    
    Aa->subGridData(G, attributes);
    Gg->Bind(attributes);
    //cout << "Grid A time: " << gettime() - start << "\n";

    attributes.clear();

    if (Bb->k == Aa->k) {
      Bb->subGridData(G,attributes);
      Gg->Bind(attributes);
      //cout << "Grid B time: " << gettime() - start << "\n";
    }
  }

  return Gg;
}


string MergeOp::newName(string Aname, string Bname) {

  string gname = "m(" + Aname + ", " + Bname + ")";
  return gname;
}
