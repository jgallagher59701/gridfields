#include "grid.h"
#include "gridfield.h"
#include "array.h"
#include "arrayreader.h"
#include "scaninternal.h"
#include "output.h"
#include "cross.h"
#include "accumulate.h"
//#include "subapply.h"
#include "apply.h"
#include "restrict.h"
#include "bind.h"
#include "project.h"
#include "elcircfile.h"
#include "tonetcdf.h"


int main(int argc, char **argv) {

  string filename("data/1_flsu.61");
  string name("salt");
  int offset = 0;
  Type type = FLOAT;

  ElcircFile ef(filename);
  GridField *scanH = ef.readHGrid();
  GridField *scanV = ef.readVGrid();
  /*
  AccumulateOp *address = new AccumulateOp(scanH, "address", "62-b+1+address", "0");

  AccumulateOp *vpos = new AccumulateOp(scanV, "zpos", "zpos+1", "0");
  RestrictOp *bath = new RestrictOp(new Condition("zpos", ">", 0), vpos);
  BindOp *bind = new BindOp(name, type, filename, offset, "zpos", bath);
  
  SubApplyOp *nest = new SubApplyOp(bind, "vgrid", address);
  //parameterize bind and test
  sub->parameterize("address", bind, (ParameterAssigner::ParamFunc) &BindOp::setOffset);
  sub->parameterize("b", bath, (ParameterAssigner::ParamFunc) &RestrictOp::setIntValue);

  // UnNestOp *unnest = new UnNestOp();
  // need a good grid to map to.
  GridField *Xx = new GridField(new OneGrid(5), 0);
  BindOp *bindx = new BindOp("datx", 0, "x", FLOAT);
  //AccumulateOp *mkX = new AccumulateOp(Xx, "x", "x+136800", "-83000");
  
  GridField *Yy = new GridField(new OneGrid(5), 0);
  BindOp *bindy = new BindOp("daty", 0, "y", FLOAT);
  //AccumulateOp *mkY = new AccumulateOp(Yy, "y", "y+340000", "-966000");

  GridField *Gg = CrossOp::Cross(mkX, mkY);

  Assignments::pointpoly pp;
  Aggregations::interpolate2D interp;
  Aggregations::special special("vgrid");
  */
  /*
   * Idea: make an aggregate function that does the right thing given 
   * a sequence of gridfields....so, we have 3 or 4 tuples of type <x,y,GF salt>
   * take the 3/4 tuples and loop over the gridfield, evaluating the 
   * interpolation function on 3/4 (x,y,s) triples.
   * Ugly, since it's all in an aggregation function, but maybe a more 
   * general solution can be found involving parameterized aggregates.
   * What to do with GF of varying size?  Stop when any GFs are done.
   */
  /*
  //make aggregate lazy
  AggregateOp *regrid = new AggregateOp(Gg, pp, special, nest);

  vector<GridField *> vec;
  vec.push_back(Xx);
  vec.push_back(Yy);
  vec.push_back(V);
  */
  
  
  //H->print();
  //V->print();
  OutputNetCDFOp::OutputNetCDFOp oo("temp.nc", scanH, Scheme("h:f"), Scheme("wave:f")); 
  oo.SetDate("2004-06-24 00:00:00");
  GridField *G = oo.getResult(); 

  ApplyOp A("wave = h", 0, &oo);

  oo.WriteTimeVars(&A, 0, 900.0);
  oo.WriteTimeVars(&A, 1, 1800.0);

//  ElcircFile out("temp.gf");
//  GridField  *GF = out.readHGrid();
/*
  OutputElcircOp::OutputElcircOp oo("temp.gf", scanH); 
  GridField *G = oo.getResult(); 
  ElcircFile out("temp.gf");
  GridField  *GF = out.readHGrid();
*/
/* 
  OutputOp::OutputOp oo("temp.gf", 0, scanH); 
  GridField *G = oo.getResult(); 
  ScanInternal si("temp.gf", 0);
  GridField  *GF = si.getResult();
  GF->print();
 
  vector<GridField *> vec;
  ApplyOp::Apply(V, "v2", "z");
  
  GridField *V1 = ProjectOp::Project(V, "z");
  GridField *V2 = ProjectOp::Project(V, "v2");
     
  vec.push_back(V1); 
  vec.push_back(V2); 

  GridField *C = 
    ProjectOp::Project(
      AccumulateOp::Accumulate(
       CrossOp::Cross(V1, V2)
       , "cvals", "z*62+v2", "z*62+v2"
      ),
      "cvals"
    );
      
  OutputOp::WriteNetCDF(vec, C);
  */

}

