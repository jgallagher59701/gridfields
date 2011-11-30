#include <cstring>
#include "src/grid.h"
#include "src/gridfield.h"
#include "src/array.h"
#include "src/restrict.h"
#include "src/merge.h"
#include "src/aggregations.h"
#include "src/apply.h"
#include "src/accumulate.h"
#include "src/project.h"
#include "src/assignments.h"

Grid *makeGrid(int scale,const string name) {
  CellArray *twocells;
  CellArray *onecells;
  CellArray *zerocells;
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;

  Cell *c;
  vector<Cell> cs;

  bool wf;
  int i;
  twocells = new CellArray();
  triangle[0] = 2;
  triangle[1] = 0;
  triangle[2] = 1;
  twocells->addCellNodes(triangle, 3);
  for (i=1; i<scale/2; i++) {
    triangle[0] = scale-i;
    triangle[1] = scale-1-i;
    triangle[2] = scale-2-i;
    c = twocells->addCellNodes(triangle, 3);
    cs.push_back(*c);
  }
/*
  sort(cs.begin(), cs.end());
  for (int i=0; i<cs.size(); i++) {
      cs[i].print(3);
  }
  */
  //twocells->print();
  //getchar(); 
  onecells = new CellArray();
  for (i=0; i<scale-1; i++) {
    segment[0] = i;
    segment[1] = i+1;
    onecells->addCellNodes(segment, 2);
  }
  //onecells->print();
  
  //getchar(); 
  grid = new Grid(name, 2);
  grid->setImplicit0Cells(scale);
  grid->setKCells(onecells, 1);
  grid->setKCells(twocells, 2);
  //grid->print(0);
  //getchar();
  return grid; 
}

Array *makeFloatArray(int size,const char *name) {
  Array *arr;
  arr = new Array(name, FLOAT, size);
  float *data;
  arr->getData(data);
  int i;

  for (i=0; i<size; i++) {
      data[i] = 2*i-10;
  }
  
  return arr;  
}

GridField *makeGridField(int size,const string gridname,const char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(12, gridname);
  AbstractCellArray *cells = G->getKCells(k);
  data = makeFloatArray(cells->getsize(), datname);

  GF = new GridField(G);
  GF->AddAttribute(k, data);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {
  
  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  GridField *GF;
  GridField *GF2;
  GridField *rGF1;
  GridField *rGF2;
  GridField *mGF;
  GridField *result;

  GF = makeGridField(12, "A", "x", 0);
  GF2 = makeGridField(12, "A", "x", 2);

  if (verbose) GF2->PrintTo(cout, 5);
  Assign::IncidentTo inci;
  Aggregate::Any mx("x", 4);
  GridField *R = AggregateOp::Aggregate(GF2, 0, inci, mx, GF2, 2);
  if (verbose) R->PrintTo(cout, 5);
  
#if 0
  //Assign::neighbors n;
  Assign::Nodes n;
  //Aggregate::_average<float> a("x");
  Aggregate::mkvector a("poly");
  //Aggregate::first a;
    
  //  GF->getAttribute("x")->print();
  GridField *res = AggregateOp::Aggregate(GF2, 2, n, a, GF, 0); 
  //res->PrintTo(cout, 5);
  //  result->getAttribute("x")->print();
  vector<Tuple> *poly = *(vector<Tuple> **)res->GetAttributeValue(2,"poly",0);
  if (verbose) cout << "main: ";
  if (verbose) cout << poly << endl;
  if (verbose) (*poly)[0].print();
  
  //cout << *(float *)(((*poly)[0])->get("x")) << endl;
  //cout << *(float *)((*poly)[0]).get("x") << endl; //", " << *(float *)((**poly)[1]).get("x") << ", " << *(float *)((**poly)[2]).get("x") << endl;
  
  
  //GF2->getScheme()->print();
  // result = ApplyOp::Apply(GF2, "result", "x<-5");
  //result->print();
  
  result = AccumulateOp::Accumulate(GF2, 2, "result", "result+1", "0", 0);
  result->PrintTo(cout, 5);
  result = AccumulateOp::Accumulate(GF2, 2, "z", "z+x", "x", 0);
  result->PrintTo(cout, 5);
  //
  //  result = ProjectOp::Project(GF2, vector<string>(1, string("x")));
  //  result->getScheme()->print();
  //ProjectOp::Project(GF2, string("x"))->getScheme()->print();
  // result->getScheme()->print();

  //  ApplyOp *apply = new ApplyOp::ApplyOp(GF2, "aa", "x+2/x");
  //  apply->Execute();
  //  apply->getResult()->print();
  
  //  ApplyOp *apply = new ApplyOp::ApplyOp(GF2, "x=x+1");
  //  GridField *ans = apply->getResult();
  //  ans->print();
#endif
}

