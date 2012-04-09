#include "tag.h"
#include "timing.h"
#include "array.h"


TagOp::TagOp(GridFieldOperator *left, Dim_t i, GridFieldOperator *right, Dim_t j) 
                  : BinaryGridFieldOperator(left, right), idx(0), _i(i), _j(j) {}

void TagOp::Execute() {
  this->PrepareForExecution();  

  string attr;
  Scheme sch = A->GetScheme(this->_i);
  int temp = 0;
  for (unsigned int i=0; i<sch.size(); i++) {
    attr = sch.getAttribute(i);
    if (consts.find(attr) != consts.end()) {
      consts[attr]->unref();
      consts.erase(attr);
    }
    consts[attr] = new ConstArray(
        attr, 
        B->Size(_j), 
        sch.getType(attr), 
        &temp);
  }
  
  Result = Tag(this->A,this->_i,this->B, this->_j, idx);

  if (1 + idx++ < A->Size(this->_i) ) {
    this->Update();
  }
}

GridField *TagOp::Tag(GridField *Aa, Dim_t i, GridField *Bb, Dim_t j, int idx) {
  GridField *out = new GridField(Bb);
  
  //cout << "idx: " << idx << endl;
  string attr;
  ConstArray *arr;
  UnTypedPtr v;
  map<string, ConstArray *>::iterator p;
  for (p=consts.begin(); p!=consts.end(); p++) {
    attr = p->first;
    arr = p->second;
    v = Aa->GetAttributeValue(i, attr, idx);
    arr->setConst(v);
    out->Bind(j, arr);
  }

  return out;  
}


TagOp::~TagOp() {
  map<string, ConstArray *>::iterator p;
  for (p=consts.begin(); p!=consts.end(); p++) {
    p->second->unref();
  }
}

string TagOp::newName(string Aname, string Bname) {
  return "Tag(" + Aname + ", " + Bname + ")";
}
