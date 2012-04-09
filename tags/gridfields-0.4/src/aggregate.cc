#include "aggregate.h"
#include "timing.h"
#include <math.h>
#include "array.h"

void debug_set(UnTypedPtr p) {
  vector<Tuple *> *vec = (vector<Tuple *> *) p;
  cout << "debug_set: " << vec << ", " << vec->size() << endl;
  vector<Tuple *>::iterator i;
  for (i=vec->begin(); i!=vec->end(); i++) {
    (*i)->print();
  }
}

AggregateOp::AggregateOp(GridFieldOperator *T, Dim_t i,
              AssignmentFunction *m,
              AggregationFunction *f,
              GridFieldOperator *S, Dim_t j) {
  this->A = NULL;
  this->B = NULL;
  this->LeftOp = T;
  this->RightOp = S;
  this->_i = i;
  this->_j = j;
  this->m = m;
  this->f = f;
}

void AggregateOp::Execute() {
  this->PrepareForExecution();
  Result =  Aggregate(this->A, 
                      this->_i,
     		      *this->m, 
     		      *this->f, 
		      this->B,
                      this->_j);
}

GridField *AggregateOp::Aggregate(GridField *Tt, Dim_t _i,
 				  AssignmentFunction &m, 
 				  AggregationFunction &f,
				  GridField *Ss, Dim_t _j) {
  Grid *T = Tt->GetGrid();
  Grid *S = Ss->GetGrid();
  int j;

  string gname = newName(T->name, S->name);


  GridField *Out;
  Out = new GridField(Tt);
  
  Scheme *ssch = new Scheme(Ss->GetScheme(_j));
  
  f.setScheme(ssch);
  
  //allocate space for any new attributes being created.
  Scheme *outscheme = f.getOutScheme();
  //cout << "scheme: " << Tt->GetGrid()->Size(_i) << endl; 
  Out->CoerceScheme(_i, outscheme, Tt->GetGrid()->Size(_i));
  Scheme finalscheme = Out->GetScheme(_i);
  
  m.setEnvironment(Out, _i, Ss, _j);

  vector<CellId> cs;
  vector<Tuple> vs;

  vector<Tuple>::iterator tp;
  vector<CellId>::iterator cp;

  //target tuple holder
  Tuple t(&finalscheme);
  //source tuple holder
  Tuple s(ssch);
  
  const Dataset &ds = Out->GetDataset(_i);
  const Dataset &sourceds = Ss->GetDataset(_j);
  
  for (size_t i=0; i<Out->Size(_i); i++) {

    m(i, cs);

    if (vs.size() != cs.size()) {
      vs.resize(cs.size(), s);
    }
    
    j=0;
    FOR(vector<CellId>, c, cs) {
      sourceds.FastBindTuple(*c, vs[j++]);
    }
//    Ss->BindTuples(cs, vs);
    
    ds.FastBindTuple(i, t);
    f(vs, t);
    //t.print();
    //getchar();

    cs.clear();

  }

  //cout << "Aggregate()" << endl;
  return Out;
}

string AggregateOp::newName(string Tname, string Sname) {

  string gname = "a(" + Tname + ", " + Sname + ")";
  return gname;
}

