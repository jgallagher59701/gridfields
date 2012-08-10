#include "visualize.h"
#include "vtkGridField.h"
#include "grid.h"
#include "onegrid.h"
#include "gridfield.h"
#include "array.h"
#include "arrayreader.h"
#include "tag.h"
#include "iterate.h"
#include "state.h"
#include "apply.h"
#include "merge.h"
#include "bind.h"
#include "cross.h"
#include "sift.h"
#include "restrict.h"
#include "elcircfile.h"
#include "accumulate.h"
#include "aggregations.h"
#include "assignments.h"
#include <sstream>
#include "timing.h"
#include "connect.h"
#include "catalog.h"
#include "bind2.h"
#include "bindconstant.h"
#include "datadump.h"
#include "output.h"
#include "scaninternal.h"

#include "vtkGlyph3D.h"
#include "vtkSphereSource.h"

//#include "vmon.h"
using namespace GF;

GridField *makeMesh() {
  Grid *grid;
  Node triangle[3];
  
  int ring[6] = {1,2,3,4,5,6};
  
  CellArray *twocells = new CellArray();
  triangle[0] = 0;
  for (int i=0; i<6; i++) {
    triangle[1] = ring[i];
    triangle[2] = ring[(i+1) % 6];
    twocells->addCellNodes(triangle, 3);
  }

  grid = new Grid("mesh", 2);
  grid->setImplicit0Cells(7);
  grid->setKCells(twocells, 2);

  float x[7] = {0.5, 0, 0, 0, 1, 1, 1};
  float y[7] = {1, 0, 1, 2, 2, 1, 0};
  float v[7] = {0, 9, 9, 9, 9, 9, 9};

  Array *ax = new Array("x", FLOAT);
  ax->copyFloatData(x, 7);

  Array *ay = new Array("y", FLOAT);
  ay->copyFloatData(y, 7);

  Array *av = new Array("v", FLOAT);
  av->copyFloatData(v, 7);
  
  GridField *gf = new GridField(grid);
  
  gf->Bind(0, ax);
  gf->Bind(0, ay);
  gf->Bind(0, av);

  return gf;
}

Grid *makeGrid(int scale, char *name) {
  CellArray *twocells;
  CellArray *onecells;
  CellArray *zerocells;
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;

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
    twocells->addCellNodes(triangle, 3);
  }

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
Array *makeIntArray(int size, char *name) {
  Array *arr;
  int data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = 2*i-10;
  }
  arr = new Array(name, INT);
  arr->copyIntData(data, size);
  return arr;  
}


Array *makeFloatArray(int size, char *name) {
  Array *arr;
  float data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = 2*i-10;
  }
  arr = new Array(name, FLOAT);
  arr->copyFloatData(data, size);
  return arr;  
}

GridField *makeGridField(int size, char *gridname, char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(size, gridname);
  AbstractCellArray *cells = G->getKCells(k);
  data = makeIntArray(G->Size(k), datname);

  GF = new GridField(G, k, data);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int toy() {
  
  GridField *Aa;
  GridField *Bb;

  OneGrid *A = new OneGrid("A", 3);
  Array *a = makeFloatArray(3, "a");
  OneGrid *B = new OneGrid("B", 3);
  Array *b = makeFloatArray(3, "b");
  
  Aa = new GridField(A, 0, a);
  Bb = new GridField(B, 0, b);
  
  ApplyOp *init = new ApplyOp(Bb, "asum=0;bsum=0");
  StateOp *state = new StateOp(init);
  
  TagOp *tag = new TagOp(Aa,Bb);

  MergeOp *merge = new MergeOp(state, tag);
  
  
  ApplyOp *func = new ApplyOp(merge, "asum=a+asum;bsum=bsum+b");
  
  IterateOp *iter = new IterateOp(Aa, func, state);
  GridField *Result = iter->getResult();
    
  Result->print();
}

string s("123");

class Foo {
  public: 
    int nums[5] ;
    Foo() {
      for (int i=0; i<5; i++) {
        nums[i]=i+1;
      }
    };
};


static int foo(Foo *f) {
//  static int nums[5] = {1,2,3,4,5};
  cout << f->nums[2] << endl;
  cout << f->nums[6] << endl;
  //nums[5] = 6;
}

int test() {

  //cout << s[5] << endl;
  //int nums[5] = {1,2,3,4,5}; 
//  string r("456");
//  Foo f;
//  foo(&f);
//  exit(0);

  ElcircFile ef("/home/howew/data/1_salt.63");
  GridField *H = ef.readHGrid();
  
  GridField *V = ef.readVGrid();
// ApplyOp *fooV = new ApplyOp(V, "foo=z+z");
// fooV->getResult()->print();
  
  AccumulateOp *ordH = new AccumulateOp(H, "hpos", "hpos+1", "0");
  //ordH->getResult()->GetAttribute(0, "hpos")->print();

  stringstream ss;
  ss << "column-b+" << V->card() << endl;
  AccumulateOp *column = new AccumulateOp(ordH, "column", ss.str(), "0");
  column->SetOffset(-1);

//  RestrictOp *zoom = new RestrictOp("(column < 1028) & (column > 920)", 0, column);
//  one->getResult()->print();
//  getchar();

  
  RestrictOp *zoom = new RestrictOp("(x<350000)&(x>330000)&(y>280000)&(y<310000)", 0, column);
  cout << "Loop size: " << zoom->getResult()->card() << endl;
  //getchar();

  AccumulateOp *ordV = new AccumulateOp(V, "vpos", "vpos+1", "0");

  TagOp *tag = new TagOp(zoom, ordV);
  
  ApplyOp *addrV = new ApplyOp(tag, "addr=column+vpos-b");
  RestrictOp *restrict = new RestrictOp("vpos > (b-1)", 0, addrV);

  FileArrayReader *ar = ef.getVariableReader(0, "addr");
  BindOp *bind = new BindOp("salt", FLOAT, ar, restrict); 

  RestrictOp *surf = new RestrictOp("salt>-99", 0, bind);
  surf->getResult()->GetAttribute(0, "salt")->print();
  surf->getResult()->GetAttribute(0, "vpos")->print();

  //attribute to sum, and value to use when summing an empty set.
  Aggregate::_sum<float> sum("salt", -88);
  Assign::inverse_pointer iptr("vpos");
  AggregateOp *agg = new AggregateOp(ordV, 0, &iptr, &sum, surf, 0);
  agg->getResult()->GetAttribute(0, "sumsalt")->print();
  
  
  ApplyOp *init = new ApplyOp(ordV, "sum=0; count=0; bottomcount=0; surfcount=0");
  StateOp *state = new StateOp(init);
  
  MergeOp *merge = new MergeOp(state,agg);

  ApplyOp *add = new ApplyOp(merge, "sum=sum+sumsalt*(sumsalt>-80); count=count+(sumsalt>-80); bottomcount=bottomcount+(sumsalt=-88); surfcount=surfcount+(sumsalt<-90)");

  IterateOp *iter = new IterateOp(zoom, add, state);
  //merge->getResult()->print();

  RestrictOp *remove_drynodes = new RestrictOp("count > 0", 0, iter);
  
  ApplyOp *mean = new ApplyOp(remove_drynodes, "meansalt=sum/count");
  
  mean->getResult()->print();
}

int memleak() {
  ElcircFile ef("/home/bill/testbed/data/1_salt.63");
  
  GridField *H = ef.readHGrid();

  FileArrayReader *ar = ef.getSurfReader(0, "");

  BindOp *bind = new BindOp("surf", INT, ar, H); 

  bind->getResult();

  int offset=0;
 
  for (int j=0; j<100; j++) { 
  for (int i=0; i<96; i++) {
    offset = ef.getSurfOffset(i);
    bind->setOffsetInt(offset);
    bind->getResult()->GetAttribute(0, "surf")->print();
  }
  }
  
}
/*
int testConnect() {
  Grid *G = new ZeroGrid("G", 10);
  GridField *GF = new GridField(G, 0);

   float xy[20] = {
     -4.4999921736307369, -2.3684622117955817,
     -2.0413498678629751, 3.0327672376602583,
     -6.452955383763607, 7.17886471718444,
     1.4346928963760779, -5.1164979223315585,
     5.3309653464993139, -2.4654278894564396,
     8.02970019358182344, 9.1711493843897706,
     -10.1165843490665633, -4.433157762450313,
     3.1867727126802995, 1.08897664313109278,
     2.3461668909026002, 3.02692877783154024,
     6.1539189626033594, -6.0840006429553037,
   };

   float *x = xy;
   float *y = xy+10;

   Array *x_arr = new Array("x", FLOAT);
   x_arr->setVals((UnTypedPtr *) x, 10);
   Array *y_arr = new Array("y", FLOAT);
   y_arr->setVals((UnTypedPtr *) y, 10);

   GF->Bind(x_arr);
   GF->Bind(y_arr);
   GF->print();
  // ElcircFile ef("/home/bill/testbed/data/1_salt.63");
  // GridField *H = ef.readHGrid();
  // H->Dust();
   ConnectOp *conn = new ConnectOp(GF);
   
   GridField *Result = conn->getResult();
   
   vtkGridField *vtkgrid = toVTK(Result, "x");
   vtkRenderWindow *renWin = vtkRenderWindow::New();
   ShowEdges(vtkgrid->GetOutput(), renWin);
}
  */ 
int testToVTK() {
   ElcircFile ef("/home/bill/testbed/data/1_salt.63");
   GridField *H = ef.readHGrid();
   GridField *V = ef.readVGrid();
  
   RestrictOp *zoom = new RestrictOp("(x<350000)&(x>330000)&(y>280000)&(y<310000)", 0, H);
   //V->print();
  
   GridField *B = makeGridField(10, "G", "x", 0);
  
   ApplyOp *mkX = new ApplyOp(V, "y=z;x=z");
   //mkX->getResult()->print();
   
   CrossOp *cross = new CrossOp(zoom, V);
   //cross->getResult()->print();
   
   vtkGridField *vtkgrid = toVTK(cross->getResult(), "x");
   vtkGridField *vtkgrid2 = toVTK(V->getResult(), "z");
   vtkRenderWindow *renWin = vtkRenderWindow::New();
   //ViewTwo(vtkgrid->GetOutput(), vtkgrid2->GetOutput(), renWin);
   DirectVis(vtkgrid->GetOutput(), renWin);
}

int testapply() {
  ElcircFile ef("/home/bill/testbed/data/1_salt.63");
  
  GridField *V = makeGridField(10, "G", "x", 0);
  ApplyOp *app = new ApplyOp(V, "x = x * 2");

  GridField *X = app->getResult();
  X->print();
}

//========================================================

int sergey() {

  float start = gettime();
  float last = start;
  
  ElcircFile ef("/home/howew/data/1_salt.63");
  
  GridField *H = ef.readHGrid();
  H->Dust();
  AccumulateOp *ordH = new AccumulateOp(H, "hpos", "hpos+1", "0");

  GridField *V = ef.readVGrid();
  AccumulateOp *ordV = new AccumulateOp(V, "vpos", "vpos+1", "0");

  GridField *T = ef.readTGrid();

  RestrictOp *resT = new RestrictOp("tstep < 50", 0, T);
/*  AccumulateOp *ordT = new AccumulateOp(T, "timestep", "timestep+1", "0");
  stringstream calc_taddr;
  calc_taddr << "taddr=timestep*" << ef.getTimestepSize() << "/4";
  ApplyOp *taddr = new ApplyOp(ordT, calc_taddr.str());
*/
  stringstream ss;
  ss << "column-b+" << V->card() << endl;
  AccumulateOp *column = new AccumulateOp(ordH, "column", ss.str(), "0");
  column->SetOffset(-1);

//  RestrictOp *zoom = new RestrictOp("(column < 1028) & (column > 920)", column);
//  one->getResult()->print();
//  getchar();

  column->getResult();
  cout << "intro: " << gettime() - last << endl;
  last = gettime();
  
  RestrictOp *zoom = new RestrictOp("(x<350000)&(x>330000)&(y>280000)&(y<310000)", 0, column);
//  cout << "Loop size: " << zoom->getResult()->card() << endl;
  zoom->getResult();
  cout << "restrict: " << gettime() - last << endl;
  last = gettime();

  CrossOp *cross = new CrossOp(zoom, ordV);
  cross->getResult();
  cout << "cross: " << gettime() - last << endl;
  last = gettime();
  
  RestrictOp *restrict = new RestrictOp("vpos > (b-1)", 0, cross);
  restrict->getResult();
  cout << "bottom: " << gettime() - last << endl;
  last = gettime();

  float intro = gettime();
  cout << "Prior to Iteration: " << intro-start << endl;
  
  TagOp *tag = new TagOp(resT, restrict);
  
  ApplyOp *addr = new ApplyOp(tag, "addr = taddr + column + vpos - b");
  
  FileArrayReader *ar = ef.getVariableReader(0, "addr");
  BindOp *bind = new BindOp("salt", FLOAT, ar, addr); 
 /* 
  GridField *tst = bind->getResult();
  tst->GetAttribute(0, "addr")->print();
  tst->GetAttribute(0, "salt")->print();
*/
//  RestrictOp *surf = new RestrictOp("salt=-99", 0, bind);

  //attribute to sum, and value to use when summing an empty set.
  //Aggregate::Sum sum("salt", -88);
  //Assign::inverse_pointer iptr("vpos");
  //AggregateOp *agg = new AggregateOp(ordV, &iptr, &sum, surf);
  
  ApplyOp *init = new ApplyOp(restrict, "sumsalt=0; count=0");
  StateOp *state = new StateOp(init);
  
  MergeOp *merge = new MergeOp(state,bind);

  ApplyOp *add = new ApplyOp(merge, "sumsalt=sumsalt+salt*(salt>-99); count=count+(salt>-99)");
  //add->getResult()->GetAttribute("sumsalt")->print();

  IterateOp *iter = new IterateOp(resT, add, state);
  //add->getResult()->GetAttribute("sumsalt")->print();

//  RestrictOp *remove_drynodes = new RestrictOp("count > 0", 0, iter);
  
  ApplyOp *mean = new ApplyOp(iter, "meansalt=sumsalt/count");
  
  iter->getResult()->GetAttribute(0, "sumsalt")->print();
}

int scanMemory() {
  GridField *GF = makeGridField(10, "gf","x", 0);
  
  OutputOp *out = new OutputOp("store.gf", 0, GF);

  GridField *stored = out->getResult();

  ifstream f("store.gf", ios::binary | ios::in);
  f.seekg(0, ios_base::end);
  int size = f.tellg();
  f.seekg(0, ios_base::beg);
  
  char *data = new char[size];
  f.read(data, size);

  ScanInternal *scan = new ScanInternal(data, size);
  
  GridField *scanned = scan->getResult();
  
  scanned->print();
}

int littleBind() {
  GridField *GF = makeGridField(10, "gf","x", 0);

  AccumulateOp *acc = new AccumulateOp(GF, "pos", "pos+1", "0");

  acc->getResult()->GetAttribute(0, "pos")->print();
  RestrictOp *r = new RestrictOp("x<-3 | x>4", 0, acc);
  
  FileArrayReader *ar = new FileArrayReader("testdata", 0, "pos");
  BindOp *bop = new BindOp("new", INT, ar, r); 


  GridField *Result = bop->getResult();

  //Result->print();
  
  Result->GetAttribute(0, "new")->print();
  for (int i=0; i<1000; i++) {
    bop->setOffsetInt(i%2 * 40);
    Result = bop->getResult();
    Result->GetAttribute(0, "new")->print();
  }
  
  //Result->print();
}

int newBind() {
  Catalog c;

  ElcircFile ef("/home/bill/testbed/data/1_salt.63");
  
  GridField *H = ef.readHGrid();
  AccumulateOp *ordH = new AccumulateOp(H, "hpos", "hpos+1", "0");

  GridField *V = ef.readVGrid();
  AccumulateOp *ordV = new AccumulateOp(V, "vpos", "vpos+1", "0");

  GridField *T = ef.readTGrid();
  
  stringstream ss;
  ss << "column-b+" << V->card() << endl;
  AccumulateOp *column = new AccumulateOp(ordH, "column", ss.str(), "0");
  column->SetOffset(-1);

  column->getResult();
  
 RestrictOp *zoom = new RestrictOp("(column < 1028) & (column > 920)", 0, column);
 // RestrictOp *zoom = new RestrictOp("(x<355000)&(x>350000)&(y>290000)&(y<310000)", column);

  CrossOp *cross = new CrossOp(zoom, ordV);
  cross->getResult();
  
  ApplyOp *addrV = new ApplyOp(cross, "addr=column+vpos-b");
  
  RestrictOp *restrict = new RestrictOp("vpos > (b-1)", 0, addrV);
  restrict->getResult();

  Bind2Op *bind = new Bind2Op(&c, restrict, "2002;1;01;1;5", "u", "addr");

  DataDumpOp *dump = new DataDumpOp("outfile.dmp", 0, bind); 

  GridField *g = dump->getResult();
  cout << g->card() << endl;
  g->GetAttribute(0, "u")->print();

}

int tupSpeedTest() {

  Catalog c;

  //FileArrayReader *arr = c.getElcircArrayReader(2002, 1, "01", 1, 5, "hvel");

  //GridField *
/*
  
  GridField *G = makeGridField(12, "g", "x", 0);


  ApplyOp *app = new ApplyOp(G, "y=x+2");
  app->getResult()->print();
  */
  /*
  Tuple t(G->getScheme());
  Tuple u(G->getScheme());

  G->BindTuple(t, 0);
  G->BindTuple(u, 0);

  t.print();
  u.print();

  t.Next();
  G->BindTuple(u, 1);

  t.print();
  u.print();
  */
}

void testAdjacent() {
  GridField *gf = makeMesh();

  gf->recordOrdinals("pos");

  //g->print();
  //getchar();
  
  Cell center(1);
  center.getnodes()[0] = 1;

  set<CellId> out;
  gf->grid->adjacent(0, 1, out);

  set<CellId>::iterator p;
  for (p=out.begin(); p!=out.end(); p++) {
  //  ((Cell *)&(*p))->print();
  }

  UnitGrid U;
  GridField Uf(&U, 0);

  CrossOp X(gf, &Uf);

  //Assign::adjacent adj;
  //Aggregate::gradient<float> grad("v");
  Assign::Nodes nds;
  Aggregate::triGradient<float> grad("v");
  
  GridField two(gf->grid, 2);
  
  AggregateOp agg(&two, 2, &nds, &grad, gf, 0);

  AccumulateOp acc(&agg, "cid", "cid+1", "0");

  Aggregate::_sum<float> sumup("cid");
  Assign::neighbors nbrs;

  AggregateOp back(gf, 2, &nbrs, &sumup, &acc, 0);

  GridField *R = back.getResult();
  R->print();
}

void testAdjacentBig() {
  string salt63("/home/workspace/ccalmr/forecasts_dev/2004/2004-360/run/1_salt.63");
  float start = gettime(); 
  ElcircFile ef(salt63);
  
  GridField *H = ef.readHGrid();
  AccumulateOp aH(H, "hpos", "hpos+1", "0");
  
  GridField *V = ef.readVGrid();
  AccumulateOp aV(V, "vpos", "vpos+1", "0");
  
  RestrictOp rH("(x<337000) & (x>286000) & (265000<y) & (y<342000)", 0, &aH);

  //rH.getResult()->grid->normalize();

  stringstream ss;
/*
//2-D slice
  ss << "z=-h*10; addr=hpos*" << V->card() << "+25";

  cout << ss.str() << endl;
  ApplyOp G(&rH, ss.str());
*/

  //3-D
  CrossOp X(&rH, &aV);
  
  ss << "z=z*h*5; addr=hpos*" << V->card() << "+vpos";
  ApplyOp G(&X, ss.str());

  G.getResult()->GetAttribute(0, "z")->print();
  FileArrayReader *ar = ef.getVariableReader(0, "addr");

  BindOp bnd("salt", FLOAT, ar, &G);
  ApplyOp clamp(&bnd, "salt = (salt<33)*(salt>0)*salt");
 
  Assign::adjacent adj;
  Assign::Nodes nds;
  //Aggregate::triGradient<float> grad("h");
  Aggregate::gradient3D<float> grad("salt");
  
  //GridField two(H->grid, 2);
 
  cout << "clamp dim = " << clamp.getResult()->grid->getdim() << endl;
  
  AggregateOp agg(&clamp, &adj, &grad, &clamp);

  ApplyOp mag(&agg, "mag=1000*(gradxsalt^2 + gradysalt^2)^(0.5)");
  //Aggregate::_sum<float> sumup("area; gradxh; gradyh");
  //Assign::neighbors nbrs;

  //AggregateOp back(H, &nbrs, &sumup, &agg);
  RestrictOp front("(mag>0.8) & (mag<5)", 0, &mag);
  
  GridField *R = front.getResult();
  R->GetAttribute(0, "mag")->print();
  vtkGridField *vtkgrid = toVTK(R, "mag");

  cout << gettime() - start << endl; 
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  DirectVis(vtkgrid->GetOutput(), renWin);
}

void offScreen() {
  
}

void testScalar3d() {
  float start = gettime();

  ElcircFile ef("/home/howew/data/1_salt.63");
  
  GridField *H = ef.readHGrid();
  AccumulateOp *ordH = new AccumulateOp(H, "hpos", "hpos+1", "0");

  GridField *V = ef.readVGrid();
  AccumulateOp *ordV = new AccumulateOp(V, "vpos", "vpos+1", "0");

  GridField *D = ef.readDGrid();
  D->print();
  exit(1);
  
  stringstream ss;
  ss << "column-b+" << V->card() << endl;
  AccumulateOp *column = new AccumulateOp(ordH, "column", ss.str(), "0");
  column->SetOffset(-1);
  
  RestrictOp *zoom = new RestrictOp("(350000<x)&(x<355000)&(290000<y)&(y<310000)", 0, column);
  RestrictOp *rV = new RestrictOp("(1560<z)&(z<4890)", 0, ordV);

  CrossOp *cross = new CrossOp(zoom, rV);
  
  ApplyOp *addrV = new ApplyOp(cross, "addr=column+vpos-b");
  
  RestrictOp *restrict = new RestrictOp("vpos > (b-1)", 0, addrV);

  FileArrayReader *ar = ef.getVariableReader(0, "addr");
  BindOp *bind = new BindOp("salt", FLOAT, ar, restrict); 
  
  vtkGridField *vtkgrid = toVTK(bind->getResult(), "salt");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  cout << "Time: " << gettime() - start << endl;
  //DirectVis(vtkgrid->GetOutput(), renWin);

}

void testInterpolate() {
/*
  GridField *R0 = makeMesh();

  int sz = 2;
  ZeroGrid S("S", sz);
  float x[sz];
  float y[sz];
  float v[sz];
  
  x[0] = 0.5;
  y[0] = 0.5;
  v[0] = 100; 
  x[0] = 0.2;
  y[0] = 0.8;
  v[0] = 200; 
  Array *ax = new Array("x", FLOAT);
  ax->copyFloatData(x, sz);

  Array *ay = new Array("y", FLOAT);
  ay->copyFloatData(y, sz);

  Array *av = new Array("v", FLOAT);
  av->copyFloatData(v, sz);

  GridField S0(&S, 0);

  S0.Bind(ax);
  S0.Bind(ay);
  S0.Bind(av);
*/
  ElcircFile elcircf("/home/workspace/ccalmr/hindcasts/2004-01-11/run/1_salt.63");
  GridField *elcirc = elcircf.readHGrid();

  ElcircFile selfef("/home/workspace/ccalmr/forecasts_dev/2004/2004-360/run/1_salt.63");
  GridField *selfe = selfef.readHGrid();

  //RestrictOp zoom("300000<x & x<350000 & 280000<y & y<320000", S0);
  string conditionS = "310000<x & x<330000 & 290000<y & y<310000";
  string conditionT = "310000<x & x<330000 & 290000<y & y<310000";
  //string condition = "320000<x & x<330000 & 290000<y & y<300000";
  //string conditionS = "320000<x & x<325000 & 290000<y & y<295000";
  //string conditionT = "319500<x & x<325500 & 289500<y & y<295500";
  RestrictOp zoomT(conditionT, 0, elcirc);
  RestrictOp zoomS(conditionS, 0, selfe);

  zoomT.getResult()->grid->normalize();

  ApplyOp pushS(&zoomS, "x=x + 22000");
  ApplyOp pushT(&zoomT, "z=10");


  GridFieldOperator *T0 = &pushT;
  GridFieldOperator *S0 = &pushS;

  Assign::Nodes nodes;
  Aggregate::area area("area");
  Aggregate::mkvector poly("poly");
  Aggregate::dotwo poly_area(poly, area);
  AggregateOp T2(T0, 2, &nodes, &poly_area, T0, 0);
  
  Assign::contains cont;
  Aggregate::_average<float> avg("h", 10000);
  Aggregate::_count cnt;
  Aggregate::dotwo both(avg, cnt);
  AggregateOp map(&T2, 2, &cont, &both, &zoomS, 0);
  map.getResult()->GetAttribute(0, "count")->print();

  RestrictOp mapped("count>0", 2, &map);
  
  ApplyOp weights(&mapped, "weighted = avgh*area");

  Assign::neighbors incidentto;
  Aggregate::_sum<float> sum("weighted, area, count", 60);
  AggregateOp addup(&weights, 2, &incidentto, &sum, &weights, 0);
  
  ApplyOp newval(&addup, "newval = sumweighted/sumarea");


  MergeOp merge(T0, &newval);
  //map.getResult()->print(); //getAttribute("avgv")->print();
  //map.getResult()->getAttribute("avgv")->print();
  //addup.getResult()->getAttribute("newval")->print();
  //vtkGridField *vtkgrid = toVTK(elcirc->getResult(), "h");
  //vtkRenderWindow *renWin = vtkRenderWindow::New();
  //DirectVis(vtkgrid->GetOutput(), renWin);
  merge.getResult()->GetAttribute(0, "h")->print();
  vtkGridField *vtkS = toVTK(S0->getResult(), "h");
  vtkGridField *vtkT = toVTK(merge.getResult(), "newval");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  ViewTwo(vtkS->GetOutput(), vtkT->GetOutput(), renWin);
}

void testInterpolate2() {

  string conditionT = "332000<x & x<350000 & 285000<y & y<300000";
  string conditionS = "330000<x & x<355000 & 280500<y & y<305000";
  //string conditionT = "336000<x & x<344000 & 290000<y & y<295000";
  //string conditionS = "336000<x & x<344000 & 290000<y & y<295000";

//==================================
// Prepare Elcirc Slice

  ElcircFile elcircf("/home/workspace/ccalmr/forecasts/forecasts_ref/2005/2005-150/run/1_salt.63");
  GridField *elcirc = elcircf.readHGrid();

  AccumulateOp ordelcirc(elcirc, "hpos", "hpos+1", "0");

  GridField *eV = elcircf.readVGrid();

  stringstream ss;
  ss << "column-b+" << eV->card() << endl;
  AccumulateOp column(&ordelcirc, "column", ss.str(), "0");
  column.SetOffset(-1);
  
  ApplyOp addrV(&column, "addr=column+61-b");
  
  stringstream econd;
  econd << conditionS << " & 61 > (b-1)";
  RestrictOp bathymetry(econd.str(), 0, &addrV);

  FileArrayReader *ear = elcircf.getVariableReader(0, "addr");
  BindOp elcircbind("salt", FLOAT, ear, &bathymetry); 
  
  RestrictOp elcircclamp("salt > -99", 0, &elcircbind);
  
//===============================
// Prepare Selfe Slice
  
  ElcircFile selfef("/home/workspace/ccalmr/forecasts/forecasts_exp/2005/2005-150/run/1_salt.63");
  GridField *selfe = selfef.readHGrid();

  AccumulateOp ordselfe(selfe, "hpos", "hpos+1", "0");

  RestrictOp szoom(conditionT, 0, &ordselfe);

  GridField *sV = selfef.readVGrid();
  stringstream sss;
  sss << "addr=hpos*" << sV->card() << "+ 25";
  ApplyOp G(&szoom, sss.str());

  FileArrayReader *sar = selfef.getVariableReader(0, "addr");

  BindOp selfebind("salt", FLOAT, sar, &G);
  ApplyOp selfeclamp(&selfebind, "salt = (salt<33)*(salt>0)*salt");

 //--------------------------------------------
 /*
  vtkGridField *vtkT = toVTK(elcircclamp.getResult(), "salt");
  vtkGridField *vtkS = toVTK(selfeclamp.getResult(), "salt");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  //DirectVis(vtkS->GetOutput(), renWin);
  ViewTwo(vtkS->GetOutput(), vtkT->GetOutput(), renWin);
  */

  //RestrictOp zoomT(conditionT, elcirc);
  //RestrictOp zoomS(conditionS, selfe);
  string attr = "salt";
  
  GridField *Source = elcircclamp.getResult();
  //GridField *Source = zoomS.getResult();
  Source->grid->normalize();
  GridField *Target = selfeclamp.getResult();
  //GridField *Target = zoomT.getResult();
  Target->grid->normalize();
  
  GridFieldOperator *T0 = Target;
  GridFieldOperator *S0 = Source;

  Assign::Nodes nodes;
  Aggregate::area area("area");
  Aggregate::_average<float> avg_salt(attr);
  Aggregate::mkvector poly("poly");
  Aggregate::dotwo poly_area(poly, area);
  Aggregate::dotwo avg_poly_area(avg_salt, poly_area);
  AggregateOp S2(Source, 2, &nodes, &avg_poly_area, Source, 0);

  S2.getResult();
  S2.getResult()->getScheme()->print();
  
  Assign::fastcontainedby cont;
  Aggregate::_average<float> avg("avg"+attr, 10000);
  Aggregate::_count cnt;
  Aggregate::dotwo both(avg, cnt);

  AggregateOp map(Target, 0, &cont, &avg, &S2, 2);

  map.getResult()->getScheme()->print();

  ApplyOp newval(&map, "newval=avgavg"+attr+"");

  RestrictOp translate_prep("newval<35", 0, &newval);
  ApplyOp translate(&translate_prep, "diff=(newval-salt); x=x+26000");

  newval.getResult()->GetAttribute(0, "newval")->print();
  translate.getResult()->GetAttribute(0, "diff")->print();
  vtkGridField *vtkS = toVTK(translate.getResult(), "newval");
  //vtkGridField *vtkT = toVTK(newval.getResult(), "salt");
  //vtkGridField *vtkT = toVTK(newval.getResult(), "salt");
  //vtkGridField *vtkT = toVTK(Source->getResult(), "salt");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  DirectVis(vtkS->GetOutput(), renWin);
  //ViewTwo(vtkS->GetOutput(), vtkT->GetOutput(), renWin);
}

void testVerticalInterpolate() {

  //ElcircFile selfef("/home/workspace/ccalmr/forecasts/forecasts_exp/2005/2005-150/run/1_salt.63");
  ElcircFile selfef("/home/workspace/ccalmr25/kilgren/selfe_fraser/2003-26-RWKoo/run/1_hvel.64");
  GridField *H = selfef.readHGrid();
  AccumulateOp ordH(H, "hpos", "hpos+1", "0");
  GridField *V = selfef.readVGrid();
  AccumulateOp ordV(V, "vpos", "vpos+1", "0");

  FileArrayReader *ar = selfef.getSurfReader(0, "hpos");
  BindOp b("surf", FLOAT, ar, H);
  b.getResult()->GetAttribute(0, "surf")->print();
  exit(1);
  
  //string conditionS = "330000<x & x<335000 & 290000<y & y<295000";
  string conditionS = "320000<x & x<335000 & 280000<y & y<295000";
  RestrictOp zoom(conditionS, 0, &ordH);
  zoom.getResult()->grid->normalize();

/*  
  ApplyOp slice(zoom, "z=4400");


  Assign::Nodes nodes;
  Aggregate::mkvector interval("rng");
  AggregateOp intervals(V1, nodes, interval, V);

  Assignment::intervalContains stab("rng", "z", "z");
  AggregateOp(slice, stab, 
  */

  ZeroGrid gH0("H0", zoom.getResult()->card());

  GridField h0(&gH0, 0);

  MergeOp H0(&zoom, &h0);
  
 // H0.getResult()->print();
  /*
  vtkSphereSource *sph = vtkSphereSource::New();
  sph->SetRadius(0.025);
  sph->SetThetaResolution(12);
  sph->SetPhiResolution(12);
  
  vtkGlyph3D *glyph = vtkGlyph3D::New();
  glyph->SetInput(vtkS->GetOutput());
  glyph->SetSource(sph->GetOutput());
  glyph->GetOutput();
  glyph->SetColorModeToColorByScalar();
  */

  Assign::Nodes nodes;
  Aggregate::_min<float> mn("z");
  Aggregate::_max<float> mx("z");
  Aggregate::dotwo minmax(mn, mx);
  AggregateOp V1(V, 1, &nodes, &minmax, V, 0);

  CrossOp C(&H0, &V1);
  
  string depth = "-3";
  string scale = "100";
  ApplyOp A(&C, "mindepth = (minz*h); maxdepth = (maxz*h)");
  /*
  ApplyOp A_(&A, "filter = mindepth < -18 & maxdepth > -18");
  A_.getResult()->GetAttribute("minz")->print();
  A_.getResult()->GetAttribute("maxz")->print();
  A_.getResult()->GetAttribute("filter")->print();
  */
  RestrictOp R("((mindepth < " + depth  + ") | (mindepth = " + depth +")) & ((" + depth + " < maxdepth) | (" + depth + " = maxdepth))", 0, &A);

  SiftOp S(&R);
  
  CrossOp D(&H0, &ordV);

  ApplyOp D_(&D, "z=h*z*"+scale);

  stringstream sss;
  sss << "addr=hpos*" << V->card() << "+ vpos";
  ApplyOp D_addr(&D_, sss.str());

  //FileArrayReader *sar = selfef.getVariableReader(0, "addr");
  //BindOp salt("salt", FLOAT, sar, &D_addr);
  FileArrayReader *saru = selfef.getVariableReader("u", 0, "addr");
  BindOp u("u", FLOAT, saru, &D_addr);
  FileArrayReader *sarv = selfef.getVariableReader("v", 0, "addr");
  BindOp v("v", FLOAT, sarv, &u);

  GridField *G = v.getResult();
  //for (int i=0; i<G->card(); i++) {
  //  cout << *(float *) G->GetAttributeVal(0, "v", i) << endl;
  //}

  MergeOp M(&v, &S);
  
  //M.getResult()->grid->normalize();
  
  AccumulateOp M_id(&M, "id", "id+1", "0");

  AccumulateOp zoom_id(&zoom, "id", "id+1", "0");
  
  stringstream ss_p;
  ss_p << "depth="+depth+";";
  ss_p << "lop = id*2+1;";
  ss_p << "hip = id*2;";
  ApplyOp visH(&zoom_id, ss_p.str());

  visH.getResult()->GetAttribute(0, "lop")->print();
  visH.getResult()->GetAttribute(0, "hip")->print();
  M_id.getResult()->GetAttribute(0, "id")->print();
  M_id.getResult()->GetAttribute(0, "hpos")->print();
  
  Assign::match equijoin("hpos", "hpos");
  Aggregate::interpolate1D<float> interp("z", "z", "u");
  //AggregateOp join(&visH, &equijoin, &interp, &M);
  //
  
  Assign::bypointer ptlo("lop");
  Assign::bypointer pthi("hip");
  Aggregate::first one("u,v,z",-99);
  AggregateOp joinlo(&visH, 0, &ptlo, &one, &M_id, 0);
  ApplyOp renamelo("lo_u=u; lo_v=v; lo_z=z", &joinlo, 0);
  AggregateOp joinhi(&renamelo, 0, &pthi, &one, &M_id, 0);
  ApplyOp renamehi(&joinhi, "hi_u=u; hi_v=v; hi_z=z");

  string expr = "";
  expr += "u = hi_u*(hi_z-depth)/(hi_z-lo_z) + lo_u*(depth-lo_z)/(hi_z-lo_z); ";
  expr += "v = hi_v*(hi_z-depth)/(hi_z-lo_z) + lo_v*(depth-lo_z)/(hi_z-lo_z);";
  expr += "z=depth*" + scale;
  ApplyOp _interp(&renamehi, expr);
  
  renamehi.getResult()->GetAttribute(0, "lo_u")->print();
  renamehi.getResult()->GetAttribute(0, "hi_u")->print();
  renamehi.getResult()->GetAttribute(0, "lo_v")->print();
  renamehi.getResult()->GetAttribute(0, "hi_v")->print();
  M.getResult()->GetAttribute(0, "u")->print();
  M.getResult()->GetAttribute(0, "v")->print();
  //_interp.getResult()->GetAttribute("hi_v")->print();
  //_interp.getResult()->GetAttribute("lo_u")->print();
  //_interp.getResult()->GetAttribute("lo_v")->print();
  //_interp.getResult()->GetAttribute("u")->print();
  //_interp.getResult()->GetAttribute("v")->print();
  
/*
  CrossOp S_(&zoom, &stretchz);
  
  ApplyOp Selfe(&S_, "z=-h*z");
  
  vtkGridField *vtkSelfe = toVTK(Selfe.getResult(), "z");
*/
  
  v.getResult()->GetAttribute(0, "u")->print();
  vtkGridField *vtkS = toVTK(_interp.getResult(), "u");
  vtkGridField *vtkT = toVTK(v.getResult(), "u");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  //DirectVis(vtkT->GetOutput(), renWin);
  ViewTwo(vtkT->GetOutput(), vtkS->GetOutput(), renWin);
}

void testDrawCross() {
  int n = 4;
  
  ZeroGrid xline("X", n);
  float *xs = new float[n];
  for (int i=0; i<n; i++) xs[i] = i;
  Array ax("x", FLOAT);
  ax.shareFloatData(xs, n);

  OneGrid yline("Y", n);
  float *ys = new float[n];
  for (int i=0; i<n; i++) ys[i] = i;
  Array ay("y", FLOAT);
  ay.shareFloatData(xs, n);

  GridField X(&xline);
  X.AddAttribute(0, &ax);
  GridField Y(&yline, 0, &ay);
  Y.AddAttribute(0, &ay);

  CrossOp C(&X, &Y);

  C.getResult()->PrintTo(cout, 5);

/*
  CellArray *twocells = new CellArray();
  int sq[4];
  sq[0] = 0;
  sq[1] = 1;
  sq[2] = 2;
  sq[3] = 3;
  
  twocells->addCellNodes(sq, 4);

  Grid square("Sq", 2);
  square.setImplicit0Cells(4);
  square.setKCells(twocells, 2);

  float x[7] = {0, 0, 1, 1};
  float y[7] = {0, 1, 1, 0};

  Array *ax = new Array("x", FLOAT);
  ax->copyFloatData(x, 4);

  Array *ay = new Array("y", FLOAT);
  ay->copyFloatData(y, 4);


  GridField *gf = new GridField(&square, 0);
  
  gf->Bind(ax);
  gf->Bind(ay);

  GridField *test = makeMesh();
  */
  vtkGridField *vtkS = toVTK(C.getResult(), "x");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  DirectVis(vtkS->GetOutput(), renWin);
}

void testBindConstant() {
  GridField *gf = makeMesh();

  BindConstantOp bind("const", 4.5, gf);

  bind.getResult()->print();

  bind.setAttributeVal("const", 6.5);

  bind.getResult()->GetAttribute(0, "const")->print();
  
  bind.setAttributeVal("x", 2.5);
  bind.getResult()->GetAttribute(0, "x")->print();

  ApplyOp app(&bind, "foo=cos(y)");

  app.getResult()->print();
  
}

void testSmallRestrict() {
   ElcircFile ef("/home/bill/testbed/data/1_salt.63");
   GridField *H = ef.readHGrid();
   float start = gettime();
   RestrictOp *zoom = new RestrictOp("(x<350000)&(x>330000)&(y>280000)&(y<310000)", 0, H);
  zoom->getResult();
  cout << gettime() - start << endl;
  vtkGridField *vtkS = toVTK(zoom->getResult(), "b");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  DirectVis(vtkS->GetOutput(), renWin);

}

void testApplyUpdate() {
  GridField *gf = makeMesh();
  
  ApplyOp app(gf, "foo=x+5");
  
  app.getResult()->print();

  app.setExpression("fooTwo=x-5");

  app.getResult()->print();

}

int main() {
   //vmon_begin();
   //tupSpeedTest();
   //newBind();
   //littleBind();
   //memleak();
   //testConnect();
   //scanMemory();
   //sergey();
   //testapply();
   //vmon_done();
   //testToVTK();
   //testAdjacentBig();
   //testAdjacent();
   //testScalar3d();
   testSmallRestrict();
   //offScreen();
   //testInterpolate();
   //testInterpolate2();
   //testVerticalInterpolate();
   //test();
   //testDrawCross();
   //testBindConstant();
   //testApplyUpdate();
}
