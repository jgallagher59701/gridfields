#ifndef _AGGREGATE_H
#define _AGGREGATE_H

#include "config.h"

#include <iostream>
#include "tuple.h"
#include "gridfield.h"
#include "gridfieldoperator.h"
#if HAVE_TR1_FUNCTIONAL
#include <tr1/functional>
#endif
// old code jhrg #include <ext/functional>
#include <string>

namespace GF {

//typedef pointer_to_unary_function<Cell, Tuple *> GridFunction;

void debug_set(UnTypedPtr p);
class AssignmentFunction : 
 virtual public unary_function<CellId&, vector<Cell>&> {
 public:
 
  virtual ~AssignmentFunction() {}; 
  GridField *T;
  GridField *S;
  Dim_t _i, _j;
  const Dataset *dstarget, *dssource;
  Scheme schtarget;
  Scheme schsource;
  
  virtual void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j) { 
    T=t; S=s; _i=i; _j=j;
    dstarget = &T->GetDataset(_i);
    dssource = &S->GetDataset(_j);
    schtarget = dstarget->GetScheme();
    schsource = dssource->GetScheme();
  }
  virtual void operator()(const CellId &, vector<CellId> &) = 0;
  
};

class AggregationFunction : 
 virtual public unary_function<vector<Tuple>&, Tuple> {
 public:

  virtual ~AggregationFunction() {}; 
  void setScheme(Scheme *sch) { 
    inscheme = sch; 
    outscheme = getOutScheme(inscheme); 
  }
  Scheme *getOutScheme() { return outscheme; }
  virtual Scheme *getOutScheme(Scheme *inscheme) = 0;

  virtual void operator()(vector<Tuple> &, Tuple &) = 0;
/*
  ~AggregationFunction() {
    delete outscheme;
  }
*/
 private:
  Scheme *inscheme;
  Scheme *outscheme;
};

class AggregateOp : public BinaryGridFieldOperator {
 public:
  
  AggregateOp(GridFieldOperator *T, Dim_t i,
              AssignmentFunction *m, 
              AggregationFunction *f, 
              GridFieldOperator *S,
              Dim_t j);
  
  static GridField *Aggregate(GridField *T, Dim_t i,
     			      AssignmentFunction &m, 
     			      AggregationFunction &f,
			      GridField *S, Dim_t j);
 protected:
  AssignmentFunction *m;
  AggregationFunction *f;
  Dim_t _i,_j;
  
  void Execute(); 
 private:
  static string newName(string Aname, string Bname);

};

} // namespace GF

#endif /* AGGREGATE_H */
